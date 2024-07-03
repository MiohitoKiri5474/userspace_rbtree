#include <pthread.h>
#include <unistd.h>

#include "include/rbtree.h"
#include "include/rbtree_test.h"

#define maxN 1000000
int deleted_cnt = 0, inserted_cnt = 0;
ptlock_t *deleted_cnt_lock, *inserted_cnt_lock;

static __always_inline void DO_INSERT(void)
{
    int key = rand() % maxN;
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
    int key = rand() % maxN;
    if (erase(key, &root)) {
        LOCK(deleted_cnt_lock);
        deleted_cnt++;
        UNLOCK(deleted_cnt_lock);
    }
}

static __always_inline void DO_QUERY(void)
{
    find(rand() % maxN, root.rb_node);
}

int counting_nodes(rb_node *o)
{
    if (!o)
        return 0;
    return 1 + counting_nodes(o->rb_left) + counting_nodes(o->rb_right);
}

static __always_inline void *thread_op(void *arg)
{
    for (int i = 1; i <= maxN; i++) {
        DO_INSERT();
    }
    printf("[Info]: Thread %d finish insert\n", *(int *) arg);

    for (int i = 1; i <= (maxN >> 2); i++) {
        DO_ERASE();
    }
    printf("[Info]: Thread %d finish erase\n", *(int *) arg);

    for (int i = 1; i <= maxN; i++) {
        DO_QUERY();
    }
    printf("[Info]: Thread %d finish query\n", *(int *) arg);

    for (int i = 0; i <= maxN; i++) {
        switch (rand() % 3) {
        case 0:
            DO_INSERT();
            break;
        case 1:
            DO_ERASE();
            break;
        case 2:
            DO_QUERY();
            break;
        default:
            break;
        }
    }
    printf("[Info]: Thread %d finish mixed operation\n", *(int *) arg);
    return NULL;
}

int main()
{
    srand(clock() * time(NULL));
    puts("rbtree testing");

    root.rb_node = NULL;
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
        thread_id[i] = i + 1;
        if (pthread_create(thread + i, NULL, thread_op, thread_id + i)) {
            fprintf(stderr, "[Error]: Create thread %d\n", i + 1);
            return 0;
        }
        printf("[Info]: Thread %d created\n", i + 1);
    }
    for (int i = 0; i < thread_num; i++) {
        if (pthread_join(thread[i], NULL)) {
            fprintf(stderr, "[Error]: Join thread %d\n", i + 1);
            return 0;
        }
        printf("[Info]: Thread %d joined\n", i + 1);
    }

    printf("inserted_cnt: %d\n", inserted_cnt);
    printf("deleted_cnt: %d\n", deleted_cnt);
    cnt = counting_nodes(root.rb_node);

    printf("cnt: %d\n", cnt);
    printf("total node count: %d\n", cnt + deleted_cnt);
    printf("diff: %d\n", cnt + deleted_cnt - inserted_cnt);
    return 0;
}
