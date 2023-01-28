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
		*.h \
		*.c \
		-o richtext

# Linux code style
# See https://www.gnu.org/software/indent/manual/indent.html
indent:
	indent \
		-nbad \
		-bap \
		-nbc \
		-bbo \
		-hnl \
		-br \
		-brs \
		-c33 \
		-cd33 \
		-ncdb \
		-ce \
		-ci4 \
		-cli0 \
		-d0 \
		-di1 \
		-nfc1 \
		-i8 \
		-ip0 \
		-l80 \
		-lp \
		-npcs \
		-nprs \
		-npsl \
		-sai \
		-saf \
		-saw \
		-ncs \
		-nsc \
		-sob \
		-nfca \
		-cp33 \
		-ss \
		-ts8 \
		-il1 \
		*.h \
		*.c
	rm *.h~ *.c~
