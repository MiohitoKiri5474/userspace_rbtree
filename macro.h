#include <stdint.h>


typedef uint8_t __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
typedef uint64_t __u64;

typedef __u8 __attribute__((__may_alias__)) __u8_alias_t;
typedef __u16 __attribute__((__may_alias__)) __u16_alias_t;
typedef __u32 __attribute__((__may_alias__)) __u32_alias_t;
typedef __u64 __attribute__((__may_alias__)) __u64_alias_t;

#define barrier() __asm__ __volatile__("" : : : "memory")
static __always_inline void __write_once_size(volatile void *p,
                                              void *res,
                                              int size)
{
    switch (size) {
    case 1:
        *(volatile __u8_alias_t *) p = *(__u8_alias_t *) res;
        break;
    case 2:
        *(volatile __u16_alias_t *) p = *(__u16_alias_t *) res;
        break;
    case 4:
        *(volatile __u32_alias_t *) p = *(__u32_alias_t *) res;
        break;
    case 8:
        *(volatile __u64_alias_t *) p = *(__u64_alias_t *) res;
        break;
    default:
        barrier();
        __builtin_memcpy((void *) p, (const void *) res, size);
        barrier();
    }
}

#define READ_ONCE(x)                                \
    ({                                              \
        union {                                     \
            typeof(x) __val;                        \
            char __c[1];                            \
        } __u = {.__c = {0}};                       \
        __read_once_size(&(x), __u.__c, sizeof(x)); \
        __u.__val;                                  \
    })

#define WRITE_ONCE(x, val)                           \
    ({                                               \
        union {                                      \
            typeof(x) __val;                         \
            char __c[1];                             \
        } __u = {.__val = (val)};                    \
        __write_once_size(&(x), __u.__c, sizeof(x)); \
        __u.__val;                                   \
    })

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:        the pointer to the member.
 * @type:       the type of the container struct this is embedded in.
 * @member:     the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member)                      \
    ({                                                       \
        const typeof(((type *) 0)->member) *__mptr = (ptr);  \
        (type *) ((char *) __mptr - offsetof(type, member)); \
    })
