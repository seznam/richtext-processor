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
SRCS   = $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/json/*.c)
OBJS   = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

TARGET  = richtext

# Inspired by check and GNU Autotools - https://libcheck.github.io/check/
TESTDIR = tests
TESTS = $(patsubst $(TESTDIR)/%,%,\
                $(patsubst %.c,%, \
			$(wildcard $(TESTDIR)/check_*.c) \
			$(wildcard $(TESTDIR)/json/check_*.c) \
		)\
        )
check_PROGRAMS = $(TESTS)
check_CFLAGS = $(CFLAGS)
check_layout_resolver_SOURCES	= $(SRCDIR)/layout_resolver.c \
				  $(SRCDIR)/string.c $(SRCDIR)/vector.c \
				  $(SRCDIR)/parser.c $(SRCDIR)/lexer.c \
				  $(TESTDIR)/unit.c \
				  $(TESTDIR)/check_layout_resolver.c
check_layout_resolver_CFLAGS	= $(check_CFLAGS)
check_lexer_SOURCES		= $(SRCDIR)/lexer.c $(SRCDIR)/string.c \
				  $(SRCDIR)/vector.c $(TESTDIR)/unit.c \
				  $(TESTDIR)/check_lexer.c
check_lexer_CFLAGS		= $(check_CFLAGS)
check_parser_SOURCES		= $(SRCDIR)/parser.c $(SRCDIR)/lexer.c \
				  $(SRCDIR)/string.c $(SRCDIR)/vector.c \
				  $(TESTDIR)/unit.c $(TESTDIR)/check_parser.c
check_parser_CFLAGS		= $(check_CFLAGS)
check_string_SOURCES		= $(SRCDIR)/string.c $(TESTDIR)/unit.c \
				  $(TESTDIR)/check_string.c
check_string_CFLAGS		= $(check_CFLAGS)
check_vector_SOURCES		= $(SRCDIR)/vector.c $(TESTDIR)/unit.c \
				  $(TESTDIR)/check_vector.c
check_vector_CFLAGS		= $(check_CFLAGS)
json_check_ast_node_SOURCES	= $(SRCDIR)/vector.c $(SRCDIR)/string.c \
				  $(SRCDIR)/parser.c \
				  $(SRCDIR)/json/json_value.c \
				  $(SRCDIR)/json/json_encoder.c \
				  $(SRCDIR)/json/ast_node.c $(TESTDIR)/unit.c \
				  $(TESTDIR)/json/check_ast_node.c
json_check_ast_node_CFLAGS	= $(check_CFLAGS)
json_check_json_encoder_SOURCES = $(SRCDIR)/json/json_encoder.c \
				  $(SRCDIR)/string.c $(SRCDIR)/vector.c \
				  $(SRCDIR)/json/json_value.c \
				  $(TESTDIR)/unit.c \
				  $(TESTDIR)/json/check_json_encoder.c
json_check_json_encoder_CLAGS   = $(check_CFLAGS)
json_check_json_value_SOURCES	= $(SRCDIR)/json/json_value.c \
				  $(SRCDIR)/vector.c $(SRCDIR)/string.c \
				  $(TESTDIR)/unit.c \
				  $(TESTDIR)/json/check_json_value.c
json_check_json_value_CFLAGS	= $(check_CFLAGS)
json_check_layout_content_alignment_SOURCES =\
	$(SRCDIR)/json/layout_content_alignment.c $(SRCDIR)/json/json_value.c \
	$(SRCDIR)/json/json_encoder.c $(SRCDIR)/layout_resolver.c \
	$(SRCDIR)/parser.c $(SRCDIR)/vector.c $(SRCDIR)/string.c \
	$(TESTDIR)/unit.c $(TESTDIR)/json/check_layout_content_alignment.c
json_check_layout_content_alignment_CFLAGS = $(check_CFLAGS)
json_check_layout_line_SOURCES	= $(SRCDIR)/json/layout_line.c \
				  $(SRCDIR)/json/layout_line_segment.c \
				  $(SRCDIR)/json/layout_content_alignment.c \
				  $(SRCDIR)/json/ast_node.c \
				  $(SRCDIR)/json/json_value.c \
				  $(SRCDIR)/json/json_encoder.c \
				  $(SRCDIR)/vector.c $(SRCDIR)/string.c \
				  $(SRCDIR)/parser.c \
				  $(SRCDIR)/layout_resolver.c \
				  $(TESTDIR)/unit.c \
				  $(TESTDIR)/json/check_layout_line.c
json_check_layout_line_CFLAGS	= $(check_CFLAGS)	
json_check_layout_line_segment_SOURCES =\
	$(SRCDIR)/json/layout_line_segment.c $(SRCDIR)/json/json_value.c \
	$(SRCDIR)/string.c $(SRCDIR)/vector.c $(SRCDIR)/layout_resolver.c \
	$(SRCDIR)/parser.c $(SRCDIR)/json/ast_node.c \
	$(SRCDIR)/json/layout_content_alignment.c \
	$(SRCDIR)/json/json_encoder.c $(TESTDIR)/unit.c \
	$(TESTDIR)/json/check_layout_line_segment.c
json_check_layout_line_segment_CFLAGS = $(check_CFLAGS)

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
		$(SRCDIR)/json/*.h~ \
		$(SRCDIR)/json/*.c~ \
		$(TESTDIR)/*.h~ \
		$(TESTDIR)/*.c~ \
		$(TESTDIR)/json/*.h~ \
		$(TESTDIR)/json/*.c~

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
	$(foreach program,$(check_PROGRAMS), \
		$(CC) $($(subst /,_,$(program))_CFLAGS) $(LDFLAGS) \
			-o /tmp/richtext-processor/tests/$(program) \
			$($(subst /,_,$(program))_SOURCES) && \
	) true

indent:
	indent $(INDENTFLAGS) \
		$(SRCDIR)/*.h \
		$(SRCDIR)/*.c \
		$(SRCDIR)/json/*.h \
		$(SRCDIR)/json/*.c \
		$(TESTDIR)/*.h \
		$(TESTDIR)/*.c \
		$(TESTDIR)/json/*.c

$(DEPDIR) $(OBJDIR):
	@mkdir -p $@ $@/json

DEPFILES := $(SRCS:$(SRCDIR)/%.c=$(DEPDIR)/%.d)
$(DEPFILES):

include $(wildcard $(DEPFILES))
