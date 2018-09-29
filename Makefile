CC= gcc
CFLAGS = -std=c99 -g3 -Wall -pedantic
HWK = /c/cs323/Hwk2

parsnip: parsnip.c /c/cs323/Hwk2/mainParsnip.o /c/cs323/Hwk2/tokenize.o
	${CC} ${CFLAGS} -I/c/cs323/Hwk2 -o $@ $^

