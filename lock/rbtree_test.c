#include "rbtree_test.h"
#include <pthread.h>
#include "rbtree.h"

void print_rbtree(void)
{
    for (rb_node *rb = rb_first(&root); rb; rb = rb_next(rb)) {
        printf("%d ", rb_entry(rb, test_node, rb)->key);
    }
    putchar('\n');
}

test_node *find(int key)
{
    rb_node *node = root.rb_node;
    int node_key = 0;
    while (node) {
        node_key = rb_entry(node, test_node, rb)->key;
        if (node_key < key)
            node = node->rb_right;
        else if (node_key > key)
            node = node->rb_left;
        else
            return rb_entry(node, test_node, rb);
    }
    return NULL;
}

static __always_inline void DO_INSERT(void)
{
    int key = rand();
    test_node *tmp = (test_node *) malloc(sizeof(test_node));
    tmp->key = key, tmp->val = 0;
    insert(tmp, &root);
}

static __always_inline void *thread_op(void *arg)
{
    for (int i = 0; i < 1000000; i++) {
        if (i % 10000 == 0) {
            printf("thread %d: %d\n", *(int *) arg, i);
        }
        DO_INSERT();
    }
    return NULL;
}

int main()
{
    srand(clock() * time(NULL));
    puts("rbtree testing");

    int thread_num = 1, *thread_id, cnt = 0;
    pthread_t thread[thread_num];
    thread_id = malloc(sizeof(int) * thread_num);
    for (int i = 0; i < thread_num; i++) {
        thread_id[i] = i;
        if (pthread_create(thread + i, NULL, thread_op, thread_id + i)) {
            fprintf(stderr, "[Error]: Create thread %d\n", i);
            return 0;
        }
    }
    for (int i = 0; i < thread_num; i++) {
        if (pthread_join(thread[i], NULL)) {
            fprintf(stderr, "[Error]: Join thread %d\n", i);
            return 0;
        }
    }

    for (rb_node *rb = rb_first(&root); rb; rb = rb_next(rb)) {
        cnt++;
    }

    printf("%d\n", cnt);

    return 0;
}
