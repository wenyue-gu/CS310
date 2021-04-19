do_test_case() {
    ./autotest$1.bin > rawout.txt
    error=$?
    if [ $error -ne 0 ]
    then
        echo ./autotest$1.bin failed - bad return code
        return

    fi
      
    grep UnitTestLab rawout.txt > out.txt
    diff out.txt TC$1.output
    error=$?
    if [ $error -eq 0 ]
    then
        ./autotest$1.bin passed
    else
        ./autotest$1.bin failed - does not match solution
    fi
}


rm -f *.bin *.dat *~ core .test_* *.o rawout.txt out.txt

gcc -g -Wall -o autotest1.bin autotest1.c simple_test_lib.c
gcc -g -Wall -o autotest2.bin autotest2.c simple_test_lib.c
gcc -g -Wall -o autotest3.bin autotest3.c simple_test_lib.c
gcc -g -Wall -o autotest4.bin autotest4.c simple_test_lib.c
gcc -g -Wall -o autotest5.bin autotest5.c simple_test_lib.c

do_test_case 1
do_test_case 2
do_test_case 3
do_test_case 4
do_test_case 5
