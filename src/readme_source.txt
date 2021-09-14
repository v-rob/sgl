This file documents the basic structure, principles, and style guidelines of Super Grayland's
source code. Please read this before venturing into the source code itself.

File Structure:
* `common.h`: This holds basic programming utilities not necessarily related to SGL in specific,
  but are used throughout the source code.  It is a must to know the contents of this header,
  especially error handling and fixed point, as they are found everywhere. Every file should
  include this one. It also includes `tigcclib.h`.
* `game.h/c`: This is where the game starts, handles initialization/deinitialization and has the
  mainloops for each state of the game.
* `map.h/c`: Handles the static map, including reading from/writing to map files and everything
  having to do with tiles and specials.
* `screen.c/h`: Manages all sprites and drawing to the screen.

General:
* All structs that hold allocated or file data that needs freeing/closing must follow certain
  stipulations:
	* When creating such a struct, it must be zeroed using `COM_zero(str)` to ensure that
	  everything starts in its deinitialized state.
	  although when there is a union involved, memset must be used to ensure that everything
	  is set to zero.
	* There must be an `init` function to allocate/open everything and a `deInit` function to
	  deallocate/close it all.
	* The `init` function must throw an error on failure (see `common.h` for error handling`).
	  It does not need to clean itself up because `deInit` will be automatically called by the
	  global error handler in `game.c`.
	* The `deInit` function must always be called, whether `init` succeeeded or failed. Hence,
	  `deInit` must be callable on the struct regardless of whether it is initialized, not
	  initialized, or partially initialized due to an error. In other words, before
	  deinitializing any member, it must check if it is NULL, H_NULL, flagged as not
	  initialized, or whatever, and leave it alone if it is.
	* Finally, `deInit` must zero the structure.
	* While these guidelines are somewhat long, they ensure that all allocations and files are
	  handled in a uniform way without the benefits of automatic constructors and destructors.
* Do not use the TRY block or error handling outside of the context of the global error handling
  mechanism. Throwing errors is only for unrecoverable errors; recoverable errors should return
  a boolean with `TRUE` indicating an error and `FALSE` indicating success, or, in more complex
  scenarios, an error integer/enum with zero being success.
* All non-const global variables must be reset at the beginning of the `_main` function in case
  the exectable is or was in RAM.
* GNU extensions, such as nested functions, typeof, or statements in expressions, are perfectly
  fine (SGL requries GCC4TI, after all) and are encouraged where they make things better.

Style Guidelines:
These guidelines may be broken in exceptional circumstances, but should usually be adhered to.

They're designed to both conform to the general style of TI-OS routines as well as making it
possible to determine what something is without looking it up (like FOO_PascalCase for types,
FOO_camelCase for functions, snake_case for local variables, etc).

If something is not explicitly stated, look for other code and try to emulate it. Better yet,
talk to somebody to get it written in this file.

* Spacing:
	* Indentation is tabs that are four spaces long.
	* The soft limit for lines is 95 characters with 100 being the absolute maximum. This
	  includes documentation files like this one. Continuation lines are indented by two tabs.
	* Preprocessor directives (when not at top level), goto labels, and default/case labels in
	  switch statements are unindented one tab to make them very obvious, like so:
		```
		// Some code...
		u16 whatever = 0;

	#define THING 20

	label:
		switch (x) {
		case 0:
			// Do thing
		case 1:
			// Do another thing
			break;
		default:
			break;
		}
		```
	  Gotos should be rare, but if they're used for a legitimate reason, it should be mind-
	  numbingly obvious where they jump to.
	* Place a space between single line comments and the text in them, i.e. `code; // Comment`,
	  and please use proper grammar, captialization, and punctuation.
	* Multi-line comments should have their bodies indented by one tab with no text on the lines
	  with the start and end comment delimiters. Put namespace titles on a separate line before
	  the multi-line comment. When using asterisk bullet points for documentation (such as in this
	  file), two spaces should be used to align to the bullet point after the tabs. An example:
		```
		// FOO Namespace: Functions that are foo-ey and bar-ey
		/*
			* Functions with `Foo`: Important component of foobar. For example, `FOO_doFoo()`.
			* Functions with `Bar`: Other important component of foobar. They are differentiated
			  from `Foo` functions by the fact that they have the letters B, A, and R.
		*/
		```
	* Continuation backslashes in macros should be aligned with each other using tabs, like so:
		```
		#define SOME_MACRO					\
				this						\
				that_and_the_other_thing	\
				those						\
				these
		```
	* Keywords like `if`, `while`, `for`, etc all should have a space between them and the
	  following parenthesis while function definitions, function calls, and `sizeof` should
	  not. `sizeof` should always have parentheses around its operand. For example:
		```
		if (clear)
			memset(&var, 0, sizeof(var));
		```
	* Braces after structs, enums, unions, and function definitions as well as freestanding
	  braces should be on a new line at the same indentation level while everything else should
	  be on the same line with one space separating them:
		```
		void someFunction()
		{
			struct SomeStruct
			{
				u16 X;
				u16 Y;
			};

			struct SomeStruct some;
			// Do stuff...
			if (some.X > some.Y) {
				u16 swapper = some.Y;
				some.Y = some.X;
				some.X = swapper;
			}
		}
		```
	  All with the exception of nameless unions/structs inside structs/unions, which look too
	  spaced out with braces on their own lines.
	* Pointers should have the asterisk next to the name, not the variable, so `u8 *var`.
	* Casts should not have a space between the type and variable, so `(u8 *)var`.
* Naming Conventions:
	* The naming conventions have been crafted so that one can look at a identifier and know
	  whether its a function, type, variable, or constant, and where it comes from.
	* Every identifier in header files should have a namespace prefix. This is two or three
	  capitalized letters with an underscore prepended to the indentifier, e.g. `GME_Game`.
		* Namespaces sort of double as C++ namespaces for separate functions as well as member
		  functions of structs. Therefore, structs like `GME_Game` have functions like `GME_init`
		  which act on that struct, while other functions like `COM_throwErr` operate alone.
		* Namespaces can be nested if they have their own logical group solely within another,
		  like `SCR_TB`.
		* Things only in `.c` files (like static functions) should not have namespaces.
		* No variables should ever have a namespace. Even though global variables can be in
		  header files, they should still have no namespace to easily differentiate them from
		  type identifiers. Globals should be very rare anyway, so there should be no confusion.
	* Local variables should be snake_case. Global and static variables should be PascalCase.
	* Structs, enums, and unions should in PascalCase and should not be typedef'd except when
	  necessary. Integer and fixed point typedefs should also be in PascalCase.
	* Struct and union members should be in PascalCase.
	* Enum members should have the form `<enum name>_<CAPS_SNAKE_CASE identifier>`; e.g. the
	  enum `MAP_Wrap` has the members `MAP_Wrap_NONE`, `MAP_Wrap_OBJS`, and `MAP_Wrap_LEVEL`.
	* Functions and function-like macros should be in camelCase.
	* Constants and non-function macros should be in CAPS_SNAKE_CASE.
	* If it is necessary to know how many elements are in an enum, insert an element at the end
	  with the name `<enum name>_LEN`, such as `OBJ_Type_LEN`.
* Miscellaneous:
	* Comment everything! Too many comments are better than none.
		* All namespaces should have a detailed explanation in the header of what they are all
		  about. See the point on multiline comments for an example.
		* Basically every struct, union, enum, member, constant, global variable, and function
		  should have a comment describing its functionality.
		* Stupidly obvious things don't need comments, but when in doubt of whether it is obvious
		  or not, add a comment.
		* Ideally, one should never have to venture into `.c` files to understand how to use a
		  function or struct.
		* In `.c` files, how complex algorithms work should be documented liberally with comments.
		* If you have to write a TODO comment, it should start with `TODO:` so a simple find in
		  files operation can find all places that require work.
	* Types:
		* Sizes with unspecified types, meaning int, long, short, etc, are completely forbidden.
		  An error will occur if they are used. Instead, use the types in `common.h`, meaning
		  the unsigned integers u8, u16, and u32 and their signed counterparts s8, s16, and s32.
		* The 89's floats are also not allowed because they are slow and might conflict with
		  `COM_zero`. Use fixed point instead.
		* The 8-bit integers should only be used in space constrained structs, whereas 16-bit
		  should be preferred to 8-bit for other variables and members due to their faster speed.
		* Strings should still use char, not u8. Don't use char for an integer, though.
		* Booleans should use the `bool` type in conjunction with the macros `TRUE` and `FALSE`.
	* All switch statements that don't switch on an enum and have `case` statements for every
	  value in that enum should have a `default` branch at the end, even when it simply breaks,
	  to show that not every value is covered intentionally.
	* `#include`'s should be in alphabetical order, except for `common.h`, which should be first
	  in all headers and separated from the rest with a newline. Headers should use `#pragma once`
	  instead of header guards.
