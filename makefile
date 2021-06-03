CC := tigcc
RM := rm -rf
CFLAGS += -Wall -Wextra -Wno-missing-field-initializers -std=c99 -O2

ifdef DEBUG
CFLAGS += -DDEBUG
endif

SRC_DIR := src
SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)

TARGET := sgl
TARGETS := $(TARGET).89z $(TARGET).9xz $(TARGET).v2z

# Make sure project gets completely recompiled if CFLAGS change
ifneq ($(file <.cflags),${CFLAGS})
ifneq ($(file <.cflags),)
$(warning CFLAGS changed. Recompiling everything...)
endif
$(file >.cflags,${CFLAGS})
endif

all: $(TARGETS)

%.89z %.9xz %.v2z: $(OBJ) .cflags
# Simply link the target together
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c .cflags
	$(CC) $(CFLAGS) $< -c -o $@

# DEPENDENCIES ----------------------------------------------------------------
src/game.o: src/common.h src/game.h src/map.h src/screen.h
src/map.o: src/common.h src/map.h
src/obj.o: src/common.h src/obj.h
src/screen.o: src/common.h src/screen.h
# END DEPENDENCIES ------------------------------------------------------------

clean:
	$(RM) $(TARGETS)
	$(RM) $(OBJ)

help:
	@echo "make             - build $(TARGET)"
	@echo "make DEBUG=1     - build $(TARGET) with debug enabled"
	@echo "make -j8         - build $(TARGET) using 8 processes"
	@echo "make clean       - remove all build files (including $(TARGET))"

.PHONY: clean help all
