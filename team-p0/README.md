
This is an updated base directory for the p0 repo.  It includes:

* Modified base dmm.h, dmm.c, and Makefile.  These are streamlined and annotated to make them
easier to understand.  Also, dmm.c is modified to use mmap instead of sbrk, so it will build without
warnings on platforms that have deprecated sbrk.

* Exerciser tests.  These tests are reasonably comprehensive and similar to those used on the AG.

You can say:

```
make test >/dev/null
```

To run all the tests, throw away the status output, and just see if you passed.

Please do not ask what the tests do.  If you want to know, read the code!

Chase 8/24/20
