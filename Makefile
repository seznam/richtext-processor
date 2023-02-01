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

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

TARGET  = richtext

.PHONY: all

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
%.o: %.c $(DEPDIR)/%.d | $(DEPDIR)
	$(COMPILE.c) $(OUTPUT_OPTION) $<

clean:
	$(RM) $(OBJS)
	$(RM) -r $(DEPDIR)
	$(RM) *.h~ *.c~

distclean:
	$(RM) $(TARGET)

indent:
	indent $(INDENTFLAGS) *.h *.c

$(DEPDIR):
	@mkdir -p $@

DEPFILES := $(SRCS:%.c=$(DEPDIR)/%.d)
$(DEPFILES):

include $(wildcard $(DEPFILES))
