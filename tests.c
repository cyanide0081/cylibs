#define CY_IMPLEMENTATION
#include "cy.h"

#include "testutils.h"

static i32 exit_code;

static void test_page_allocator(void)
{
    cy_printf("%sTesting Page Allocator...%s\n", VT_BOLD, VT_RESET);

    CyFile f = {0};
    CyFileError err = cy_file_open(&f, "sample.txt");
    TEST_ASSERT(err == 0, "unable to open file: %s", cy_file_error_as_str(err));

    isize txt_len = cy_file_size(&f);

    CyAllocator a = cy_page_allocator();

    isize txt_size = txt_len + 1;
    char *txt_buf = cy_alloc(a, txt_size);
    print_s("allocated page");

    TEST_ASSERT(
        (uintptr)txt_buf % CY_DEFAULT_ALIGNMENT == 0,
        "returned memory is not properly aligned"
    );
    print_s("validated memory alignment");

    cy_file_read(&f, txt_buf, txt_len);
    cy_file_close(&f);
    print_s("allocated message (%.2lfKB) (page size: %.2lfKB)",
        txt_len / KB, cy_page_allocator_alloc_size(txt_buf) / KB);

    {
        void *new_buf = cy_resize(a, txt_buf, txt_size, 0x80);
        TEST_ASSERT_NOT_NULL(
            new_buf, "unable to shrink page size: %s", cy_file_error_as_str(err)
        );

        txt_buf = new_buf;
        txt_size = 0x80;
        print_s("shrunk page size (%.2lfKB)", txt_size / KB);
    }
    {
        void *new_buf = cy_resize(a, txt_buf, txt_size, 0x100000);
        TEST_ASSERT_NOT_NULL(
            new_buf, "unable to expand page size: %s", cy_file_error_as_str(err)
        );

        txt_buf = new_buf;
        txt_size = 0x100000;
        print_s("expanded page size (%.2lfkB)",
            cy_page_allocator_alloc_size(txt_buf) / KB);
    }
    {
        u8 val = 69;
        isize page_size = cy_page_allocator_alloc_size(txt_buf);
        cy_mem_set(txt_buf, val, page_size);
        print_s("wrote [%u] to page (%.2lfKB)", val, page_size / KB);
    }

    cy_free(a, txt_buf);
    print_s("deallocated page");
}

static void test_arena_allocator(void)
{
    cy_printf("%sTesting Arena Allocator...%s\n", VT_BOLD, VT_RESET);

    CyFile f = {0};
    CyFileError err = cy_file_open(&f, "sample.txt");
    TEST_ASSERT(err == 0, "unable to open file: %s", cy_file_error_as_str(err));

    isize txt_len = cy_file_size(&f);

    CyArena arena = cy_arena_init(cy_heap_allocator(), 0x4000);
    CyAllocator a = cy_arena_allocator(&arena);
    print_s("initialized arena");

    isize txt_size = txt_len + 1;
    char *txt_buf = cy_alloc_align(a, txt_size, 1);
    cy_file_read(&f, txt_buf, txt_len);
    cy_file_close(&f);
    print_s(
        "allocated buffer storing file contents (%.2lfKB)",
        (txt_len + 1) / KB
    );

    isize expanded_size = txt_size * 2;
    {
        txt_buf = cy_resize(a, txt_buf, txt_size, expanded_size);
        TEST_ASSERT_NOT_NULL(txt_buf, "unable to expand buffer in arena");

        print_s("expanded message buffer (%.2lfKB)", expanded_size / KB);
    }
    {
        isize shrunk_size = expanded_size / 4;
        txt_buf = cy_resize(a, txt_buf, expanded_size, shrunk_size);
        TEST_ASSERT_NOT_NULL(txt_buf, "unable to shrink buffer in arena");

        print_s("shrunk message buffer (%.2lfKB)", shrunk_size / KB);
    }
    {
        cy_free_all(a);
        TEST_ASSERT(
            arena.state.first_node->offset == 0, "unexpected arena offset"
        );
        f64 size = arena.state.first_node->size / KB;
        print_s("freed whole arena (Available size: %.2lfKB)", size);
    }
    {
        CyArenaNode *first_node = arena.state.first_node;
        while (first_node->offset < first_node->size)
            (void)cy_alloc_align(a, 0x1, 1);

        print_s("exhausted arena (ofs: %zu, size: %zu)",
            first_node->offset, first_node->size);
    }
    {
        u8 val = 69;
        CyArenaNode *first_node = arena.state.first_node;
        cy_mem_set(first_node->buf, val, first_node->size);
        print_s("wrote [%u] to arena (%.2lfKB)", val, first_node->size / KB);
    }
    {
        const char *str = "basolutely.";
        char *str_buf = cy_alloc_string(a, str);
        print_s(
            "allocated string into arena (str_buf: '%.*s')",
            (i32)cy_str_len(str_buf), str_buf
        );
    }
    {
        cy_free_all(a);
        TEST_ASSERT(
            arena.state.first_node->offset == 0, "unexpected arena offset"
        );
        f64 size = arena.state.first_node->size / KB;
        print_s("freed whole arena (Available size: %.2lfKB)", size);
    }

    cy_arena_deinit(&arena);
    print_s("deinitialized arena");
}

static void test_stack_allocator(void)
{
    cy_printf("%sTesting Stack Allocator...%s\n", VT_BOLD, VT_RESET);

    CyFile f = {0};
    CyFileError err = cy_file_open(&f, "sample.txt");
    TEST_ASSERT(err == 0, "unable to open file: %s", cy_file_error_as_str(err));

    isize txt_len = cy_file_size(&f);

    CyStack stack = cy_stack_init(cy_heap_allocator(), 0x4000);
    CyAllocator a = cy_stack_allocator(&stack);
    print_s("initialized stack");

    isize txt_size = txt_len + 1;
    char *txt_buf = cy_alloc_align(a, txt_size, 1);
    cy_file_read(&f, txt_buf, txt_len);
    cy_file_close(&f);
    print_s(
        "allocated buffer storing file contents (%.2lfKB)",
        (txt_len + 1) / KB
    );

    isize expanded_size = txt_size * 2;
    {
        txt_buf = cy_resize(a, txt_buf, txt_size, expanded_size);
        TEST_ASSERT_NOT_NULL(txt_buf, "unable to expand buffer in stack");

        print_s("expanded message buffer (%.2lfKB)", expanded_size / KB);
    }
    {
        isize shrunk_size = expanded_size / 4;
        txt_buf = cy_resize(a, txt_buf, expanded_size, shrunk_size);
        TEST_ASSERT_NOT_NULL(txt_buf, "unable to shrink buffer in stack");

        print_s("shrunk message buffer (%.2lfKB)", shrunk_size / KB);
    }
    {
        cy_free_all(a);
        TEST_ASSERT(
            stack.state.first_node->offset == 0, "unexpected stack offset"
        );
        f64 size = stack.state.first_node->size / KB;
        print_s("freed whole stack (Available size: %.2lfKB)", size);
    }
    {
        const char *strs[] = {
            "basolutely.",
            "ok.",
            "hmm.",
        };
        char *buf = NULL;
        for (isize i = 0; i < CY_ARRAY_LEN(strs); i++) {
            buf = cy_alloc_string(a, strs[i]);
            print_s("allocated string into stack: '%s'", buf);
        }

        cy_free(a, buf);
        print_s("freed string from stack (LIFO check)");
    }

    cy_stack_deinit(&stack);
    print_s("deinitialized stack");
}

static void test_pool_allocator(void)
{
    cy_printf("%sTesting Pool Allocator...%s\n", VT_BOLD, VT_RESET);


    CyPool pool = cy_pool_init(cy_heap_allocator(), 8, cy_sizeof(f64));
    CyAllocator a = cy_pool_allocator(&pool);
    print_s("initialized pool");

    f64 *f = cy_alloc_item(a, f64);
    TEST_ASSERT_NOT_NULL(f, "unable to allocate pool chunk");

    print_s("allocated pool chunk");
    while (pool.free_list_head != NULL) {
        f = cy_alloc_item(a, f64);
    }
    
    print_s("exhausted pool");

    cy_free_all(a);
    print_s("freed all chunks in pool");
    
    cy_pool_deinit(&pool);
    print_s("deinitialized pool");
}

static void test_cy_strings(void)
{
    cy_printf("%sTesting CyStrings...%s\n", VT_BOLD, VT_RESET);

    CyArena arena = cy_arena_init(cy_heap_allocator(), 0x1000);
    CyAllocator a = cy_arena_allocator(&arena);

    isize len = 13;
    CyString str = cy_string_create(a, "Hello, World!");
    TEST_ASSERT_NOT_NULL(str, "string creation failed");
    TEST_ASSERT(cy_string_len(str) == len, "unexpected string length");
    TEST_ASSERT(cy_string_cap(str) == len, "unexpected string capacity");

    print_s("created new string '%s'", str);

    const char *suffix = "我爱你";
    str = cy_string_append_fmt(str, " %s", suffix);
    TEST_ASSERT_NOT_NULL(str, "unable to append string");
    TEST_ASSERT(
        cy_string_len(str) == len + 1 + cy_str_len(suffix),
        "unexpected string length difference"
    );

    print_s("appended to string, result: '%s'", str);

    const char *prefix = "OwO";
    str = cy_string_prepend_c(str, " ");
    TEST_ASSERT_NOT_NULL(str, "unable to prepend string");
    str = cy_string_prepend_c(str, prefix);
    TEST_ASSERT_NOT_NULL(str, "unable to prepend string");
    TEST_ASSERT(
        cy_string_len(str) == cy_str_len(prefix) + len + cy_str_len(suffix) + 2,
        "unexpected string length difference"
    );

    print_s("prepended to string, result: '%s'", str);

    CyString dup = cy_string_dup(a, str);
    TEST_ASSERT_NOT_NULL(dup, "string duplication failed");

    print_s("duplicated string");

    TEST_ASSERT(
        cy_string_are_equal(str, dup),
        "duplicated string is not equal to source"
    );

    print_s("validated contents of duplicated string");

    isize old_len = cy_string_len(str);
    str = cy_string_append_c(str, " \t  ");
    TEST_ASSERT_NOT_NULL(str, "unable to append to string");

    str = cy_string_append_c(str, " \t\t   ");
    str = cy_string_trim(str, " \t");
    TEST_ASSERT_NOT_NULL(str, "unable to trim string");
    TEST_ASSERT(
        cy_string_len(str) == old_len, "incorrectly trimmed string: '%s'", str
    );

    print_s("appended whitespace to string, result: '%s'", str);

    str = cy_string_trim_trailing_whitespace(str);
    TEST_ASSERT_NOT_NULL(str, "unable to trim string");
    TEST_ASSERT(
        cy_string_len(str) == old_len,
        "incorrectly trimmed string: '%s'", str
    );

    print_s("trimmed trailing whitespace from string, result: '%s'", str);

    cy_free_all(a);
    print_s("freed all strings");
}

int main(void)
{
    test_page_allocator();
    test_arena_allocator();
    test_stack_allocator();
    test_pool_allocator();
    test_cy_strings();

    return exit_code;
}
