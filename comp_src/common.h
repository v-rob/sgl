// Super Grayland, Copyright 2020 Vincent Robinson under the zlib license. See 'LICENSE.txt' for more information.

#pragma once

#include "SDL2/SDL.h"
// We don't want SDL's main function TODO keep?
#undef main

#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Main types */

// Integers; ALWAYS use these, not int, long, short, etc, unless those are absolutely necessary.
// For characters and strings, char shall be used, but _not_ as an 8 bit integer
typedef  int8_t  s8;
typedef uint8_t  u8;
typedef  int16_t s16;
typedef uint16_t u16;
typedef  int32_t s32;
typedef uint32_t u32; // WARNING: u32 does not work with some helper or fixed point functions and macros

/* Fixed point numbers */

/* Fixed point numbers have been used in Super Grayland as opposed to floating point for many reasons:
	* Fixed point numbers are faster and will perform better with very old computers or embedded systems
	* A number can have multiple different underlying sizes, e.g. a u8, s16, or s32 are all possible while
	  floats have a large fixed size (which can be implementation defined). Therefore, fixed point numbers
	  can be packed in structs better
	* There is no loss of precision when numbers get big.
	* It would be a pain to be constantly rounding floats when changing e.g. from pos_t to scroll_t numbers whereas
	  fixed point conversion automatically floors when losing precision.
*/

// Note that most integer/fixed point defined types Super Grayland uses (e.g. tile_t, pos_t) are signed even if they don't
// need to be for simpler conversion and usage with fixed point functions like Frac_Convert. Unsigned integers are still
// supported in fixed point functions/macros with the exception of u32.

// Fixed decimal point of any integer type. Only use where appropriate; tile_t or sprite_t is usually a better choice
#define int_POINT 0

// Get the decimal point for a type
#define Frac_POINT(type) type##_POINT
// Get the size of one whole number of the specified fixed point type
#define Frac_FRAC_SIZE(type) (1 << Frac_POINT(type))
// Get the mask of the fixed point number of the specified type
#define Frac_FRAC_MASK(type) (Frac_FRAC_SIZE(type) - 1)
// Get the fractional part of the specified fixed point number
#define Frac_FRAC_PART(type, num) ((num) & Frac_FRAC_MASK(type))

// Create a new fixed point number from a whole number. Please only use with constants; Frac_CONVERT is for use with existing variables.
// This is not necessary if the integer constant is 0 since that is the same is fixed point and integers
#define Frac_NEW(type, whole) ((whole) << Frac_POINT(type))
// Create a new fixed point number with whole and fractional parts
#define Frac_NEW_FRAC(type, whole, numer, denom) (((whole) << Frac_POINT(type)) + (Frac_FRAC_SIZE(type) * (numer) / (denom)))

// Convert one fixed point type to another TODO: Not function?
#define Frac_CONVERT(from, to, num)	Frac_Convert(Frac_POINT(from), Frac_POINT(to), num)
static inline s32 Frac_Convert(u8 from, u8 to, s32 num)
{
	return to > from ? num << (to - from) : num >> (from - to);
}

// Floor a fixed point number
#define Frac_FLOOR(type, num) ((num) & ~Frac_FRAC_MASK(type))
// Ceil a fixed point number
#define Frac_CEIL(type, num) ((((num) - 1) + Frac_FRAC_SIZE(type)) & ~Frac_FRAC_MASK(type))

// Print out a fixed point number
#define Frac_DUMP(type, num) Frac_Print(Frac_POINT(type), num)
static inline void Frac_Dump(u8 size, s32 num)
{
	printf("%d %u/%u", Frac_Convert(size, 0, num), num & ((1 << size) - 1), 1 << size);
}

/* Enums in packed structs */

// Enums have a default type of int, causing a problem when used in packed structs due to alignment. This macro
// allows the underlying type to be specified while still showing what enum type it is.
// Example usage: 'PACKED_ENUM(enum tile_type, u16)' makes a variable of type u16 but is meant to hold tile_type values
#define PACKED_ENUM(enum, int) int

/* Vectors */

// Vectors signify a coordinate with x and y components. The typename is 'v2_<type>'.
// Rule of thumb for use: use a vector in a struct (unless it is impractical to do so), but use two separate variables for a function
// like 'get_tile' unless there is a good reason to use a vector.
#define V2_DEF(type) typedef struct { type x; type y; } v2_##type;
V2_DEF(s8)
V2_DEF(u8)
V2_DEF(s16)
V2_DEF(u16)
V2_DEF(s32)
V2_DEF(u32)
#undef V2_DEF

/* Integer and vector typedef */

// This typedef macro makes a new integer type and corresponding vector type.
// Example usage: 'TYPEDEF(s32, pos_t)' creates the types pos_t and v2_pos_t which are s32's underneath.
// When using this, also create a macro called '<type>_POINT' to define the fixed decimal point (0 for integers like tile_t)
#define TYPEDEF(base, type)		\
	typedef base type;			\
	typedef v2_##base v2_##type;

/* Errors */

// Boolean values for functions to specify return status
#define FAILURE 1
#define SUCCESS 0

// Specific fatal errors to be used with ERROR and S_ERROR
enum error
{
	ERROR_NONE,
	// Normal errors
	S_ERROR_UNKNOWN,
	ERROR_ALLOC,
	ERROR_LOAD_FILE,
	// SDL-specific errors
	S_ERROR_SDL_INIT,
	S_ERROR_SDL_CREATE_WINDOW,
	ERROR_SDL_LOAD_MEDIA,
	ERROR_SDL_CREATE_SURFACE,
	ERROR_SDL_CONVERT_SURFACE,
	ERROR_SDL_SET_COLOR_KEY,
	ERROR_LEN
};

// Global current error number
extern enum error g_error_no;

// Display a fatal error with extra information; use 'ERROR_*' enum values without the prefix
#define ERROR(error_no, extra) error(ERROR_##error_no, extra, __func__, __LINE__)
// Display a fatal error that requires no other information; use 'S_ERROR_*' enum values without the prefix
#define S_ERROR(error_no) error(S_ERROR_##error_no, NULL, __func__, __LINE__)

// Full error prototype; use ERROR or S_ERROR instead
void error(enum error error_no, const char extra[], const char func[], u32 line);

/* Helper functions/macros */

// Function to print to both log file and terminal
int log_error(const char *format, ...);

// Max, min, and abs. Don't use with u32.
#ifdef _MSC_VER // Thanks, Microsoft
	#undef min
	#undef max
#endif

static inline s32 min(s32 a, s32 b)
{
	return a < b ? a : b;
}

static inline s32 max(s32 a, s32 b)
{
	return a > b ? a : b;
}

static inline s32 abs_s32(s32 n)
{
	return n < 0 ? -n : n;
}
#define abs abs_s32

// Flip u16/s16 endianness in place to little endian if necessary since files use big endian
void endian_swap(void *word);

// Adds a number to a variable unless it overflows the end, in which case it resets to the initial.
// Subtraction is possible with negative numbers, and end is a minimum in that case, not a maximum
#define WRAP_ADD(var, change, end, initial) ((var) = ((var) == (end) ? (initial) : (var) + (change)));
