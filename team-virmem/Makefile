FLAGS   := -no-pie -ggdb -Wall

all: jonesforth.bin paged_forth.bin

forth_embed.o: forth/forth_embed.c forth/forth_embed.h 
	gcc $(FLAGS) -c forth/forth_embed.c -o forth_embed.o

jonesforth.o: forth/myjf.S
	gcc $(FLAGS) -c forth/myjf.S -o jonesforth.o

jonesforth.bin: forth/jf_intepret.c forth_embed.o jonesforth.o
	gcc $(FLAGS) -o $@ forth_embed.o jonesforth.o forth/jf_intepret.c

paged_forth_lib.o: paged_forth_lib.c paged_forth.h
	gcc $(FLAGS) -c -o $@ paged_forth_lib.c

paged_forth.bin: paged_forth_lib.o forth_embed.o jonesforth.o paged_forth.c
	gcc $(FLAGS) -o $@ $^

paged_forth_solution.bin: forth_embed.o jonesforth.o paged_forth_lib_solution.c paged_forth.c
	gcc $(FLAGS) -o $@ $^

interactive: jonesforth.bin
	./jonesforth.bin forth/jonesforth.f $(PROG)

clean:
	rm -f *.bin *.dat *~ core .test_* *.o rawout.txt out.txt
