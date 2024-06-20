#include "rbtree.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "rbtree_augmented.h"

#define NODES 10000
#define PERF_LOOPS 100

struct test_node {
    uint32_t key;
    struct rb_node rb;

    uint32_t val, augmented;
};

static struct rb_root root = RB_ROOT;
static struct test_node nodes[NODES];

int main()
{
    srand(time(NULL));
    int i, j;
    struct timeval begin, end;
    double tm;
    puts("rbtree testing");
    return 0;
}
