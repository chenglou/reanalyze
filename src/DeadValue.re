/* Adapted from https://github.com/LexiFi/dead_code_analyzer */

open DeadCommon;

let checkAnyValueBindingWithNoSideEffects =
    (
      {vb_pat: {pat_desc}, vb_expr: expr, vb_loc: loc}: Typedtree.value_binding,
    ) =>
  switch (pat_desc) {
  | Tpat_any when !SideEffects.checkExpr(expr) && !loc.loc_ghost =>
    let name = "_" |> Name.create(~isInterface=false);
    let currentModulePath = ModulePath.getCurrent();
    let path = currentModulePath.path @ [Common.currentModuleName^];
    name
    |> addValueDeclaration(
         ~path,
         ~loc,
         ~moduleLoc=currentModulePath.loc,
         ~sideEffects=false,
       );
  | _ => ()
  };

let collectValueBinding = (super, self, vb: Typedtree.value_binding) => {
  let oldCurrentBindings = Current.bindings^;
  let oldLastBinding = Current.lastBinding^;
  checkAnyValueBindingWithNoSideEffects(vb);
  let loc =
    switch (vb.vb_pat.pat_desc) {
    | Tpat_var(id, {loc: {loc_start, loc_ghost} as loc})
    | Tpat_alias(
        {pat_desc: Tpat_any},
        id,
        {loc: {loc_start, loc_ghost} as loc},
      )
        when !loc_ghost && !vb.vb_loc.loc_ghost =>
      let name = Ident.name(id) |> Name.create(~isInterface=false);
      let optionalArgs =
        vb.vb_expr.exp_type
        |> DeadOptionalArgs.fromTypeExpr
        |> OptionalArgs.fromList;
      let exists =
        switch (PosHash.find_opt(decls, loc_start)) {
        | Some({declKind: Value(r)}) =>
          r.optionalArgs = optionalArgs;
          true;
        | _ => false
        };
      let currentModulePath = ModulePath.getCurrent();
      let path = currentModulePath.path @ [Common.currentModuleName^];

      let isFirstClassModule =
        switch (vb.vb_expr.exp_type.desc) {
        | Tpackage(_) => true
        | _ => false
        };
      if (!exists && !isFirstClassModule) {
        // This is never toplevel currently
        let isToplevel = oldLastBinding == Location.none;
        let sideEffects = SideEffects.checkExpr(vb.vb_expr);
        name
        |> addValueDeclaration(
             ~isToplevel,
             ~loc,
             ~moduleLoc=currentModulePath.loc,
             ~optionalArgs,
             ~path,
             ~sideEffects,
           );
      };
      switch (PosHash.find_opt(decls, loc_start)) {
      | None => ()
      | Some(decl) =>
        // Value bindings contain the correct location for the entire declaration: update final position.
        // The previous value was taken from the signature, which only has positions for the id.

        let declKind =
          switch (decl.declKind) {
          | Value(vk) =>
            DeclKind.Value({
              ...vk,
              sideEffects: SideEffects.checkExpr(vb.vb_expr),
            })
          | dk => dk
          };
        PosHash.replace(
          decls,
          loc_start,
          {
            ...decl,
            declKind,
            posEnd: vb.vb_loc.loc_end,
            posStart: vb.vb_loc.loc_start,
          },
        );
      };
      loc;
    | _ => Current.lastBinding^
    };
  Current.bindings := PosSet.add(loc.loc_start, Current.bindings^);
  Current.lastBinding := loc;
  let r = super.Tast_mapper.value_binding(self, vb);
  Current.bindings := oldCurrentBindings;
  Current.lastBinding := oldLastBinding;
  r;
};

let processOptionalArgs =
    (~expType, ~locFrom: Location.t, ~locTo, ~path, args) =>
  if (expType |> DeadOptionalArgs.hasOptionalArgs) {
    let supplied = ref([]);
    let suppliedMaybe = ref([]);
    args
    |> List.iter(((lbl, arg)) => {
         let argIsSupplied =
           switch (arg) {
           | Some({
               Typedtree.exp_desc: Texp_construct(_, {cstr_name: "Some"}, _),
             }) =>
             Some(true)
           | Some({
               Typedtree.exp_desc: Texp_construct(_, {cstr_name: "None"}, _),
             }) =>
             Some(false)
           | Some(_) => None
           | None => Some(false)
           };
         switch (lbl) {
         | Asttypes.Optional(s) when !locFrom.loc_ghost =>
           if (argIsSupplied != Some(false)) {
             supplied := [s, ...supplied^];
           };
           if (argIsSupplied == None) {
             suppliedMaybe := [s, ...suppliedMaybe^];
           };
         | _ => ()
         };
       });
    (supplied^, suppliedMaybe^)
    |> DeadOptionalArgs.addReferences(~locFrom, ~locTo, ~path);
  };

let collectExpr = (super, self, e: Typedtree.expression) => {
  let locFrom = e.exp_loc;
  switch (e.exp_desc) {
  | Texp_ident(_path, _, {Types.val_loc: {loc_ghost: false, _} as locTo}) =>
    if (locFrom == locTo && _path |> Path.name == "emptyArray") {
      // Work around lowercase jsx with no children producing an artifact `emptyArray`
      // which is called from its own location as many things are generated on the same location.
      if (Common.Cli.debug^) {
        Log_.item(
          "addDummyReference %s --> %s@.",
          Location.none.loc_start |> posToString,
          locTo.loc_start |> posToString,
        );
      };
      ValueReferences.add(locTo.loc_start, Location.none.loc_start);
    } else {
      addValueReference(~addFileReference=true, ~locFrom, ~locTo);
    }

  | Texp_apply(
      {
        exp_desc:
          Texp_ident(
            path,
            _,
            {Types.val_loc: {loc_ghost: false, _} as locTo},
          ),
        exp_type,
      },
      args,
    ) =>
    args
    |> processOptionalArgs(
         ~expType=exp_type,
         ~locFrom: Location.t,
         ~locTo,
         ~path,
       )

  | Texp_let(
      // generated for functions with optional args
      Nonrecursive,
      [
        {
          vb_pat: {pat_desc: Tpat_var(idArg, _)},
          vb_expr: {
            exp_desc:
              Texp_ident(
                path,
                _,
                {Types.val_loc: {loc_ghost: false, _} as locTo},
              ),
            exp_type,
          },
        },
      ],
      {
        exp_desc:
          Texp_function({
            cases: [
              {
                c_lhs: {pat_desc: Tpat_var(etaArg, _)},
                c_rhs: {
                  exp_desc:
                    Texp_apply({exp_desc: Texp_ident(idArg2, _, _)}, args),
                },
              },
            ],
          }),
      },
    )
      when
        Ident.name(idArg) == "arg"
        && Ident.name(etaArg) == "eta"
        && Path.name(idArg2) == "arg" =>
    args
    |> processOptionalArgs(
         ~expType=exp_type,
         ~locFrom: Location.t,
         ~locTo,
         ~path,
       )

  | Texp_field(
      _,
      _,
      {lbl_loc: {Location.loc_start: posTo, loc_ghost: false}, _},
    ) =>
    if (Config.analyzeTypes^) {
      DeadType.addTypeReference(~posTo, ~posFrom=locFrom.loc_start);
    }

  | Texp_construct(
      _,
      {cstr_loc: {Location.loc_start: posTo, loc_ghost} as locTo, cstr_tag},
      _,
    ) =>
    switch (cstr_tag) {
    | Cstr_extension(path, _) =>
      path |> DeadException.markAsUsed(~locFrom, ~locTo)
    | _ => ()
    };
    if (Config.analyzeTypes^ && !loc_ghost) {
      DeadType.addTypeReference(~posTo, ~posFrom=locFrom.loc_start);
    };

  | _ => ()
  };
  super.Tast_mapper.expr(self, e);
};

/*
 type k. is a locally abstract type
 https://caml.inria.fr/pub/docs/manual-ocaml/locallyabstract.html
 it is required because in ocaml >= 4.11 Typedtree.pattern and ADT is converted
 in a GADT
 https://github.com/ocaml/ocaml/commit/312253ce822c32740349e572498575cf2a82ee96
 in short: all branches of pattern matches aren't the same type.
 With this annotation we declare a new type for each branch to allow the
 function to be typed.
 */
let collectPattern:
  type k. (_, _, Compat.generalPattern(k)) => Compat.generalPattern(k) =
  (super, self, pat) => {
    let posFrom = pat.Typedtree.pat_loc.loc_start;
    switch (pat.pat_desc) {
    | Typedtree.Tpat_record(cases, _clodsedFlag) =>
      cases
      |> List.iter(((_loc, {Types.lbl_loc: {loc_start: posTo}}, _pat)) =>
           if (Config.analyzeTypes^) {
             DeadType.addTypeReference(~posFrom, ~posTo);
           }
         )
    | _ => ()
    };
    super.Tast_mapper.pat(self, pat);
  };

let rec getSignature = (moduleType: Types.module_type) =>
  switch (moduleType) {
  | Mty_signature(signature) => signature
  | Mty_functor(_) =>
    switch (moduleType |> Compat.getMtyFunctorModuleType) {
    | Some((_, mt)) => getSignature(mt)
    | _ => []
    }
  | _ => []
  };

let rec processSignatureItem =
        (~doTypes, ~doValues, ~moduleLoc, ~path, si: Types.signature_item) =>
  switch (si) {
  | Sig_type(_) when doTypes =>
    let (id, t) = si |> Compat.getSigType;
    if (Config.analyzeTypes^) {
      DeadType.addDeclaration(~typeId=id, ~typeKind=t.type_kind);
    };
  | Sig_value(_) when doValues =>
    let (id, loc, kind, valType) = si |> Compat.getSigValue;
    if (!loc.Location.loc_ghost) {
      let isPrimitive =
        switch (kind) {
        | Val_prim(_) => true
        | _ => false
        };
      if (!isPrimitive || Config.analyzeExternals) {
        let optionalArgs =
          valType |> DeadOptionalArgs.fromTypeExpr |> OptionalArgs.fromList;
        Ident.name(id)
        |> Name.create(~isInterface=false)
        |> addValueDeclaration(
             ~loc,
             ~moduleLoc,
             ~optionalArgs,
             ~path,
             ~sideEffects=false,
           );
      };
    };
  | Sig_module(_)
  | Sig_modtype(_) =>
    switch (si |> Compat.getSigModuleModtype) {
    | Some((id, moduleType, moduleLoc)) =>
      let collect =
        switch (si) {
        | Sig_modtype(_) => false
        | _ => true
        };
      if (collect) {
        getSignature(moduleType)
        |> List.iter(
             processSignatureItem(
               ~doTypes,
               ~doValues,
               ~moduleLoc,
               ~path=[id |> Ident.name |> Name.create, ...path],
             ),
           );
      };
    | None => ()
    }
  | _ => ()
  };

/* Traverse the AST */
let traverseStructure = (~doTypes, ~doExternals) => {
  /* Tast_mapper */
  let super = Tast_mapper.default;

  let expr = (self, e) => e |> collectExpr(super, self);
  let pat = (self, p) => p |> collectPattern(super, self);
  let value_binding = (self, vb) => vb |> collectValueBinding(super, self);
  let structure_item = (self, structureItem: Typedtree.structure_item) => {
    let oldModulePath = ModulePath.getCurrent();
    switch (structureItem.str_desc) {
    | Tstr_module({mb_expr, mb_id, mb_loc}) =>
      let hasInterface =
        switch (mb_expr.mod_desc) {
        | Tmod_constraint(_) => true
        | _ => false
        };
      ModulePath.setCurrent({
        ...oldModulePath,
        loc: mb_loc,
        path: [
          mb_id |> Compat.moduleIdName |> Name.create,
          ...oldModulePath.path,
        ],
      });
      if (hasInterface) {
        switch (mb_expr.mod_type) {
        | Mty_signature(signature) =>
          signature
          |> List.iter(
               processSignatureItem(
                 ~doTypes,
                 ~doValues=false,
                 ~moduleLoc=mb_expr.mod_loc,
                 ~path=
                   ModulePath.getCurrent().path @ [Common.currentModuleName^],
               ),
             )
        | _ => ()
        };
      };

    | Tstr_primitive(vd) when doExternals && Config.analyzeExternals =>
      let currentModulePath = ModulePath.getCurrent();
      let path = currentModulePath.path @ [Common.currentModuleName^];
      let exists =
        switch (PosHash.find_opt(decls, vd.val_loc.loc_start)) {
        | Some({declKind: Value(_)}) => true
        | _ => false
        };
      let id = vd.val_id |> Ident.name;
      if (!exists
          && id
          != "unsafe_expr" /* see https://github.com/BuckleScript/bucklescript/issues/4532 */) {
        id
        |> Name.create(~isInterface=false)
        |> addValueDeclaration(
             ~path,
             ~loc=vd.val_loc,
             ~moduleLoc=currentModulePath.loc,
             ~sideEffects=false,
           );
      };

    | Tstr_type(_recFlag, typeDeclarations) when doTypes =>
      if (Config.analyzeTypes^) {
        typeDeclarations
        |> List.iter((typeDeclaration: Typedtree.type_declaration) => {
             DeadType.addDeclaration(
               ~typeId=typeDeclaration.typ_id,
               ~typeKind=typeDeclaration.typ_type.type_kind,
             )
           });
      }

    | Tstr_include({incl_mod, incl_type}) =>
      switch (incl_mod.mod_desc) {
      | Tmod_ident(_path, _lid) =>
        let currentPath =
          ModulePath.getCurrent().path @ [Common.currentModuleName^];
        incl_type
        |> List.iter(
             processSignatureItem(
               ~doTypes,
               ~doValues=false, // TODO: also values?
               ~moduleLoc=incl_mod.mod_loc,
               ~path=currentPath,
             ),
           );
      | _ => ()
      }

    | Tstr_exception(_) =>
      switch (structureItem.str_desc |> Compat.tstrExceptionGet) {
      | Some((id, loc)) =>
        let path = ModulePath.getCurrent().path @ [Common.currentModuleName^];
        let name = id |> Ident.name |> Name.create;
        name |> DeadException.add(~path, ~loc, ~strLoc=structureItem.str_loc);
      | None => ()
      }
    | _ => ()
    };
    let result = super.structure_item(self, structureItem);
    ModulePath.setCurrent(oldModulePath);
    result;
  };
  Tast_mapper.{...super, expr, pat, structure_item, value_binding};
};

/* Merge a location's references to another one's */
let processValueDependency =
    (
      (
        {
          val_loc:
            {loc_start: {pos_fname: fnTo} as posTo, loc_ghost: ghost1} as locTo,
        }: Types.value_description,
        {
          val_loc:
            {loc_start: {pos_fname: fnFrom} as posFrom, loc_ghost: ghost2} as locFrom,
        }: Types.value_description,
      ),
    ) =>
  if (!ghost1 && !ghost2 && posTo != posFrom) {
    let addFileReference = fileIsImplementationOf(fnTo, fnFrom);
    addValueReference(~addFileReference, ~locFrom, ~locTo);
    DeadOptionalArgs.addFunctionReference(~locFrom, ~locTo);
  };

let processStructure =
    (
      ~cmt_value_dependencies,
      ~doTypes,
      ~doExternals,
      structure: Typedtree.structure,
    ) => {
  let traverseStructure = traverseStructure(~doTypes, ~doExternals);
  structure |> traverseStructure.structure(traverseStructure) |> ignore;

  let valueDependencies = cmt_value_dependencies |> List.rev;

  valueDependencies |> List.iter(processValueDependency);
};
