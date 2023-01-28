SHELL=/bin/sh

richtext:
	gcc \
		-ansi \
		-Wpedantic \
		-Wall \
		-Wextra \
		-Wtraditional \
		-Wshadow \
		-Wpointer-arith \
		-Wstrict-prototypes \
		-Wdeclaration-after-statement \
		-Wcast-qual \
		richtext.c \
		-o richtext
