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
SRCS   = $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/*/*.c)
OBJS   = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

TARGET  = richtext

# Inspired by check and GNU Autotools - https://libcheck.github.io/check/
TESTDIR = tests
TESTS = $(patsubst $(TESTDIR)/%,%,\
                $(patsubst %.c,%, \
			$(wildcard $(TESTDIR)/check_*.c) \
			$(wildcard $(TESTDIR)/*/check_*.c) \
		)\
        )
check_PROGRAMS	= $(TESTS)
check_CFLAGS	= $(CFLAGS)
check_SOURCES	= $(SRCDIR)/*.c $(SRCDIR)/json/*.c $(SRCDIR)/utf8/*.c \
		  $(TESTDIR)/unit.c

.PHONY: all

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPDIR)/%.d | $(DEPDIR) $(OBJDIR)
	$(COMPILE.c) $(OUTPUT_OPTION) $<

clean:
	$(RM) -r $(OBJDIR)
	$(RM) -r $(DEPDIR)
	$(RM) \
		$(SRCDIR)/*.h~ \
		$(SRCDIR)/*.c~ \
		$(SRCDIR)/*/*.h~ \
		$(SRCDIR)/*/*.c~ \
		$(TESTDIR)/*.h~ \
		$(TESTDIR)/*.c~ \
		$(TESTDIR)/*/*.h~ \
		$(TESTDIR)/*/*.c~

distclean:
	$(RM) $(TARGET)

test: check
	$(foreach test,$(TESTS),\
		echo "$(test)" && /tmp/richtext-processor/tests/$(test) && \
		echo &&\
	) true
	$(RM) -r /tmp/richtext-processor/tests/

check:
	@mkdir -p /tmp/richtext-processor/tests/
	@mkdir -p /tmp/richtext-processor/tests/json/
	@mkdir -p /tmp/richtext-processor/tests/utf8/
	$(foreach program,$(check_PROGRAMS), \
		echo "Compiling $(program)..." && \
		$(CC) \
			$(if $($(subst /,_,$(program))_CFLAGS), \
				$($(subst /,_,$(program))_CFLAGS) \
			, \
				$(check_CFLAGS) \
			) \
			$(LDFLAGS) \
			-o /tmp/richtext-processor/tests/$(program) \
			$(if $($(subst /,_,$(program))_SOURCES), \
				$($(subst /,_,$(program))_SOURCES) \
			, \
				$(check_SOURCES) $(TESTDIR)/$(program).c \
			) \
			&& \
	) true

indent:
	indent $(INDENTFLAGS) \
		$(SRCDIR)/*.h \
		$(SRCDIR)/*.c \
		$(SRCDIR)/*/*.h \
		$(SRCDIR)/*/*.c \
		$(TESTDIR)/*.h \
		$(TESTDIR)/*.c \
		$(TESTDIR)/*/*.c

$(DEPDIR) $(OBJDIR):
	@mkdir -p $& $@/cli
	@mkdir -p $@ $@/json
	@mkdir -p $@ $@/utf8

DEPFILES := $(SRCS:$(SRCDIR)/%.c=$(DEPDIR)/%.d)
$(DEPFILES):

include $(wildcard $(DEPFILES))
