all: simple_tests.bin autotest1.bin autotest2.bin autotest3.bin autotest4.bin autotest5.bin

simple_tests.bin: simple_test_lib.c simple_test_lib.h main.c
	gcc -g -Wall -o simple_tests.bin simple_test_lib.c main.c -pthread

autotest%.bin: autotest%.c simple_test_lib.c simple_test_lib.h 
	gcc -g -Wall -o $@ $< simple_test_lib.c -pthread

solutions: autotest1_solution.bin autotest2_solution.bin autotest3_solution.bin autotest4_solution.bin autotest5_solution.bin

autotest%_solution.bin: autotest%.c simple_test_lib.c simple_test_lib.h 
	gcc -g -Wall -o $@ $< simple_test_solution_lib.c -pthread

clean:
	rm -f *.bin *.o out.txt rawout.txt
