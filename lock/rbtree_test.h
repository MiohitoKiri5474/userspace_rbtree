#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "rbtree.h"
#include "rbtree_augmented.h"

#define NODES 10000
#define PERF_LOOPS 100
#define CHECK_LOOPS 10

typedef struct {
    struct rb_node rb;
    uint32_t augmented, val, key;
} test_node;

typedef struct rb_node rb_node;
typedef struct rb_root rb_root;

static rb_root root = RB_ROOT;

__attribute__((unused)) static void insert(test_node *node, rb_root *root)
{
    rb_node **new_node = &root->rb_node, *parent = NULL;
    uint32_t key = node->key;
    ptlock_t *current_lock = NULL;

    while (*new_node) {
        parent = *new_node;
        ptlock_t *prev = (*new_node)->lock;
        LOCK(prev);
        if (current_lock)
            UNLOCK(current_lock);

        new_node =
            (key < rb_entry(parent, test_node, rb)->key ? &parent->rb_left
                                                        : &parent->rb_right);
        current_lock = prev;
    }
    rb_link_node(&node->rb, parent, new_node);
    rb_insert_color(&node->rb, root);
    if (current_lock)
        UNLOCK(current_lock);
}

__attribute__((unused)) static inline bool erase(int __key, rb_root *root)
{
    rb_node *target = root->rb_node, *parent = NULL;
    ptlock_t *current_lock = NULL, *parent_lock = NULL;

    while (target) {
        parent = target;
        ptlock_t *prev = parent->lock;

        LOCK(prev);
        if (parent_lock)
            UNLOCK(parent_lock);
        parent_lock = current_lock;


        int key = rb_entry(parent, test_node, rb)->key;
        if (__key == key) {
            current_lock = prev;
            break;
        }

        target = (__key < key ? parent->rb_left : parent->rb_right);
        current_lock = prev;
    }
    if (!target || rb_entry_safe(target, test_node, rb)->key != __key) {
        if (current_lock)
            UNLOCK(current_lock);
        if (parent_lock)
            UNLOCK(parent_lock);
        return false;
    }
    if (current_lock)
        UNLOCK(current_lock);
    rb_erase(target, root);
    if (parent_lock)
        UNLOCK(parent_lock);
    return true;
}

static __always_inline test_node *find(int key, rb_node *root)
{
    rb_node *node = root;
    int node_key = 0;
    ptlock_t *current_lock = NULL;

    while (node) {
        ptlock_t *prev = node->lock;
        LOCK(prev);
        if (current_lock)
            UNLOCK(current_lock);

        node_key = rb_entry(node, test_node, rb)->key;

        if (key == node_key) {
            UNLOCK(prev);
            if (current_lock)
                UNLOCK(current_lock);
            return rb_entry(node, test_node, rb);
        }
        node = (node_key < key ? node->rb_left : node->rb_right);

        current_lock = prev;
    }
    if (current_lock)
        UNLOCK(current_lock);
    return NULL;
}
