#ifndef _CY_H
#define _CY_H

// NOTE(cya): a lot of this library is taken from GingerBill's gb library
// https://github.com/gingerBill/gb/blob/master/gb.h

#if defined(_WIN32) || defined(_WIN64)
    #define CY_OS_WINDOWS 1
    #include <windows.h>
#elif defined(__APPLE__) && defined(__MACH__)
    #define CY_OS_MACOSX 1
#elif defined(__unix__)
    #define CY_OS_UNIX 1
    #define _DEFAULT_SOURCE

    #if defined(__linux__)
        #define CY_OS_LINUX 1
    #elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
        #define CY_OS_FREEBSD 1
    #else
        #error Unsupported UNIX-like OS
    #endif
#else
    #error Unsupported OS
#endif

/* --------------------------------- Types ---------------------------------- */
#ifdef _MSC_VER
    #if _MSC_VER < 1300
    typedef signed char i8;
    typedef signed short i16;
    typedef signed int i32;

    typedef unsigned char u8;
    typedef unsigned short u16;
    typedef unsigned int u32;
    #else
    typedef signed __int8 i8;
    typedef signed __int16 i16;
    typedef signed __int32 i32;

    typedef unsigned __int8 u8;
    typedef unsigned __int16 u16;
    typedef unsigned __int32 u32;
    #endif

typedef signed __int64 i64;
typedef unsigned __int64 u64;
#else
    #include <stdint.h>

    typedef int8_t i8;
    typedef int16_t i16;
    typedef int32_t i32;
    typedef int64_t i64;

    typedef uint8_t u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;
#endif

#if defined(CY_OS_UNIX)
    #include <stddef.h>
#endif

typedef ptrdiff_t isize;
typedef size_t usize;

typedef intptr_t intptr;
typedef uintptr_t uintptr;

typedef i8 b8;
typedef i16 b16;
typedef i32 b32;

#define true (0 == 0)
#define false (0 != 0)

typedef i32 Rune; // Unicode codepoint
#define CY_RUNE_INVALID (Rune)(0xfffd)
#define CY_RUNE_MAX     (Rune)(0x0010ffff)
#define CY_RUNE_BOM     (Rune)(0xfeff)
#define CY_RUNE_EOF     (Rune)(-1)

// NOTE(cya): should clarify the meaning behind keyword usage
#define cy_global static
#define cy_internal static
#define cy_persist static

#ifndef CY_STATIC_ASSERT
    // NOTE(cya): because C macro expansion bizarreness
    #define CY_STATIC_ASSERT_INTERNAL_EX(cond, line) \
        typedef char STATIC_ASSERTION_FAILED_AT_LINE_##line[!!(cond) * 2 - 1]
    #define CY_STATIC_ASSERT_INTERNAL(cond, line) \
        CY_STATIC_ASSERT_INTERNAL_EX(cond, line)
    #define CY_STATIC_ASSERT(cond) CY_STATIC_ASSERT_INTERNAL(cond, __LINE__)
#endif

CY_STATIC_ASSERT(true); // sanity check

#ifndef CY_ASSERT_MSG
    #define CY_ASSERT_MSG(cond, msg, ...) { \
        if (!(cond)) { \
            cy_handle_assertion( \
                "Assertion failed",#cond, \
                __FILE__, (i64)__LINE__, msg, ##__VA_ARGS__ \
            ); \
            CY_DEBUG_TRAP(); \
        } \
    } (void)0
#endif

#ifndef CY_ASSERT
#define CY_ASSERT(cond) CY_ASSERT_MSG(cond, NULL)
#endif

#ifndef CY_DEBUG_TRAP
    #if defined(_MSC_VER)
        #if _MSC_VER < 1300
            #define CY_DEBUG_TRAP() __asm int 3
        #else
            #define CY_DEBUG_TRAP() __debugbreak()
        #endif
    #else
        #define CY_DEBUG_TRAP() __builtin_trap()
    #endif
#endif

#include <stdio.h>   // for optional logging
#include <stdarg.h>  // va_args

// TODO(cya): implement printf family and move this stuff to some better place
#define cy_printf_err(...) fprintf(stderr, __VA_ARGS__)
#define cy_printf_err_va(fmt, va) vfprintf(stderr, fmt, va)

static void cy_handle_assertion(
    const char *prefix,
    const char *cond,
    const char *file,
    i32 line,
    const char *msg,
    ...
) {
    cy_printf_err("%s(%d): %s: ", file, line, prefix);
    if (cond != NULL) {
        cy_printf_err( "`%s` ", cond);
    }
    if (msg != NULL) {
        va_list va;
        va_start(va, msg);
        cy_printf_err_va(msg, va);
        va_end(va);
    }

    cy_printf_err("\n");
}

// TODO(cya): remove header and implement mem functions
#include <string.h>

static void *cy_mem_copy(
    void *restrict dst,
    const void *restrict src,
    usize bytes
) {
    return memcpy(dst, src, bytes);
}

static void *cy_mem_move(void *dst, const void *src, usize bytes)
{
    return memmove(dst, src, bytes);
}

static void *cy_mem_set(void *dst, u8 val, usize bytes)
{
    return memset(dst, val, bytes);
}

static void *cy_mem_zero(void *dst, usize bytes)
{
    return cy_mem_set(dst, 0, bytes);
}

#define CY_BIT(n) (1 << (n))

/* ------------------------------- Allocators ------------------------------- */

typedef enum {
    CY_ALLOCATION_ALLOC,
    CY_ALLOCATION_FREE,
    CY_ALLOCATION_FREE_ALL,
    CY_ALLOCATION_RESIZE,
} CyAllocationType;

typedef enum {
    CY_ALLOCATOR_CLEAR_TO_ZERO = CY_BIT(0),
} CyAllocatorFlags;

#define CY_ALLOCATOR_PROC(name)            \
    void *name(                            \
        void *data, CyAllocationType type, \
        isize size, isize align,           \
        void *old_mem, isize old_size,     \
        u64 flags                          \
    )

typedef CY_ALLOCATOR_PROC(CyAllocatorProc);

typedef struct {
    CyAllocatorProc *proc;
    void *data;
} CyAllocator;

#define CY_DEFAULT_ALIGNMENT (2 * sizeof(void*))
#define CY_DEFAULT_ALLOCATOR_FLAGS (CY_ALLOCATOR_CLEAR_TO_ZERO)

static void *cy_alloc_align(CyAllocator a, isize size, isize align)
{
    return a.proc(
        a.data, CY_ALLOCATION_ALLOC,
        size, align,
        NULL, 0,
        CY_DEFAULT_ALLOCATOR_FLAGS
    );    
}

static void *cy_alloc(CyAllocator a, isize size)
{
    return cy_alloc_align(a, size, CY_DEFAULT_ALIGNMENT);
}

static void cy_free(CyAllocator a, void *ptr)
{
    if (ptr != NULL) {
        a.proc(
            a.data, CY_ALLOCATION_FREE,
            0, 0,
            ptr, 0,
            CY_DEFAULT_ALLOCATOR_FLAGS
        );    
    }
}

static void cy_free_all(CyAllocator a)
{
    a.proc(
        a.data, CY_ALLOCATION_FREE_ALL,
        0, 0,
        NULL, 0,
        CY_DEFAULT_ALLOCATOR_FLAGS
    );    
}

static void *cy_resize_align(CyAllocator a, void *ptr, isize old_size, isize new_size, isize align)
{
    return a.proc(
        a.data, CY_ALLOCATION_RESIZE,
        new_size, align,
        ptr, old_size,
        CY_DEFAULT_ALLOCATOR_FLAGS
    );    
}

static void *cy_resize(CyAllocator a, void *ptr, isize old_size, isize new_size)
{
    return cy_resize_align(a, ptr, old_size, new_size, CY_DEFAULT_ALIGNMENT);
}

static void *cy_alloc_copy_align(CyAllocator a, const void *src, isize size, isize align)
{
    return cy_mem_copy(cy_alloc_align(a, size, align), src, size);
}

static void *cy_alloc_copy(CyAllocator a, const void *src, isize size)
{
    return cy_alloc_copy_align(a, src, size, CY_DEFAULT_ALIGNMENT);
}

static char *cy_alloc_string_len(CyAllocator a, const char *str, isize len)
{
    char *res = cy_alloc_copy(a, str, len + 1);
    res[len] = '\0';
    return res; 
}

#define U8_MAX 0xFFU
#define CY__ONES ((usize)-1 / U8_MAX)
#define CY__HIGHS (CY__ONES * (U8_MAX / 2 + 1))
#define CY__HAS_ZERO(word) ((word) - CY__ONES & ~(word) & CY__HIGHS)

static inline isize cy_string_len(const char *str)
{
    if (str == NULL) {
        return 0;
    }
    
    const char *begin = str;
    while ((uintptr)str % sizeof(isize) != 0) {
        if (*str == '\0') {
            return str - begin;
        }

        str += 1;
    }

    const isize *w = (const isize*)str;
    while (!CY__HAS_ZERO(*w)) w += 1;

    str = (const char*)w;
    while (*str != '\0') str += 1;

    return str - begin;
}

static char *cy_alloc_string(CyAllocator a, const char *str)
{
    return cy_alloc_string_len(a, str, cy_string_len(str));
}

// TODO(cya): all the rest
// NOTE(cya): for single items and arrays
#define cy_alloc_item(allocator, Type) (Type*)cy_alloc(allocator, sizeof(Type))
#define cy_alloc_array(allocator, Type, count) \
    (Type*)cy_alloc(allocator, sizeof(Type) * (count))

// NOTE(cya): the default malloc-style heap allocator
static CyAllocator cy_heap_allocator(void);
static CyAllocatorProc cy_heap_allocator_proc;

#define cy_heap_alloc(size) cy_alloc(cy_heap_allocator(), size)
#define cy_heap_free(ptr) cy_free(cy_heap_allocator(), ptr)

static inline b8 cy_is_power_of_two(isize n)
{
    return (n > 0) && (n & (n - 1)) == 0;
}

static inline isize cy_align_forward(isize size, isize align)
{
    CY_ASSERT(cy_is_power_of_two(align));

    isize mod = size & (align - 1);
    return mod ? size + align - mod : size;
}

static inline void *cy_align_ptr_forward(void *ptr, isize align)
{
    return (void*)cy_align_forward((isize)ptr, align);
}

/* Aligns a pointer forward accounting for both the size
 * of a header and the alignment */
static inline usize cy_calc_header_padding(
    uintptr ptr,
    usize align,
    usize header_size
) {
    CY_ASSERT(cy_is_power_of_two(align));

    uintptr a = (uintptr)align;
    uintptr mod = ptr & (a - 1);
    uintptr padding = mod ? a - mod : 0;
    if (padding < (uintptr)header_size) {
        uintptr needed_space = header_size - padding;
        padding += (needed_space & (a - 1)) ?
            a * (needed_space / a + 1) : a * (needed_space / a);
    }

    return (usize)padding;
}

/* ---------- Page Allocator Section ---------- */
#if defined(CY_OS_WINDOWS)
    #include <windows.h>
    #define CY_PAGE_SIZE (4 * 1024)
#else
    #include <unistd.h>
    #include <sys/mman.h>
    #if defined(CY_OS_MACOSX)
        #define CY_PAGE_SIZE (16 * 1024)
    #else
        #define CY_PAGE_SIZE (4 * 1024)
    #endif
#endif

typedef struct PageChunk {
    usize size;  // Total size of allocation (including aligned meta-chunk)
    usize align; // Alignment (for tracking down the start of the allocation)
} PageChunk;

/* Returns the total size of the page(s) reserved by the OS (including chunk) */
static inline usize cy_page_aligned_size(void *ptr)
{
    PageChunk *chunk = (PageChunk*)((char*)ptr - sizeof(*chunk));
    return chunk->size;
}

static inline void *page_alloc_align(usize size, usize align)
{
    CY_ASSERT(size > 0);

    isize chunk_aligned_size =
        cy_align_forward(sizeof(PageChunk), align);
    isize aligned_size =
        cy_align_forward(size + chunk_aligned_size,
            align > CY_PAGE_SIZE ? align : CY_PAGE_SIZE);
    PageChunk chunk = { .size = aligned_size, .align = align };

    void *mem = NULL;
#if defined(CY_OS_WINDOWS)
    mem = VirtualAlloc(NULL, aligned_size,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    mem = mmap(NULL, aligned_size, PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANON, -1, 0);
#endif
    if (mem == NULL) return NULL;

    CY_ASSERT(mem == cy_align_ptr_forward(mem, align));

    /* Append size of allocation to the area just before the first byte
     * of aligned memory which will be handed to the caller at return */
    cy_mem_copy((u8*)mem + chunk_aligned_size - sizeof(chunk),
        &chunk, sizeof(chunk));

    return (void*)((u8*)mem + chunk_aligned_size);
}

/* Returns an allocated block of memory containing [size] bytes rounded
 * up to a multiple of the system's page size (with default alignment) */
static inline void *page_alloc(usize size)
{
    return page_alloc_align(size, CY_DEFAULT_ALIGNMENT);
}

/* Immediately returns the allocated memory starting @ [ptr] back to the OS */
static inline void page_free(void *ptr)
{
    /* All allocated pages will have a metadata chunk right before the
     * beginning of the pointer that must be stepped back into
     * in order to free the whole block -> [*...[PageChunk][ptr]...] */
    PageChunk* chunk = (PageChunk*)((char*)ptr - sizeof(*chunk));

#if defined(CY_OS_WINDOWS)
    VirtualFree((void*)chunk, 0, MEM_RELEASE);
#else
    const isize chunk_aligned_size =
        cy_align_forward(sizeof(*chunk), chunk->align);
    munmap((char*)ptr - chunk_aligned_size, chunk->size);
#endif
}

static inline void *page_realloc_align(
    void *ptr,
    usize new_size,
    usize align
) {

    PageChunk *chunk = (PageChunk*)((char*)ptr - sizeof(*chunk));

    const isize current_size = chunk->size;
    const isize new_aligned_size = cy_align_forward(new_size, align);
    const isize chunk_aligned_size =
        cy_align_forward(sizeof(*chunk), chunk->align);
    const isize new_page_aligned_size = cy_align_forward(
        new_aligned_size + chunk_aligned_size, CY_PAGE_SIZE);

    if (new_page_aligned_size <= current_size) {
        isize isizeo_free = current_size - new_size - chunk_aligned_size;

#if defined(CY_OS_WINDOWS)
        VirtualFree((char*)ptr + new_size, isizeo_free, MEM_DECOMMIT);
#else
        munmap((char*)ptr + new_size, isizeo_free);
#endif

        chunk->size = new_page_aligned_size;
        return ptr;
    }

    void *new_ptr = page_alloc(new_aligned_size);
    if (new_ptr == NULL) return NULL;

    cy_mem_copy(new_ptr, ptr, current_size - chunk_aligned_size);
    page_free(ptr);

    return new_ptr;
}

/* Returns a pointer to a block of memory containing the same data as [ptr]
 * (or up to where the new size allows, in case it's smaller the current size)
 * If a reallocation is needed and FAILS, page_realloc() returns NULL and
 * DOESN'T free the block pointed to by [ptr] */
static inline void *page_realloc(void *ptr, usize new_size)
{
    return page_realloc_align(ptr, new_size, CY_DEFAULT_ALIGNMENT);
}

/* Returns the allocation size that was given to page_alloc/page_realloc */
static inline usize page_get_size(void *ptr)
{
    PageChunk *chunk = (PageChunk*)((char*)ptr - sizeof(*chunk));
    return chunk->size - cy_align_forward(sizeof(*chunk), chunk->align);
}

/* ---------- Arena Allocator Section ---------- */
typedef struct ArenaNode {
    unsigned char *buf;     // actual arena memory
    usize size;            // size of the buffer in bytes
    usize offset;          // offset to first byte of (unaligned) free memory
    usize prev_offset;     // offset to first byte of previous allocation
    struct ArenaNode *next; // next node (duh)
} ArenaNode;

typedef struct ArenaState {
    ArenaNode *first_node;
#if 0 // probably won't even use this
    usize end_index;
#endif
} ArenaState;

typedef struct Arena {
    AllocFunc alloc;     // backing allocator
    FreeFunc free; // backing deallocator
    ArenaState *state;
} Arena;

/* TODO:
 *  1. create an internal stripped-off linked-list implementation that inserts
 *     new nodes at the beginning of the list for better performance
 */

/* Default initial size set to one page */
#define CY_ARENA_INIT_SIZE     CY_PAGE_SIZE
#define CY_ARENA_GROWTH_FACTOR 2.0
#define CY_ARENA_STRUCTS_SIZE \
    (sizeof(Arena) + sizeof(ArenaState) + sizeof(ArenaNode))

/* Returns an initialized Arena struct with a capacity of initial_size
 * takes in a backing allocator and deallocator complying to the standard
 * malloc() interface (uses malloc() and free() in case they're null) */
static inline Arena *arena_init(
    usize initial_size,
    Allocator *backing
) {
    usize default_size = CY_ARENA_INIT_SIZE;
    b8 custom = backing == NULL ||
        (backing->alloc == NULL && backing->dealloc == NULL);
    if (custom) {
        backing->alloc = page_alloc;
        backing->dealloc = page_free;
        default_size -= sizeof(PageChunk) + CY_ARENA_STRUCTS_SIZE;
    } else if (backing->alloc == NULL || backing->dealloc == NULL) {
        return NULL;
    }

    if (initial_size == 0) initial_size = default_size;

    usize size = CY_ARENA_STRUCTS_SIZE + initial_size;
    Arena *arena = backing->alloc(size);
    if (arena == NULL) return NULL;

    arena->alloc = backing->alloc;
    arena->free = backing->dealloc;
    arena->state = (ArenaState*)(arena + 1);

    arena->state->first_node = (ArenaNode*)(arena->state + 1);

    ArenaNode *first_node = arena->state->first_node;
    first_node->buf = (unsigned char*)(first_node + 1);
    first_node->size = initial_size;

    return arena;
}

static inline ArenaNode *cy_arena_create_node(Arena *arena, usize size)
{
    ArenaNode *cur_node = arena->state->first_node;
    while (cur_node != NULL) cur_node = cur_node->next;

    usize cur_node_size = sizeof(ArenaNode) + size;
    cur_node = arena->alloc(cur_node_size);
    if (cur_node == NULL) return NULL;

    cur_node->buf = (unsigned char*)(arena->state->first_node + 1);
    cur_node->size = size;

    return cur_node;
}

static inline void *arena_alloc_align(
    Arena *arena,
    usize bytes,
    usize align
) {
    ArenaNode *cur_node = arena->state->first_node;
    uintptr buf = (uintptr)cur_node->buf, curr_ptr = buf + cur_node->offset;
    uintptr offset = cy_align_forward(curr_ptr, align) - buf;
    while (offset + bytes > cur_node->size) {
        /* need more memory! (add new node to linked list) */
        if (cur_node->next == NULL) {
            usize new_size = (usize)(cur_node->size * CY_ARENA_GROWTH_FACTOR);
            if (bytes + sizeof(ArenaNode) > new_size + sizeof(ArenaNode)) {
                new_size = cy_align_forward(bytes, CY_PAGE_SIZE);
            }

            cur_node->next = cy_arena_create_node(arena, new_size);
        }

        cur_node = cur_node->next;

        buf = (uintptr)cur_node->buf;
        curr_ptr = buf + cur_node->offset;
        offset = cy_align_forward(curr_ptr, align) - buf;
    }

    cur_node->prev_offset = offset;
    cur_node->offset = offset + bytes;

    return (void*)(cur_node->buf + offset);
}

/* Returns a suitable place in memory for a buffer of the given size in bytes,
 * from somewhere within the current context arena */
static inline void *arena_alloc(Arena *arena, usize bytes)
{
    return arena_alloc_align(arena, bytes, CY_DEFAULT_ALIGNMENT);
}

/* Frees all memory allocated for the current arena including itself
 * (and any other memory necessary for internal keeping) */
static inline void arena_deinit(Arena *arena)
{
    ArenaNode *cur_node = arena->state->first_node, *next = cur_node->next;
    while (next != NULL) {
        cur_node = next;
        next = next->next;
        arena->free(cur_node);
    }

    arena->free(arena);
}

/* Resizes the last allocation done in the arena (if [old_memory] doesn't match
 * the return value of the last allocation done in [arena], this function
 * returns NULL) */
static inline void *arena_realloc_align(
    Arena *arena,
    void *old_memory,
    usize old_size,
    usize new_size,
    usize align
) {
    CY_ASSERT(cy_is_power_of_two(align));

    unsigned char *old_mem = old_memory;
    if (old_mem == NULL || old_size == 0) {
        return arena_alloc_align(arena, new_size, align);
    }

    ArenaNode *cur_node = arena->state->first_node;
    while (cur_node->next != NULL) cur_node = cur_node->next;

    if (cur_node->buf + cur_node->prev_offset != old_mem) return NULL;

    usize aligned_size = cy_align_forward(new_size, align);
    uintptr aligned_offset = cy_align_forward(cur_node->offset, align);
    unsigned char *new_mem = old_mem + aligned_offset;
    if (new_mem + aligned_size < cur_node->buf + cur_node->size) {
        cur_node->offset = aligned_offset + aligned_size;
        if (cur_node->offset < cur_node->prev_offset + old_size) {
            cy_mem_set(cur_node->buf + cur_node->offset, 0,
                old_size - aligned_size);
        }

        return (void*)new_mem;
    }

    void *new_memory = arena_alloc_align(arena, new_size, align);
    if (new_memory == NULL) return NULL;

    usize copy_size = old_size < new_size ? old_size : new_size;
    cy_mem_move(new_memory, old_memory, copy_size);
    return new_memory;
}

static inline void *arena_realloc(
    Arena *arena,
    void *old_memory,
    usize old_size,
    usize new_size
) {
    return arena_realloc_align(arena, old_memory,
        old_size, new_size, CY_DEFAULT_ALIGNMENT);
}

/* Frees all the excess allocations done by the current context arena and
 * clears all of its space to zero */
static inline void arena_flush(Arena *arena) {
    ArenaNode *cur_node = arena->state->first_node, *next = cur_node->next;
    while (next != NULL) {
        cur_node = next;
        next = next->next;
        arena->free(cur_node);
    }

    usize node_size = sizeof(ArenaNode) + arena->state->first_node->size;
    cy_mem_set(arena->state->first_node, 0, node_size);
}

/* Returns allocated space in the current context arena containing a copy of
 * (len) bytes of the given string (null terminator is appended) */
static inline char *arena_alloc_string(Arena *a, const char *str, usize len) {
    char *s = arena_alloc(a, (len + 1) * sizeof(char));
    s[len] = '\0';
    return cy_mem_copy(s, str, len);
}

/* Returns allocated space in the current context arena containing a copy of
 * the provided C-string */
static inline char *arena_alloc_c_string(Arena *a, const char *str) {
    usize bytes = 0;
    while (*(str + bytes++));

    char *buf = (char*)arena_alloc(a, bytes);
    usize idx = 0;
    while((*(buf + idx) = *(str + idx))) idx++;

    return buf;
}

/* Returns a pointer to the first character of an allocated string inside
 * [arena] with a format defined by [fmt] */
static inline char *arena_sprintf(Arena *arena, const char *fmt, ...) {
    va_list args, args2;
    va_start(args, fmt);
    va_copy(args2, args);

    usize bytes = vsnprintf(NULL, 0 , fmt, args) + 1;
    va_end(args);

    char *s = arena_alloc_align(arena, bytes, CY_DEFAULT_ALIGNMENT);
    if (s == NULL) return NULL;

    vsprintf(s, fmt, args2);
    va_end(args2);

    return s;
}

/* ---------- Stack Allocator Section ---------- */
typedef struct StackNode {
    unsigned char *buf;
    usize size;
    usize prev_offset;
    usize offset;
    struct StackNode *next;
} StackNode;

typedef struct StackState {
    StackNode *first_node;
} StackState;

typedef struct Stack {
    void *(*alloc)(usize);
    void (*free)(void*);
    StackState *state;
} Stack;

typedef struct StackHeader {
    usize prev_offset;
    usize padding;
} StackHeader;

/* Default initial size set to one page */
#define CY_STACK_INIT_SIZE     CY_PAGE_SIZE
#define CY_STACK_GROWTH_FACTOR 2.0
#define CY_STACK_STRUCTS_SIZE \
    (sizeof(Stack) + sizeof(StackState) + sizeof(StackNode))

static inline Stack *stack_init(
    usize initial_size,
    void *(*backing_allocator)(usize),
    void (*backing_deallocator)(void*)
) {
    if (initial_size == 0) initial_size = CY_STACK_INIT_SIZE;

    if (backing_allocator == NULL && backing_deallocator == NULL) {
        backing_allocator = page_alloc;
        backing_deallocator = page_free;
        initial_size = cy_align_forward(initial_size, CY_PAGE_SIZE);
    } else if (backing_allocator == NULL || backing_deallocator == NULL) {
        return NULL;
    }

    usize size = CY_STACK_STRUCTS_SIZE + initial_size;
    Stack *stack = backing_allocator(size);
    if (stack == NULL) return NULL;

    stack->alloc = backing_allocator;
    stack->free = backing_deallocator;
    stack->state = (StackState*)(stack + 1);

    stack->state->first_node = (StackNode*)(stack->state + 1);

    StackNode *first_node = stack->state->first_node;
    first_node->buf = (unsigned char*)(first_node + 1);
    first_node->size = initial_size;

    return stack;
}

static inline StackNode *cy_stack_insert_node(Stack *stack, usize size)
{
    if (stack == NULL) return NULL;

    StackNode *cur_node = stack->state->first_node;
    usize new_node_size = sizeof(*cur_node) + size;
    StackNode *new_node = stack->alloc(new_node_size);
    if (new_node == NULL) return NULL;

    new_node->buf = (unsigned char*)(new_node + 1);
    new_node->size = size;
    new_node->next = stack->state->first_node;
    stack->state->first_node = new_node;

    return new_node;
}

void *stack_alloc_align(Stack *stack, usize size, usize align)
{
    if (stack == NULL) return NULL;

    StackHeader *header;
    StackNode *cur_node = stack->state->first_node;
    uintptr cur_address = (uintptr)(cur_node->buf + cur_node->offset);
    usize padding = cy_calc_header_padding(cur_address,
        align, sizeof(*header));
    usize alloc_size = padding + size;
    if (cur_node->offset + padding + size > cur_node->size) {
        /* out of memory (TODO: implement finished linked-list stack logic) */
        usize node_size = cur_node->size * CY_STACK_GROWTH_FACTOR;
        if (node_size < alloc_size)
            node_size = cy_align_forward(alloc_size, CY_PAGE_SIZE);

        cur_node = cy_stack_insert_node(stack, node_size);
        if (cur_node == NULL) return NULL;

        cur_address = (uintptr)(cur_node->buf + cur_node->offset);
        padding = cy_calc_header_padding(cur_address, align, sizeof(*header));
    }

    uintptr next_address = cur_address + (uintptr)padding;
    header = (StackHeader*)(next_address - sizeof(*header));
    header->padding = (u8)padding;
    header->prev_offset = cur_node->offset;

    cur_node->prev_offset = header->prev_offset;
    cur_node->offset += alloc_size;

    return (void*)next_address;
}

void *stack_alloc(Stack *stack, usize size)
{
    return stack_alloc_align(stack, size, CY_DEFAULT_ALIGNMENT);
}

void stack_free(Stack *stack, void *ptr)
{
    if (ptr == NULL) return;

    StackNode *cur_node = stack->state->first_node;
    uintptr start = (uintptr)cur_node->buf;
    uintptr end = start + (uintptr)cur_node->size;
    uintptr cur_address = (uintptr)ptr;
    if (!(start <= cur_address && cur_address < end)) {
        CY_ASSERT(0 && "out-of-bounds pointer deallocation (stack_free())");
        return;
    }
    if (cur_address >= start + (uintptr)cur_node->offset) {
        /* allow double-frees */
        return;
    }

    StackHeader *header = (StackHeader*)(cur_address - sizeof(*header));
    usize prev_offset = (usize)(cur_address - header->padding - start);
    if (prev_offset != header->prev_offset) {
        CY_ASSERT(0 && "out-of-order stack deallocation (stack_free())");
        return;
    }
    cur_node->offset = prev_offset;
    cur_node->prev_offset = header->prev_offset;
}

void *stack_realloc_align(
    Stack *stack,
    void *ptr,
    usize old_size,
    usize new_size,
    usize align
) {
    if (ptr == NULL) {
        return stack_alloc_align(stack, new_size, CY_DEFAULT_ALIGNMENT);
    } else if (new_size == 0) {
        stack_free(stack, ptr);
        return NULL;
    }

    StackNode *cur_node = stack->state->first_node;
    uintptr start = (uintptr)cur_node->buf;
    uintptr end = start + cur_node->size;
    uintptr cur_address = (uintptr)ptr;
    if (!(start <= cur_address && cur_address < end)) {
        CY_ASSERT(0 && "out-of-bounds stack reallocation (stack_realloc())");
        return NULL;
    }
    if (cur_address >= start + (uintptr)cur_node->offset) {
        CY_ASSERT(0 && "out-of-order stack reallocation (stack_realloc())");
        return NULL;
    }

    StackHeader *header = (StackHeader*)(cur_address - sizeof(*header));
    uintptr cur_padding = header->padding;
    uintptr alloc_start = (uintptr)header - cur_padding;
    uintptr new_padding =
        cy_calc_header_padding(alloc_start, align, sizeof(*header));
    if (new_size <= old_size && new_padding == cur_padding) {
        cur_node->offset -= old_size - new_size;
        return ptr;
    }

    usize prev_offset = (usize)(cur_address - header->padding - start);
    usize new_offset = (usize)(alloc_start + new_padding + new_size - start);
    if (new_offset <= end) {
        uintptr new_address = alloc_start + new_padding;
        usize min_size = old_size < new_size ? old_size : new_size;
        cy_mem_move((void*)new_address, ptr, min_size);

        header = (StackHeader*)(new_address - sizeof(*header));
        header->padding = new_padding;
        header->prev_offset = prev_offset;
        cur_node->offset = new_offset;

        return (void*)new_address;
    }

    void *new_ptr = stack_alloc_align(stack, new_size, align);
    cy_mem_move(new_ptr, ptr, old_size < new_size ? old_size : new_size);

    cur_node->offset = prev_offset;
    cur_node->prev_offset = header->prev_offset;

    return new_ptr;
}

void *stack_realloc(Stack *stack, void *ptr, usize old_size, usize new_size)
{
    return stack_realloc_align(stack, ptr,
        old_size, new_size, CY_DEFAULT_ALIGNMENT);
}

void stack_deinit(Stack *stack)
{
    StackNode *cur_node = stack->state->first_node, *next = cur_node->next;
    while (next != NULL) {
        stack->free(cur_node);
        cur_node = next;
        next = next->next;
    }

    stack->free(stack);
}

/* -------------------------------- Strings --------------------------------- */
typedef struct {
    u8 *data;
    isize len;
} String;

typedef struct {
    u8 *data;
    isize len;
    isize cap;
    Allocator *alloc;
} StringBuilder;

#endif /* _CY_H */
