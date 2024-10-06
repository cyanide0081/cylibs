#include "cy.h"
#include "testutils.h"

static i32 exit_code;

static void test_page_allocator(void)
{
    printf("%sTesting Page Allocator...%s\n", VT_BOLD, VT_RESET);

    FILE *f = fopen("sample.txt", "r");
    TEST_ASSERT_NOT_NULL(f, "unable to open file: %s", strerror(errno));

    fseek(f, 0, SEEK_END);
    size_t txt_len = ftell(f);
    rewind(f);

    CyAllocator a = cy_page_allocator();

    size_t txt_size = txt_len + 1;
    char *txt_buf = cy_alloc(a, txt_size);
    print_s("allocated page");

    TEST_ASSERT(
        (uintptr)txt_buf % CY_DEFAULT_ALIGNMENT == 0,
        "returned memory is not properly aligned"
    );
    print_s("validated memory alignment");

    fread(txt_buf, sizeof(u8), txt_len, f);
    print_s("allocated message (%.2lfKB) (page size: %.2lfKB)",
        txt_len / KB, cy_page_allocator_alloc_size(txt_buf) / KB);

    {
        void *new_buf = cy_resize(a, txt_buf, txt_size, 0x80);
        TEST_ASSERT_NOT_NULL(
            new_buf, "unable to shrink page size: %s", strerror(errno)
        );

        txt_buf = new_buf;
        txt_size = 0x80;
        print_s("shrunk page size (%.2lfKB)", txt_size / KB);
    }
    {
        void *new_buf = cy_resize(a, txt_buf, txt_size, 0x100000);
        TEST_ASSERT_NOT_NULL(
            new_buf, "unable to expand page size: %s", strerror(errno)
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
    fclose(f);
}

static void test_arena_allocator(void)
{
    printf("%sTesting Arena Allocator...%s\n", VT_BOLD, VT_RESET);

    FILE *f = fopen("sample.txt", "r");
    TEST_ASSERT_NOT_NULL(f, "unable to open file: %s", strerror(errno));

    fseek(f, 0, SEEK_END);
    isize txt_len = ftell(f);
    rewind(f);

    CyArena arena = cy_arena_init(cy_heap_allocator(), txt_len + 1);
    CyAllocator a = cy_arena_allocator(&arena);
    print_s("initialized arena");

    isize txt_size = txt_len + 1;
    char *txt_buf = cy_alloc_align(a, txt_size, 1);
    fread(txt_buf, sizeof(char), txt_len, f);
    fclose(f);
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
        cy_free_all(a);
        TEST_ASSERT(
            arena.state.first_node->offset == 0, "unexpected arena offset"
        );
        f64 size = arena.state.first_node->size / KB;
        print_s("freed whole arena (Available size: %.2lfKB)", size);
    }
    {
        size_t shrunk_size = expanded_size / 4;
        txt_buf = cy_resize(a, txt_buf, expanded_size, shrunk_size);
        TEST_ASSERT_NOT_NULL(txt_buf, "unable to shrink buffer in arena");

        print_s("shrunk message buffer (%.2lfKB)", shrunk_size / KB);
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

    arena_deinit(&arena);
    print_s("deinitialized arena");
}

static void test_cy_strings(void)
{
    printf("%sTesting CyStrings...%s\n", VT_BOLD, VT_RESET);

    CyArena arena = cy_arena_init(cy_heap_allocator(), 0x1000);
    CyAllocator a = cy_arena_allocator(&arena);

    isize len = 13;
    CyString str = cy_string_create(a, "Hello, World!");
    TEST_ASSERT_NOT_NULL(str, "string creation failed");
    TEST_ASSERT(cy_string_len(str) == len, "unexpected string length");
    TEST_ASSERT(cy_string_cap(str) == len, "unexpected string capacity");

    print_s("created new string '%s'", str);

<<<<<<< Updated upstream
    const char *suffix = "我爱你";
    str = cy_string_append_c(str, " ");
    TEST_ASSERT_NOT_NULL(str, "unable to append string");
    str = cy_string_append_c(str, suffix);
=======
<<<<<<< Updated upstream
    const char *other = "我爱你";
    str = cy_string_append_c(str, " ");
    TEST_ASSERT_NOT_NULL(str, "unable to append string");
    str = cy_string_append_c(str, other);
=======
<<<<<<< HEAD
    const char *suffix = "我爱你";
    str = cy_string_append_c(str, " ");
    TEST_ASSERT_NOT_NULL(str, "unable to append string");
    str = cy_string_append_c(str, suffix);
=======
    const char *other = "我爱你";
    str = cy_string_append_c(str, " ");
    TEST_ASSERT_NOT_NULL(str, "unable to append string");
    str = cy_string_append_c(str, other);
>>>>>>> 8841a63 (repurpose stack allocator)
>>>>>>> Stashed changes
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
    str = cy_string_append_c(str, " \t  ");
    TEST_ASSERT_NOT_NULL(str, "unable to append to string");
=======
<<<<<<< Updated upstream
    str = cy_string_append_c(str, " \t\t   ");
    str = cy_string_trim(str, " \t");
=======
<<<<<<< HEAD
    str = cy_string_append_c(str, " \t  ");
    TEST_ASSERT_NOT_NULL(str, "unable to append to string");
=======
    str = cy_string_append_c(str, " \t\t   ");
    str = cy_string_trim(str, " \t");
    TEST_ASSERT_NOT_NULL(str, "unable to trim string");
    TEST_ASSERT(
        cy_string_len(str) == old_len, "incorrectly trimmed string: '%s'", str
    );
>>>>>>> 8841a63 (repurpose stack allocator)
>>>>>>> Stashed changes

    print_s("appended whitespace to string, result: '%s'", str);

    str = cy_string_trim_trailing_whitespace(str);
<<<<<<< Updated upstream
=======
>>>>>>> Stashed changes
>>>>>>> Stashed changes
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
    test_cy_strings();

    return exit_code;
}
