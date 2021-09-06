# CC = arm-linux-gcc
CFLAGS = -std=c99 -Wall -g
LIBS =
LDFLAGS +=

target = ./test
objects = test_nodebuf.o nodebuf.o

build: ${objects}
	${CC} -o ${target} ${CFLAGS} ${CXXFLAGS} $^ ${LIBS} ${LDFLAGS}

run: build
	${target}

clean:
	# rm -rf *.o
	find . -name "*.o" | xargs rm -f
	rm -f ${target}

all: clean build

.PHONY: all build clean run
