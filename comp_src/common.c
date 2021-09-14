// Super Grayland, Copyright 2020 Vincent Robinson under the zlib license. See 'LICENSE.txt' for more information.

#include "common.h"

int log_error(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	int len = vfprintf(stderr, format, args);

	FILE *log = fopen("errorlog.txt", "a");
	if (log == NULL) {
		va_end(args);
		return -1;
	}

	vfprintf(log, format, args);

	if (fclose(log)) {
		va_end(args);
		return -2;
	}

	va_end(args);

	return len;
}

const char *ERROR_MESSAGES[ERROR_LEN] = {
	"", // Dummy value; an error should never be called with 'ERROR_NONE'
	"An unhandled error occurred; this is probably a bug",
	"Unable to allocate memory for %s",
	"Unable to load file \"%s\"",
	"Unable to initialize SDL",
	"Unable to create window",
	"Unable to load image \"%s\"",
	"Unable to create %s surface",
	"Unable to convert %s surface",
	"Unable to set color key of %s surface"
};

enum error g_error_no = ERROR_NONE;

void error(enum error error_no, const char extra[], const char func[], u32 line)
{
	g_error_no = error_no;

	log_error(ERROR_MESSAGES[error_no], extra);
	log_error(":\n |- In function \"%s\", line %d\n", func, line);

	if (error_no >= S_ERROR_SDL_INIT)
		log_error(" |- SDL Traceback: %s\n", SDL_GetError());

	log_error("\n");
}

void endian_swap(void *word)
{
	u16 *uword = word;

	const union {
		u16 word;
		u8 bytes[2];
	} swapper = {0x0001};

	if (swapper.bytes[0] == 0x01) // Little endian, needs conversion
		*uword = (*uword >> 8) | (*uword << 8);
}
