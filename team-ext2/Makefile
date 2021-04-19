CC= gcc
CFLAGS= -Wall -ggdb

all: testcode filereader 451_filesystem.ext2

451_filesystem.ext2: binary.zip.base64
	base64 -d binary.zip.base64 > binary.zip
	unzip binary.zip

testcode: ext2access.o testcode.o
	${CC} -o $@ ${CFLAGS} $^ -lm

filereader: ext2access.o filereader.o
	${CC} -o $@ ${CFLAGS} $^ -lm

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f filereader testcode *.o *~ #*
