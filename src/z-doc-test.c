#include "z-doc.h"

#include <assert.h>

void test(int width)
{
    doc_ptr doc = doc_alloc(width);

    doc_read_file(doc, stdin);
    doc_write_file(doc, stdout);

    doc_free(doc);
}

int main(int argc, char *argv[])
{
    int width = 72, i;

    for (i = 1; i < argc; i++)
    {
        if ( strcmp("-w", argv[i]) == 0
          || strcmp("--width", argv[i]) == 0 )
        {
            i++;
            if (i < argc)
                width = atoi(argv[i]);
        }
    }
    test(width);
    return 0;
}

/*
gcc -g c-string.c str-map.c int-map.c z-doc.c z-doc-test.c
cat ../lib/help/tang.txt | ./a.out --width 80 | less

> time cat *.c | ./a.out --width 80 > foo.txt; ls -lah foo.txt
real    0m1.011s
user    0m0.954s
sys     0m0.120s
-rw-r--r-- 1 chris chris 25M Sep 21 14:22 foo.txt

> time cat *.c > foo.txt; ls -lah foo.txt
real    0m0.030s
user    0m0.000s
sys     0m0.032s
-rw-r--r-- 1 chris chris 8.1M Sep 21 14:26 foo.txt
*/

