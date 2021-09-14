:: Super Grayland: Copyright 2021 Vincent Robinson under the zlib license.
:: See `license.txt` for more information.
:: Usage: 'comp <compiler>' where '<compiler>' is one of the following:
::   * 68k - Compile for the TI-89/92/Voyage 200 calculators with GCC4TI (TIGCC might also work).
::   * gcc - Compile for a normal computer with GCC using legacy prototype computer code.
::   * vs - Compile for Windows with Visual Studio using legacy prototype computer code. Requires
::     the usage of the CL command line.
:: TODO: A makefile should probably be used instead, but I don't know how to use them yet, and
:: I'm to lazy to learn right now. Also, it should support defining the DEBUG macro.

@echo off
cls
echo Building Super Grayland...
echo.

:: Program name
set name=sgl

:: List of files to compile
if %1==68k (
	set files = 		^
		src/game.c		^
		src/map.c		^
		src/object.c	^
		src/screen.c
) else (
	set files=comp_src/*.c comp_src/SDL2.lib
)

:: Compile using the requested compiler
if %1==gcc (
	gcc -Wall -Wextra -O2 %files% -o %name%
)
if %1==vs (
	cl /W3 /Fe: %name% %files%
	del *.obj
)
if %1==68k (
	:: Use -O2 because -O3 triples the executable size with no visible performance increase.
	tigcc -Wall -Wextra -Wno-missing-field-initializers -std=gnu99 -O2 %files% -o %name%
)

:: Success!
echo.
if not %errorlevel%==0 (
	echo Build failed. Uh, blame Gravil.
	exit /B
)
echo Build successful
:: Run if we've produced an executable file
if not %1==68k (
	%name%
)
