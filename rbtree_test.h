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
static test_node nodes[NODES];

__attribute__((unused)) static void insert(test_node *node, rb_root *root)
{
    rb_node **new_node = &root->rb_node, *parent = NULL;
    uint32_t key = node->key;

    while (*new_node) {
        parent = *new_node;

        new_node =
            (key < rb_entry(parent, test_node, rb)->key ? &parent->rb_left
                                                        : &parent->rb_right);
    }
    rb_link_node(&node->rb, parent, new_node);
    rb_insert_color(&node->rb, root);
}

__attribute__((unused)) static inline void erase(test_node *node, rb_root *root)
{
    rb_erase(&node->rb, root);
}

__attribute__((unused)) static inline uint32_t augment_recompute(
    test_node *node,
    bool tmp)
{
    uint32_t max = node->val, child_augmented;
    if (node->rb.rb_left) {
        child_augmented = rb_entry(node->rb.rb_left, test_node, rb)->augmented;
        if (max < child_augmented)
            max = child_augmented;
    }
    if (node->rb.rb_right) {
        child_augmented = rb_entry(node->rb.rb_right, test_node, rb)->augmented;
        if (max < child_augmented)
            max = child_augmented;
    }
    return max;
}

RB_DECLARE_CALLBACKS(static,
                     augment_callbacks,
                     test_node,
                     rb,
                     augmented,
                     augment_recompute);

__attribute__((unused)) static void insert_augmented(test_node *node,
                                                     rb_root *root)
{
    rb_node **new_node = &root->rb_node, *rb_parent = NULL;
    uint32_t key = node->key, val = node->val;
    test_node *parent;

    while (*new_node) {
        rb_parent = *new_node;
        parent = rb_entry(rb_parent, test_node, rb);
        if (parent->augmented < val)
            parent->augmented = val;
        new_node =
            (key < parent->key ? &parent->rb.rb_left : &parent->rb.rb_right);
    }

    node->augmented = val;
    rb_link_node(&node->rb, rb_parent, new_node);
    rb_insert_augmented(&node->rb, root, &augment_callbacks);
}

__attribute__((unused)) static void erase_augmented(test_node *node,
                                                    rb_root *root)
{
    rb_erase_augmented(&node->rb, root, &augment_callbacks);
}

__attribute__((unused)) static void init(void)
{
    for (int i = 0; i < NODES; i++) {
        nodes[i].key = rand();
        nodes[i].val = rand();
    }
}

__attribute__((unused)) static int is_red(rb_node *rb)
{
    return !(rb->__rb_parent_color & 1);
}

__attribute__((unused)) static int black_path_count(rb_node *rb)
{
    int cnt = 0;
    while (rb) {
        cnt += !is_red(rb);
        rb = rb_parent(rb);
    }
    return cnt;
}

__attribute__((unused)) static void check_postorder_foreach(int nr_nodes)
{
    test_node *cur, *n;
    int cnt = 0;
    rbtree_postorder_for_each_entry_safe(cur, n, &root, rb) cnt++;
}

__attribute__((unused)) static void check_postorder(int nr_nodes)
{
    int cnt = 0;
    for (rb_node *rb = rb_first_postorder(&root); rb;
         rb = rb_next_postorder(rb))
        cnt++;
}

__attribute__((unused)) static void check(int nr_nodes)
{
    int cnt = 0;
    for (rb_node *rb = rb_first(&root); rb; rb = rb_next(rb)) {
        cnt++;
    }

    check_postorder(nr_nodes);
    check_postorder_foreach(nr_nodes);
}

__attribute__((unused)) static void check_augmented(int nr_nodes)
{
    check(nr_nodes);
    for (rb_node *rb = rb_first(&root); rb; rb = rb_next(rb)) {
    }
}
