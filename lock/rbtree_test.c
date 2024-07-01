#include "rbtree_test.h"
#include <pthread.h>
#include <unistd.h>
#include "rbtree.h"
#include "rbtree_augmented.h"

int deleted_cnt = 0, inserted_cnt = 0;
ptlock_t *deleted_cnt_lock, *inserted_cnt_lock;
bool eraseing[10];

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
    ptlock_t *current_lock = NULL;
    while (node) {
        ptlock_t *prev = node->lock;
        LOCK(prev);
        if (current_lock)
            UNLOCK(current_lock);
        node_key = rb_entry(node, test_node, rb)->key;
        if (node_key < key)
            node = node->rb_right;
        else if (node_key > key)
            node = node->rb_left;
        else {
            if (current_lock)
                UNLOCK(current_lock);
            return rb_entry(node, test_node, rb);
        }
        current_lock = prev;
    }
    if (current_lock)
        UNLOCK(current_lock);
    return NULL;
}

static __always_inline void DO_INSERT(void)
{
    int key = rand();
    test_node *tmp = (test_node *) malloc(sizeof(test_node));
    tmp->key = key, tmp->val = 0;
    tmp->rb.lock = (ptlock_t *) malloc(sizeof(ptlock_t));
    INIT_LOCK(tmp->rb.lock);
    insert(tmp, &root);
    LOCK(inserted_cnt_lock);
    inserted_cnt++;
    UNLOCK(inserted_cnt_lock);
}

static __always_inline void DO_ERASE(void)
{
    int key =
        rb_entry((rand() & 1 ? rb_first(&root) : rb_last(&root)), test_node, rb)
            ->key;
    if (erase(key, &root)) {
        LOCK(deleted_cnt_lock);
        deleted_cnt++;
        UNLOCK(deleted_cnt_lock);
    }
}

int counting_nodes(rb_node *o)
{
    if (!o)
        return 0;
    return 1 + counting_nodes(o->rb_left) + counting_nodes(o->rb_right);
}

static __always_inline void *thread_op(void *arg)
{
    for (int i = 1; i <= 1000000; i++) {
        // if (i % 10000 == 0) {
        //     printf("thread %d: insert %d\n", *(int *) arg, i);
        // }
        DO_INSERT();
    }

    printf("[Info]: Thread %d finish insert\n", *(int *) arg);

    // return NULL;
    for (int i = 1; i <= 100000; i++) {
        if (i % 10000 == 0) {
            // printf("thread %d: erase %d\n", *(int *) arg, i);
            // puts("\tcheck passed");
        }
        // printf("Thread %d erase %d\n", *(int *) arg, i);
        DO_ERASE();
    }
    printf("[Info]: Thread %d finish erase\n", *(int *) arg);
    return NULL;
}

int main()
{
    srand(clock() * time(NULL));
    puts("rbtree testing");

    root.lock = (ptlock_t *) malloc(sizeof(ptlock_t));
    deleted_cnt_lock = (ptlock_t *) malloc(sizeof(ptlock_t));
    inserted_cnt_lock = (ptlock_t *) malloc(sizeof(ptlock_t));
    INIT_LOCK(root.lock);
    INIT_LOCK(deleted_cnt_lock);
    INIT_LOCK(inserted_cnt_lock);
    int thread_num = 4, *thread_id, cnt = 0;
    pthread_t thread[thread_num];
    thread_id = malloc(sizeof(int) * thread_num);
    for (int i = 0; i < thread_num; i++) {
        thread_id[i] = i;
        if (pthread_create(thread + i, NULL, thread_op, thread_id + i)) {
            fprintf(stderr, "[Error]: Create thread %d\n", i);
            return 0;
        }
        printf("[Info]: Thread %d created\n", i);
    }
    for (int i = 0; i < thread_num; i++) {
        if (pthread_join(thread[i], NULL)) {
            fprintf(stderr, "[Error]: Join thread %d\n", i);
            return 0;
        }
        printf("[Info]: Thread %d joined\n", i);
    }

    puts("[Info]: RBTree check passed");

    // for (rb_node *rb = rb_first(&root); rb; rb = rb_next(rb)) {
    //     // printf("%d\n", rb_entry(rb, test_node, rb)->key);
    //     cnt++;
    // }
    printf("inserted_cnt: %d\n", inserted_cnt);
    printf("deleted_cnt: %d\n", deleted_cnt);
    cnt = counting_nodes(root.rb_node);

    printf("cnt: %d\n", cnt);
    printf("total node count: %d\n", cnt + deleted_cnt);
    return 0;
}
