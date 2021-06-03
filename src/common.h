// Super Grayland: Copyright 2021 Vincent Robinson under the MIT license.
// See `license.txt` for more information.
// Before delving into the code, please read `readme_source.txt` to understand the basic design.

#pragma once

// Define these before `tigcclib.h` is included.
#define USE_TI89
#define USE_TI92PLUS
#define USE_V200

#define MIN_AMS 100
#define SAVE_SCREEN

// No OPTIMIZE_ROM_CALLS because the OS uses that register sometimes, and it's a pain to find
// the issue and fix it (and the problem might stay hidden for quite some time). Plus, there
// are essentially no ROM calls in the mainloop.

#include <tigcclib.h>

// Exact integer types. Non-exact types are NOT ALLOWED.
typedef  int8_t s8;
typedef uint8_t u8;
typedef  int16_t s16;
typedef uint16_t u16;
typedef  int32_t s32;
typedef uint32_t u32;

#pragma GCC poison short
#pragma GCC poison int
#pragma GCC poison long
#pragma GCC poison unsigned
#pragma GCC poison signed
#pragma GCC poison float
#pragma GCC poison double

// Boolean type. GCC4TI doesn't have `stdbool.h`, but it does have `_Bool`.
typedef _Bool bool;
#define TRUE 1
#define FALSE 0

/* Debugging:
	Primitive debugging can be enabled by defining the DEBUG macro. A warning will be issued
	when debugging is enabled. There are two debugging methods:
	* Use a reserved register to view values. The TiEmu debugger can show register values, so
	  for debugging purposes, d7 can be reserved in the global variable `Debug`. Then, values
	  can be assigned to it and viewed in the debugger at your convenience.
	* Print a debugging message with printf capabilities with `SCR_dispDebug`. This method is
	  blocks the game, unlike using a register, so it may not always be the best option.
*/
// #define DEBUG
#ifdef DEBUG
register s32 Debug asm("d7");
#endif

/* FXD Namespace: Fixed point utilities
	Super Grayland primarily uses fixed point because of the lack of fast floats on the 68k calcs,
	but there are actually other very nice benefits:
	* Fixed point numbers have absolute precision, so they lose no precision when they get big.
	* Because of their absolute precision, it's super easy to convert between different precisions
	  without having to deal with the multiplication, division, rounding, and modulus operations
	  inherent to floating point conversions. This is very nice since Super Grayland uses multiple
	  precision levels (tile space: prec. 0, pixel space: prec. 3, object space: prec. 8).
	* Fixed point numbers can use integers underneath, so s8, s16, and s32 can be chosen based
	  on a balance of memory and precision, unlike floats, which are always at least four bytes.

	To create a new fixed point type, first typedef an integer to the new type name like
	any other type. The integer type _must_ be signed to ensure proper conversions and
	comparisons between all fixed point types. Then, define a macro with the identifier
	`<type>_POINT` denoting how many bits the fractional part of the type should have.
	A value of zero is possible, and commonly used. For example, this definition:
	```
	typedef s16 SCR_Pixel;
	#define SCR_Pixel_POINT 3
	```
	makes a new 13.3 fixed point number of the name `SCR_Pixel`.

	To use the fixed point macros, substitute the fixed point type name everywhere a type
	parameter is required. For example:
	```
	MAP_Scroll scroll_x;
	// ...
	MAP_Pos pos_x = FXD_convert(MAP_Scroll, MAP_Pos, FXD_ceil(MAP_Scroll, scroll_x));
	```
	Signed integers typenames (i.e. s8, s16, and s32) are also valid as arguments to any of
	these functions, but specific types should be favored over plain integers when appropriate.

	For addition, subtraction, negation, and bitwise operations, simply use the normal
	operators. To multiply, divide, or modulus by an integer, use normal multiplication and
	division operators. However, to multiply or divide two fixed numbers, the dedicated
	functions `FXD_mult` and `FXD_div` must be used.
*/

// Integer point definitions
#define  s8_POINT 0
#define s16_POINT 0
#define s32_POINT 0

// Get the number of fractional bits in the fixed point type.
#define FXD_point(type) (type##_POINT)

// Get the denominator of the fixed point type
#define FXD_denom(type) (1 << type##_POINT)
// Get the mask of the denominator bits of the number
#define FXD_denomMask(type) (FXD_denom(type) - 1)
// Get the numerator of the fractional part of the specified number
#define FXD_numer(type, num) ((num) & FXD_denomMask(type))

// Round a fixed point number down
#define FXD_floor(type, num) ((num) & ~FXD_denomMask(type))
// Round a fixed point number up
#define FXD_ceil(type, num) ((((num) - 1) + FXD_denom(type)) & ~FXD_denomMask(type))

// TODO: Test mult and div
// Muliply two fixed point numbers.
#define FXD_mult(type, num1, num2) (((num1) * (num2)) >> type##_POINT)
// Divide one fixed point number by another. `prec` of the most significant bits will be
// truncated during the division to allow for more fractional precision. Be careful not to
// set it high enough to result in data loss.
#define FXD_div(type, num1, num2, prec) ((((num1) << (prec)) / ((num2) << (prec))) >> (prec))

// Create a fixed point number "literal" from a integer literal. Only use with number literals;
// for variables, always use `FXD_convert` This macro is not necessary if `<type>_POINT` is 0
// since those literals are the same for fixed point and integers.
#define FXD_literalInt(type, integer) ((integer) << type##_POINT)
// Create a new fixed point number "literal" with integer and fractional parts. Stipulations of
// `FXD_literalInt` apply.
#define FXD_literalFrac(type, integer, numer, denom) \
		(((integer) << type##_POINT) + (FXD_denom(type) * (numer) / (denom)))

// Convert one fixed point type to another. It is not necessary to use this macro when the two
// types have the same `<type>_POINT` value.
#define FXD_convert(from_type, to_type, num)				\
({															\
	__typeof__(+(num)) _num = (num);						\
	to_type##_POINT > from_type##_POINT ?					\
			_num << (to_type##_POINT - from_type##_POINT) :	\
			_num >> (from_type##_POINT - to_type##_POINT);	\
})

/* COM Namespace: Common programming utilities
	This namespace holds many basic utilities that are used throughout the source code.
*/

// Sets a struct or union to zeros. `str` must be a pointer to the struct. This is to be used
// instead of `struct SomeStruct = {}` because that may not fully fill structs containing unions
// to zero. Since NULL and H_NULL are zero and there are no other non-zero implementation
// defined "zero" values, memset is fine for the calculator.
#define COM_zero(str) memset((str), 0, sizeof(*(str)))

// Get the larger of two integers
#define COM_max(a, b)			\
({								\
	__typeof__(+(a)) _a = (a);	\
	__typeof__(+(b)) _b = (b);	\
	_a > _b ? _a : _b;			\
})

// Get the smaller of two integers
#define COM_min(a, b)			\
({								\
	__typeof__(+(a)) _a = (a);	\
	__typeof__(+(b)) _b = (b);	\
	_a < _b ? _a : _b;			\
})

// Get the absolute value. This works on any integer type, not just int like the standard abs
// function does.
#define COM_abs(n)				\
({								\
	__typeof__(+(n)) _n = n;	\
	_n < 0 ? -_n : _n;			\
})

// Adds a number to a variable unless it overflows the end, in which case it resets to the initial.
// Subtraction is possible with negative numbers, and end is a minimum in that case, not a maximum.
#define COM_wrapAdd(var, change, end, initial) ((var) = ((var) == (end) ? (initial) : (var) + (change)));

/* Error handling
	To throw an error, use COM_throwErr with a COM_Error and an optional extra information
	string as the parameters. This thrown error can then be caught with the standard TI-OS TRY
	block. COM_Error contains a list of recurring errors that can happen in multiple places in
	the source code. If the error doesn't fall into any COM_Error categories and can only occur
	in one place, use COM_Error_OTHER and put all the error information into the extra
	information string.
*/
// Error codes to be thrown with COM_throwErr.
enum COM_Error
{
	COM_Error_NONE,   // No error. Do NOT throw this; it's only for use with local variables.
	COM_Error_OTHER,  // Error not in this list; use info to describe the error.
	COM_Error_MEMORY, // Heap allocation failed; use info for the allocation that failed.
	COM_Error_FILE    // File could not be found; use info for the filename.
};

// A global variable which can be set to a string further describing the error in COM_Error.
// It is preferred to use COM_throwErr, however.
extern char *ErrorInfo;

// Throws the COM_ErrorCode `error`, providing extra information in `info` (NULL for no extra info)
#define COM_throwErr(error, info) \
({                                \
	ErrorInfo = info;             \
	ER_throw(error);              \
})
