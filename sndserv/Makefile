PROGS = sndserver
LDLIBS = -lm

ifdef OECORE_SDK_VERSION
LDLIBS += -L/usr/lib/
else
CC = gcc
CFLAGS += -DHOST
endif

PKGS += libpipewire-0.3 libspa-0.2
ifdef PKGS
  LDLIBS += $(shell pkg-config --libs $(PKGS))
  CFLAGS += $(shell pkg-config --cflags $(PKGS))
endif

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

# Enable all warnings:
CFLAGS += -Wall

# TODO: Extra warnings:
# CFLAGS += -Werror
CFLAGS += -W
CFLAGS += -Wextra
# CFLAGS += -Wpedantic
CFLAGS += -Wmissing-prototypes
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wvla
CFLAGS += -Wformat=2
CFLAGS += -Wmaybe-uninitialized
CFLAGS += -Wunused-parameter
CFLAGS += -Wunused-but-set-parameter
CFLAGS += -Wpointer-arith
CFLAGS += -Wfloat-equal
CFLAGS += -Wdisabled-optimization
# CFLAGS += -Winline
# CFLAGS += -Wbad-function-cast

# NOTE: Disabled warnings:
CFLAGS += -Wno-sign-compare

# FINAL=y
# ASAN=y
ifeq ($(FINAL), y)
  LDFLAGS += -s
  CFLAGS += -DNDEBUG -g0 -O2
else
  CFLAGS += -g3 -DDEBUG
  ifeq ($(ASAN), y)
    CFLAGS += -fsanitize=address -O1 -fno-omit-frame-pointer
    LDLIBS += -fsanitize=address
    CFLAGS += -O1
  endif
endif

.PHONY: all
all: $(PROGS)

.PHONY: debug
debug:
	@echo "*** Debug info:"
	@echo "Source-files:" $(SRCS)
	@echo "Object-files:" $(OBJS)
	@echo "Compiler-flags:" $(CFLAGS)
	@echo "Linker-flags:" $(LDFLAGS)
	@echo "Linker-libs:" $(LDLIBS)

$(PROGS): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

.PHONY: run
run: $(PROGS)
	@./$(PROGS)

.PHONY: valgrind
valgrind: $(PROGS)
	@valgrind \
		--tool=memcheck \
		--leak-check=yes \
		--show-reachable=yes \
		--num-callers=20 \
		--track-fds=yes \
		./$(PROGS)

.PHONY: cppcheck
cppcheck:
	@echo "*** Static code analysis"
	@cppcheck $(shell find . -name "*.[ch]") \
		--verbose --enable=all -DDEBUG=1 \
		--suppress=missingIncludeSystem \
		--suppress=unusedFunction

.PHONY: indent
indent:
	@echo "*** Formatting code"
	@clang-format $(shell find . -name "*.[ch]") \
		-style=file -i -fallback-style=none

.PHONY: clean
clean:
	$(RM) $(PROGS) $(OBJS)

.PHONY: channelplay
channelplay:
	@cat channel.raw | pw-cat --playback --rate=11025 --channels=1 --format=u8 -
