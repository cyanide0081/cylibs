#include "cy.h"
#include "testutils.h"

static i32 exit_code;

static void test_page_allocator(void)
{
    printf("%sTesting Page Allocator...%s\n", VT_BOLD, VT_RESET);

    FILE *f = fopen("sample.txt", "r");
    if (f == NULL) {
        print_e("unable to open file: %s", strerror(errno));
        TEST_FAIL();
    }

    fseek(f, 0, SEEK_END);
    size_t txt_len = ftell(f);
    rewind(f);

    CyAllocator a = cy_page_allocator();

    size_t txt_size = txt_len + 1;
    char *txt_buf = cy_alloc(a, txt_size);
    print_s("allocated page");

    if ((uintptr)txt_buf % CY_DEFAULT_ALIGNMENT != 0) {
        print_e("returned memory is not properly aligned");
        TEST_FAIL();
    }

    print_s("validated memory alignment");

    fread(txt_buf, sizeof(u8), txt_len, f);
    print_s("allocated message (%.2lfKB) (page size: %.2lfKB)",
        txt_len / KB, cy_page_allocator_alloc_size(txt_buf) / KB);

    {
        void *new_buf = cy_resize(a, txt_buf, txt_size, 0x80);
        if (new_buf == NULL) {
            print_e("unable to shrink page size: %s", strerror(errno));
            TEST_FAIL();
        }

        txt_buf = new_buf;
        txt_size = 0x80;
        print_s("shrunk page size (%.2lfKB)", txt_size / KB);
    }
    {
        void *new_buf = cy_resize(a, txt_buf, txt_size, 0x100000);
        if (new_buf == NULL) {
            print_e("unable to expand page size: %s", strerror(errno));
            TEST_FAIL();
        }

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
    if (f == NULL) {
        print_e("unable to open file: %s", strerror(errno));
        TEST_FAIL();
    }

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
        if (txt_buf == NULL) {
            print_e("unable to expand buffer in arena");
            TEST_FAIL();
        }

        print_s("expanded message buffer (%.2lfKB)", expanded_size / KB);
    }
    {
        cy_free_all(a);
        CY_ASSERT(arena.state.first_node->offset == 0);
        f64 size = arena.state.first_node->size / KB;
        print_s("freed whole arena (Available size: %.2lfKB)", size);
    }
    {
        size_t shrunk_size = expanded_size / 4;
        txt_buf = cy_resize(a, txt_buf, expanded_size, shrunk_size);
        if (txt_buf == NULL) {
            print_e("unable to shrink buffer in arena");
            TEST_FAIL();
        }

        print_s("shrunk message buffer (%.2lfKB)", shrunk_size / KB);
    }
    {
        CyArenaNode *first_node = arena.state.first_node;
        while (first_node->offset < first_node->size) (void)cy_alloc_align(a, 0x1, 1);

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
            (i32)cy_cstring_len(str_buf), str_buf
        );
    }
    {
        cy_free_all(a);
        CY_ASSERT(arena.state.first_node->offset == 0);
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

    {
        CyString s = cy_string_create(a, "Hello, World!");
        CY_ASSERT(cy_string_len(s) == 13);
    }

}

int main(void)
{
    test_page_allocator();
    test_arena_allocator();
    test_cy_strings();

    return exit_code;
}
