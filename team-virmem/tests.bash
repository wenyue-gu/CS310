do_test_case() {
    ./paged_forth.bin $1 > rawout.txt
    error=$?
    if [ $error -ne 0 ]
    then
        echo TC$1 failed: paged forth returned nonzero
        return

    fi
      
    grep PMC-TC rawout.txt > out.txt
    diff out.txt TC$1.output
    error=$?
    if [ $error -eq 0 ]
    then
        echo TC$1 passed
    else
        echo TC$1 failed: does not match solution
    fi
    
}

make paged_forth.bin
do_test_case 1
do_test_case 2
do_test_case 3
do_test_case 4
