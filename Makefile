# Automatic dependency generation inspired by
# https://make.mad-scientist.net/papers/advanced-auto-dependency-generation/

SHELL  = /bin/sh

CFLAGS      = -ansi -Wpedantic -Wall -Wextra -Wtraditional -Wshadow \
              -Wpointer-arith -Wstrict-prototypes \
              -Wdeclaration-after-statement -Wcast-qual \
							-g
LDFLAGS     = -g
LDLIBS      =
DEPDIR      = .deps
DEPFLAGS    = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c

# Linux code style
# See https://www.gnu.org/software/indent/manual/indent.html
INDENTFLAGS = -nbad -bap -nbc -bbo -hnl -br -brs -c33 -cd33 -ncdb -ce -ci4 \
              -cli0 -d0 -di1 -nfc1 -i8 -ip0 -l80 -lp -npcs -nprs -npsl -sai \
              -saf -saw -ncs -nsc -sob -nfca -cp33 -ss -ts8 -il1

SRCDIR = src
OBJDIR = obj
SRCS   = $(wildcard $(SRCDIR)/*.c)
OBJS   = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

TARGET  = richtext

# Inspired by check and GNU Autotools - https://libcheck.github.io/check/
# TODO: integrate MinUnit https://jera.com/techinfo/jtns/jtn002
TESTS = $(patsubst tests/%,%,$(patsubst %.c,%,$(wildcard tests/check_*.c)))
check_PROGRAMS = $(TESTS)
check_CFLAGS = $(CFLAGS)
check_string_SOURCES = src/string.c tests/unit.c tests/check_string.c
check_string_CFLAGS  = $(check_CFLAGS)
check_vector_SOURCES = src/vector.c tests/unit.c tests/check_vector.c
check_vector_CFLAGS  = $(check_CFLAGS)
check_lexer_SOURCES  = src/lexer.c src/string.c src/vector.c tests/unit.c tests/check_lexer.c
check_lexer_CFLAGS   = $(check_CFLAGS)

.PHONY: all

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPDIR)/%.d | $(DEPDIR) $(OBJDIR)
	$(COMPILE.c) $(OUTPUT_OPTION) $<

clean:
	$(RM) $(OBJS)
	$(RM) -r $(DEPDIR)
	$(RM) $(SRCDIR)/*.h~ $(SRCDIR)/*.c~ tests/*.h~ tests/*.c~

distclean:
	$(RM) $(TARGET)

test: check
	$(foreach test,$(TESTS),\
		echo "$(test)" && /tmp/richtext-processor/tests/$(test) && echo &&\
	) true
	$(RM) -r /tmp/richtext-processor/tests/

check:
	@mkdir -p /tmp/richtext-processor/tests/
	$(foreach program,$(check_PROGRAMS), \
		$(CC) $($(program)_CFLAGS) $(LDFLAGS) \
			-o /tmp/richtext-processor/tests/$(program) $($(program)_SOURCES) && \
	) true

indent:
	indent $(INDENTFLAGS) $(SRCDIR)/*.h $(SRCDIR)/*.c tests/*.h tests/*.c

$(DEPDIR) $(OBJDIR):
	@mkdir -p $@

DEPFILES := $(SRCS:$(SRCDIR)/%.c=$(DEPDIR)/%.d)
$(DEPFILES):

include $(wildcard $(DEPFILES))
