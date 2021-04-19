FLAGS = -std=gnu++11 -ggdb -pthread -march=native

all: queue_test.out authors_condition_var.out authors_futex.out

queue_test.out: queue.o queue_test.o CuTest.o
	g++ $(FLAGS) $^ -o $@

queue_ag350.out: queue.o queue_test_ag350.c
	g++ $(FLAGS) $^ -o $@

CuTest.o: CuTest.c
	g++ -c $^ -o $@

queue.o: queue.c queue.h
	g++ $(FLAGS) -c queue.c -o $@

queue_test.o: queue_test.c queue.h
	g++ $(FLAGS) -c queue_test.c -o $@

authors_condition_var.out: authors.c barrier_condition.c
	g++ $(FLAGS) $^ -o $@

authors_futex.out: authors.c barrier_futex.c futex.c
	g++ $(FLAGS) $^ -o $@

authors_futex_solution.out: authors.c barrier_futex_solution.c futex.c
	g++ $(FLAGS) $^ -o $@


clean:
	rm -rf *.out *.o
