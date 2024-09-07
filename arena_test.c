#include <stdlib.h>
#include <errno.h>

#include "cy.h"
#include "testutils.h"

int main(void) {
    printf("%sTesting Arena Allocator...%s\n", VT_BOLD, VT_RESET);

    FILE *f = fopen("sample.txt", "r");
    if (f == NULL) {
        print_e("unable to open file: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    fseek(f, 0, SEEK_END);
    isize txt_len = ftell(f);
    rewind(f);

    CyArena arena = cy_arena_init(cy_heap_allocator(), txt_len + 1);
    CyAllocator a = cy_arena_allocator(&arena);
    print_s("initialized arena");

    isize txt_size = txt_len + 1;
    char *txt_buf = cy_alloc(a, txt_size);
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
            exit(EXIT_FAILURE);
        }

        print_s("expanded message buffer (%.2lfKB)", expanded_size / KB);
    }
    {
        CyArenaNode *first_node = arena.state.first_node;
        while (arena.state.first_node == first_node) (void)cy_alloc(a, 0x1);

        print_s("exhausted arena (ofs: %zu, size: %zu)",
            first_node->offset, first_node->size);
    }
    {
        size_t shrunk_size = expanded_size / 4;
        txt_buf = cy_resize(a, txt_buf, expanded_size, shrunk_size);
        if (txt_buf == NULL) {
            print_e("unable to shrink buffer in arena");
            exit(EXIT_FAILURE);
        }

        print_s("shrunk message buffer (%.2lfKB)", shrunk_size / KB);
    }
    {
        u8 val = 69;
        CyArenaNode *first_node = arena.state.first_node;
        memset(first_node->buf, val, first_node->size);
        print_s("wrote [%u] to arena (%.2lfKB)", val, first_node->size / KB);
    }
    {
        const char *str = "basolutely.";
        size_t str_len = strlen(str);
        char *str_buf = cy_alloc_string_len(a, str, str_len);
        print_s(
            "allocated string into arena (str_buf: '%.*s')",
            (int)str_len, str_buf
        );
    }

    arena_deinit(&arena);
    print_s("deinitialized arena");

    return 0;
}
