// Generated by ReScript, PLEASE EDIT WITH CARE

import * as List from "bs-platform/lib/es6/list.js";
import * as Curry from "bs-platform/lib/es6/curry.js";
import * as Random from "bs-platform/lib/es6/random.js";
import * as Caml_obj from "bs-platform/lib/es6/caml_obj.js";

var counter = {
  contents: Random.$$int(100)
};

function progress(param) {
  if (counter.contents < 0) {
    throw {
          RE_EXN_ID: "Assert_failure",
          _1: [
            "TestCyberTruck.res",
            8,
            6
          ],
          Error: new Error()
        };
  }
  counter.contents = counter.contents - 1 | 0;
  
}

var Nested = {
  f: progress
};

var Progress = {
  Nested: Nested
};

function justReturn(param) {
  
}

function alwaysLoop(_param) {
  while(true) {
    _param = undefined;
    continue ;
  };
}

function alwaysProgress(_param) {
  while(true) {
    progress(undefined);
    _param = undefined;
    continue ;
  };
}

function alwaysProgressWrongOrder(param) {
  alwaysProgressWrongOrder(undefined);
  return progress(undefined);
}

function doNotAlias(_param) {
  while(true) {
    _param = undefined;
    continue ;
  };
}

function progressOnBothBranches(x) {
  while(true) {
    progress(undefined);
    continue ;
  };
}

function progressOnOneBranch(x) {
  while(true) {
    if (x > 3) {
      progress(undefined);
    }
    continue ;
  };
}

function testParametricFunction(x) {
  while(true) {
    if (x > 3) {
      progress(undefined);
    }
    continue ;
  };
}

var testParametricFunction2 = testParametricFunction;

function callParseFunction(x, parseFunction) {
  return Curry._1(parseFunction, x);
}

function testCacheHit(x) {
  while(true) {
    if (x > 0) {
      progress(undefined);
    }
    continue ;
  };
}

function doNothing(param) {
  
}

function evalOrderIsNotLeftToRight(x) {
  evalOrderIsNotLeftToRight(x);
  progress(undefined);
  
}

function evalOrderIsNotRightToLeft(x) {
  progress(undefined);
  evalOrderIsNotRightToLeft(x);
  
}

function butFirstArgumentIsAlwaysEvaluated(x) {
  while(true) {
    progress(undefined);
    continue ;
  };
}

function butSecondArgumentIsAlwaysEvaluated(x) {
  while(true) {
    progress(undefined);
    continue ;
  };
}

function tokenToString(token) {
  if (typeof token !== "number") {
    return String(token._0);
  }
  switch (token) {
    case /* Asterisk */0 :
        return "*";
    case /* Eof */1 :
        return "Eof";
    case /* Lparen */2 :
        return "(";
    case /* Plus */3 :
        return "+";
    case /* Rparen */4 :
        return ")";
    
  }
}

function next(p) {
  p.token = Random.bool(undefined) ? /* Eof */1 : /* Int */({
        _0: Random.$$int(1000)
      });
  p.position = {
    lnum: Random.$$int(1000),
    cnum: Random.$$int(80)
  };
  
}

function err(p, s) {
  p.errors = {
    hd: s,
    tl: p.errors
  };
  
}

function expect(p, token) {
  if (Caml_obj.caml_equal(p.token, token)) {
    return next(p);
  } else {
    return err(p, "expected token " + tokenToString(p.token));
  }
}

var Parser = {
  tokenToString: tokenToString,
  next: next,
  err: err,
  expect: expect
};

var Expr = {};

function parseList(p, f) {
  var loop = function (p) {
    if (p.token === /* Asterisk */0) {
      return /* [] */0;
    }
    var item = Curry._1(f, p);
    var l = loop(p);
    return {
            hd: item,
            tl: l
          };
  };
  return loop(p);
}

function $$parseInt(p) {
  var n = p.token;
  var res = typeof n === "number" ? (err(p, "integer expected"), -1) : n._0;
  next(p);
  return res;
}

function parseExpression(xOpt, p) {
  var match = p.token;
  if (typeof match !== "number") {
    return {
            TAG: /* Int */0,
            _0: $$parseInt(p)
          };
  }
  if (match !== 2) {
    return {
            TAG: /* Int */0,
            _0: $$parseInt(p)
          };
  }
  next(p);
  var e1 = parseExpression(undefined, p);
  expect(p, /* Plus */3);
  var e2 = parseExpression(undefined, p);
  expect(p, /* Lparen */2);
  return {
          TAG: /* Plus */1,
          _0: e1,
          _1: e2
        };
}

function parseListInt(p) {
  return parseList(p, $$parseInt);
}

function parseListListInt(p) {
  return parseList(p, parseListInt);
}

function parseListExpression(p) {
  return parseList(p, (function (eta) {
                return parseExpression(undefined, eta);
              }));
}

function parseListExpression2(p) {
  var partial_arg = 7;
  return parseList(p, (function (param) {
                return parseExpression(partial_arg, param);
              }));
}

function parseListIntTailRecursive(p) {
  var _l = /* [] */0;
  while(true) {
    var l = _l;
    if (p.token === /* Asterisk */0) {
      return List.rev(l);
    }
    _l = {
      hd: $$parseInt(p),
      tl: l
    };
    continue ;
  };
}

function loopAfterProgress(_param) {
  while(true) {
    _param = undefined;
    continue ;
  };
}

function testLoopAfterProgress(param) {
  progress(undefined);
  return loopAfterProgress(undefined);
}

throw {
      RE_EXN_ID: "Assert_failure",
      _1: [
        "TestCyberTruck.res",
        250,
        17
      ],
      Error: new Error()
    };

export {
  progress ,
  progress2 ,
  Progress ,
  justReturn ,
  alwaysLoop ,
  alwaysProgress ,
  alwaysProgressWrongOrder ,
  doNotAlias ,
  progressOnBothBranches ,
  progressOnOneBranch ,
  testParametricFunction ,
  testParametricFunction2 ,
  callParseFunction ,
  testCacheHit ,
  doNothing ,
  evalOrderIsNotLeftToRight ,
  evalOrderIsNotRightToLeft ,
  butFirstArgumentIsAlwaysEvaluated ,
  butSecondArgumentIsAlwaysEvaluated ,
  Parser ,
  Expr ,
  parseList ,
  $$parseInt ,
  parseListInt ,
  parseListListInt ,
  parseExpression ,
  parseListExpression ,
  parseListExpression2 ,
  parseListIntTailRecursive ,
  testLoopAfterProgress ,
  loopAfterProgress ,
  UITermination ,
  ParserWihtOptionals ,
  Riddle ,
  TerminationTypes ,
  
}
/* counter Not a pure module */
