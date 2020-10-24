# c
Expression evaluator written in plain C, with support for pluggable functions

'c' (or 'c_d' in debug mode) is a mathematical expressions evaluator with the following features:
1) dynamically allocated named variables.
2) pluggable internal and external functions.
3) persistent history.

Operators:

  1) -
  2) +
  3) *
  4) /
  5) %
  6) ** (power)
  7) // (root)
  8) << (shift left)
  9) >> (shift right)
  10) & (logical and)
  11) | (logical or)
  12) ^ (logical xor)
  13) () (parenthesis)
  14) = (assign to a variable)

Last Result:

If a symbol-table object is provided, the evaluator stores its last result to the '_' variable for later reference.

Pluggable Functions:

  Initially, this calculator does not come with any pre-defined functions, they can be easily added either internally (see calcex.h, calcex.c), or as     external shell scripts. Whenever an expression try to use a function, the name of that function is first searched in the internal functions list, and if not found, that name is searched as a script-name in the same directory of the executable.
Function scripts are expected to receive any number of arguments and print out a numeric result to STDOUT.

Implementation Notes:

  1) The evaluator entry point (calc_evaluate(), see calc.h) takes a single expression string and a symbol-table interface, and returns a numeric result.
This way multiple successive calls can be made to the evaluator with the same symbol-table object, allowing passing variables from one expression to the next ones. (See main.c).
  2) The symbol-table interface (see calc.h) allows more elaborate symbols lookup logics if so needed.
  3) If the evaluator is called with a NULL symbol-table, an error will be generated upon each attempt to reference variables or functions.
  4) The projects links with the standard readline library to support persistent history.
  5) Evaluation flow is divided into the following steps:
    5.1) Lexing (lexer.h, lexer.c)
    5.2) infix-to-postfix conversion (lexer.h, lexer.c)
    5.3) Postfix tokens array evaluation (calc.h, calc.c)
  

