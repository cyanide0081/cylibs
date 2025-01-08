#ifndef _TEST_UTILS_H
#define _TEST_UTILS_H

#define KB 1024.0

#define VT_BOLD "\x1B[1m"
#define VT_RESET "\x1B[0m"
#define VT_FG_GREEN "\x1B[92m"
#define VT_FG_RED "\x1B[91m"

#define SP "[" VT_FG_GREEN "🗸" VT_RESET "] "
#define EP "[" VT_FG_RED "🗙" VT_RESET "] "

#define print_s(...) cy_printf(SP "Success: " __VA_ARGS__); cy_printf("\n")
#define print_e(...) cy_printf(EP "Failure: " __VA_ARGS__); cy_printf("\n")

#define TEST_ASSERT(cond, ...) { \
    if (!(cond)) { \
        print_e(__VA_ARGS__); \
        exit_code = -1; \
        return; \
    } \
} (void)0

#define TEST_ASSERT_NOT_NULL(ptr, ...) TEST_ASSERT((ptr) != NULL, __VA_ARGS__)

#endif /* _TEST_UTILS_H */
