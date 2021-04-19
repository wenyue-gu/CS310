
// this is a function the users of the test library provide to setup
// the state of the system before each test
extern void setup();

// asserts just return from the test function
// not super generalizable, but this is a toy
#define simple_assert(message, test) do { if (!(test)) return message; } while (0)

#define TEST_PASSED NULL
#define TESTER_NAME "UnitTestLab"

void add_test(char* (*test_func)());
void* run_test(void *test_to_run_void);
void run_all_tests();
