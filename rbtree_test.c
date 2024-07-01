#include "rbtree_test.h"

void print_rbtree(void)
{
    for (rb_node *rb = rb_first(&root); rb; rb = rb_next(rb)) {
        printf("%d ", rb_entry(rb, test_node, rb)->key);
    }
    putchar('\n');
}

static test_node *find(int key)
{
    rb_node *node = root.rb_node;
    int node_key = 0;
    while (node) {
        node_key = rb_entry(node, test_node, rb)->key;

        if (node_key == key)
            return rb_entry(node, test_node, rb);

        node = (node_key < key ? node->rb_left : node->rb_right);
    }
    return NULL;
}

int main()
{
    srand(time(NULL));
    puts("rbtree testing");

    for (int i = 10; i >= 0; i--) {
        test_node *tmp = (test_node *) malloc(sizeof(test_node));
        tmp->key = rand() % 100, tmp->val = 0;
        insert(tmp, &root);
    }

    print_rbtree();

    puts("finish insert");

    for (int i = 0, in, types; i < 10000000; i++) {
        if (scanf("%d %d", &types, &in)) {
            if (types == 1) {
                printf(find(in) ? "Yes\n" : "No\n");
            } else if (types == 2) {
                test_node *tmp = find(in);
                if (tmp) {
                    erase(tmp, &root);
                    puts("erased");
                    print_rbtree();
                } else {
                    puts("cannot find the key value");
                }
            }
        }
    }

    return 0;
}
