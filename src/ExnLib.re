let raisesLibTable = {
  let table = Hashtbl.create(15);
  open Exn;

  let array = [
    ("get", [invalidArgument]),
    ("set", [invalidArgument]),
    ("make", [invalidArgument]),
    ("init", [invalidArgument]),
    ("make_matrix", [invalidArgument]),
    ("fill", [invalidArgument]),
    ("blit", [invalidArgument]),
    ("iter2", [invalidArgument]),
    ("map2", [invalidArgument]),
  ];

  let bsJson =
    // bs-json
    [
      ("bool", [decodeError]),
      ("float", [decodeError]),
      ("int", [decodeError]),
      ("string", [decodeError]),
      ("char", [decodeError]),
      ("date", [decodeError]),
      ("nullable", [decodeError]),
      ("nullAs", [decodeError]),
      ("array", [decodeError]),
      ("list", [decodeError]),
      ("pair", [decodeError]),
      ("tuple2", [decodeError]),
      ("tuple3", [decodeError]),
      ("tuple4", [decodeError]),
      ("dict", [decodeError]),
      ("field", [decodeError]),
      ("at", [decodeError, invalidArgument]),
      ("oneOf", [decodeError]),
      ("either", [decodeError]),
    ];

  let buffer = [
    ("sub", [invalidArgument]),
    ("blit", [invalidArgument]),
    ("nth", [invalidArgument]),
    ("add_substitute", [notFound]),
    ("add_channel", [endOfFile]),
    ("truncate", [invalidArgument]),
  ];

  let bytes = [
    ("get", [invalidArgument]),
    ("set", [invalidArgument]),
    ("create", [invalidArgument]),
    ("make", [invalidArgument]),
    ("init", [invalidArgument]),
    ("sub", [invalidArgument]),
    ("sub_string", [invalidArgument]),
    ("extend", [invalidArgument]),
    ("fill", [invalidArgument]),
    ("blit", [invalidArgument]),
    ("blit_string", [invalidArgument]),
    // ("concat", [invalidArgument]), if longer than {!Sys.max_string_length}
    // ("cat", [invalidArgument]), if longer than {!Sys.max_string_length}
    // ("escaped", [invalidArgument]), if longer than {!Sys.max_string_length}
    ("index", [notFound]),
    ("rindex", [notFound]),
    ("index_from", [invalidArgument, notFound]),
    ("index_from_opt", [invalidArgument]),
    ("rindex_from", [invalidArgument, notFound]),
    ("rindex_from_opt", [invalidArgument]),
    ("contains_from", [invalidArgument]),
    ("rcontains_from", [invalidArgument]),
  ];

  let filename = [
    ("chop_extension", [invalidArgument]),
    ("temp_file", [sysError]),
    ("open_temp_file", [sysError]),
  ];

  let hashtbl = [("find", [notFound])];

  let list = [
    ("hd", [failure]),
    ("tl", [failure]),
    ("nth", [failure, invalidArgument]),
    ("nth_opt", [invalidArgument]),
    ("init", [invalidArgument]),
    ("iter2", [invalidArgument]),
    ("map2", [invalidArgument]),
    ("fold_left2", [invalidArgument]),
    ("fold_right2", [invalidArgument]),
    ("for_all2", [invalidArgument]),
    ("exists2", [invalidArgument]),
    ("find", [notFound]),
    ("assoc", [notFound]),
    ("combine", [invalidArgument]),
  ];

  let string = [
    ("get", [invalidArgument]),
    ("set", [invalidArgument]),
    ("create", [invalidArgument]),
    ("make", [invalidArgument]),
    ("init", [invalidArgument]),
    ("sub", [invalidArgument]),
    ("fill", [invalidArgument]),
    // ("concat", [invalidArgument]), if longer than {!Sys.max_string_length}
    // ("escaped", [invalidArgument]), if longer than {!Sys.max_string_length}
    ("index", [notFound]),
    ("rindex", [notFound]),
    ("index_from", [invalidArgument, notFound]),
    ("index_from_opt", [invalidArgument]),
    ("rindex_from", [invalidArgument, notFound]),
    ("rindex_from_opt", [invalidArgument]),
    ("contains_from", [invalidArgument]),
    ("rcontains_from", [invalidArgument]),
  ];

  let stdlib = [
    ("invalid_arg", [invalidArgument]),
    ("failwith", [failure]),
    ("/", [divisionByZero]),
    ("mod", [divisionByZero]),
    ("char_of_int", [invalidArgument]),
    ("bool_of_string", [invalidArgument]),
    ("int_of_string", [failure]),
    ("float_of_string", [failure]),
    ("read_int", [failure]),
    ("output", [invalidArgument]),
    ("close_out", [sysError]),
    ("input_char", [endOfFile]),
    ("input_line", [endOfFile]),
    ("input", [invalidArgument]),
    ("really_input", [endOfFile, invalidArgument]),
    ("really_input_string", [endOfFile]),
    ("input_byte", [endOfFile]),
    ("input_binary_int", [endOfFile]),
    ("close_in", [sysError]),
    ("exit", [exit]),
  ];

  let str = [
    ("search_forward", [notFound]),
    ("search_backward", [notFound]),
    ("matched_group", [notFound]),
    ("group_beginning", [notFound, invalidArgument]),
    ("group_end", [notFound, invalidArgument]),
  ];

  let yojsonBasic = [("from_string", [yojsonJsonError])];

  let yojsonBasicUtil = [
    ("member", [yojsonTypeError]),
    ("to_assoc", [yojsonTypeError]),
    ("to_bool", [yojsonTypeError]),
    ("to_bool_option", [yojsonTypeError]),
    ("to_float", [yojsonTypeError]),
    ("to_float_option", [yojsonTypeError]),
    ("to_int", [yojsonTypeError]),
    ("to_list", [yojsonTypeError]),
    ("to_number", [yojsonTypeError]),
    ("to_number_option", [yojsonTypeError]),
    ("to_string", [yojsonTypeError]),
    ("to_string_option", [yojsonTypeError]),
  ];

  [
    ("Array", array),
    ("Buffer", buffer),
    ("Bytes", bytes),
    ("Char", [("chr", [invalidArgument])]),
    ("Filename", filename),
    ("Hashtbl", hashtbl),
    ("Js.Json", [("parseExn", [jsExnError])]),
    ("Json_decode", bsJson),
    ("Json.Decode", bsJson),
    ("List", list),
    ("Pervasives", stdlib),
    ("Stdlib", stdlib),
    ("Stdlib.Array", array),
    ("Stdlib.Buffer", buffer),
    ("Stdlib.Bytes", bytes),
    ("Stdlib.Filename", filename),
    ("Stdlib.Hashtbl", hashtbl),
    ("Stdlib.List", list),
    ("Stdlib.Str", str),
    ("Stdlib.String", string),
    ("Str", str),
    ("String", string),
    ("Yojson.Basic", yojsonBasic),
    ("Yojson.Basic.Util", yojsonBasicUtil),
  ]
  |> List.iter(((name, group)) =>
       group
       |> List.iter(((s, e)) =>
            Hashtbl.add(table, name ++ "." ++ s, e |> Exceptions.fromList)
          )
     );

  table;
};

let find = (path: Common.Path.t) =>
  Hashtbl.find_opt(raisesLibTable, path |> Common.Path.toString);
