#define CY_IMPLEMENTATION
#include "cy.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

extern int printf(const char*, ...);

int main(void)
{
    int written = -1;
    f64 f = 766.3e8;

    int n = 2048;
    cy_printf("\nbig printf: `%0*d%0*d`\n", n, n, n, n);

    char buf[1024];
    isize buf_size = CY_ARRAY_LEN(buf);
    const char *str = "hello :)";
    cy_str_copy(buf, str);
    cy_printf("str:              %19c`%s`\n", ' ', str);
    cy_printf("cy_str_copy(str): %19c`%s`\n", ' ', buf);

    char smol_buf[32];
    isize len = cy_sprintf(smol_buf, sizeof(smol_buf), "%.50f", f);
    cy_printf("smol_buf[%zu]: `%s`\n", sizeof(smol_buf), smol_buf);
    cy_printf("cy_sprintf returned: %zd\n", len);

    cy_printf("&str:       %35p\n", (void*)str);
       printf("&str(libc): %35p\n", (void*)str);

    cy_printf("ext int:             % 026i\n", 12345);
    printf("ext int(libc):       % 026i\n", 12345);

    cy_printf("ext float:           %+026e\n", f);
    printf("ext float(libc):     %+026e\n", f);
    cy_printf("\n");

    isize ret = cy_printf(
        "i am a text buffer %9c(not emacs!!)\n"
        "string:        %26.*s\n"
        "char:          %26c\n"
        "unsigned long: %26.5lo\n"
        "exp float:     %+26e\n"
        "hex float:     %26a\n"
        "%n",
        ' ', 4, str, 'W', 07131UL, f, f,
        &written
    );
    cy_printf("cy_printf returned:    %18zd\n", ret);
    cy_printf("written:               %18d\n", written);
    cy_printf("\n");

    CyTicks start, elapsed;
    const char *fmt = "|%13.4a|%13.4f|%13.4e|%13.4g|\n";

    cy_printf("|------------------- CONVERSION TEST -------------------|\n\n");
    cy_printf("cy_printf:\n");

    start = cy_ticks_query();

    f = 0.0000;
    cy_printf(fmt, f, f, f, f);
    f = 0.5000;
    cy_printf(fmt, f, f, f, f);
    f = 1.0000;
    cy_printf(fmt, f, f, f, f);
    f = -1.0000;
    cy_printf(fmt, f, f, f, f);
    f = 100.0000;
    cy_printf(fmt, f, f, f, f);
    f = 1000.0000;
    cy_printf(fmt, f, f, f, f);
    f = 10000.0000;
    cy_printf(fmt, f, f, f, f);
    f = 12345.0000;
    cy_printf(fmt, f, f, f, f);
    f = 100000.0000;
    cy_printf(fmt, f, f, f, f);
    f = 123456.0000;
    cy_printf(fmt, f, f, f, f);

    elapsed = cy_ticks_elapsed(start, cy_ticks_query());
    cy_printf(
        "time elapsed: %.3fμs\n",
        cy_ticks_to_time_unit(elapsed, CY_MICROSECONDS)
    );

    cy_printf("\n");
    cy_printf("printf(libc):\n");

    start = cy_ticks_query();

    f = 0.0000;
    printf(fmt, f, f, f, f);
    f = 0.5000;
    printf(fmt, f, f, f, f);
    f = 1.0000;
    printf(fmt, f, f, f, f);
    f = -1.0000;
    printf(fmt, f, f, f, f);
    f = 100.0000;
    printf(fmt, f, f, f, f);
    f = 1000.0000;
    printf(fmt, f, f, f, f);
    f = 10000.0000;
    printf(fmt, f, f, f, f);
    f = 12345.0000;
    printf(fmt, f, f, f, f);
    f = 100000.0000;
    printf(fmt, f, f, f, f);
    f = 123456.0000;
    printf(fmt, f, f, f, f);

    elapsed = cy_ticks_elapsed(start, cy_ticks_query());
    cy_printf(
        "time elapsed: %.3fμs\n",
        cy_ticks_to_time_unit(elapsed, CY_MICROSECONDS)
    );

    cy_printf("\n");
    cy_printf("stb_printf:\n");

    start = cy_ticks_query();

    f = 0.0000;
    stbsp_snprintf(buf, buf_size, fmt, f, f, f, f);
    cy_printf("%s", buf);
    f = 0.5000;
    stbsp_snprintf(buf, buf_size, fmt, f, f, f, f);
    cy_printf("%s", buf);
    f = 1.0000;
    stbsp_snprintf(buf, buf_size, fmt, f, f, f, f);
    cy_printf("%s", buf);
    f = -1.0000;
    stbsp_snprintf(buf, buf_size, fmt, f, f, f, f);
    cy_printf("%s", buf);
    f = 100.0000;
    stbsp_snprintf(buf, buf_size, fmt, f, f, f, f);
    cy_printf("%s", buf);
    f = 1000.0000;
    stbsp_snprintf(buf, buf_size, fmt, f, f, f, f);
    cy_printf("%s", buf);
    f = 10000.0000;
    stbsp_snprintf(buf, buf_size, fmt, f, f, f, f);
    cy_printf("%s", buf);
    f = 12345.0000;
    stbsp_snprintf(buf, buf_size, fmt, f, f, f, f);
    cy_printf("%s", buf);
    f = 100000.0000;
    stbsp_snprintf(buf, buf_size, fmt, f, f, f, f);
    cy_printf("%s", buf);
    f = 123456.0000;
    stbsp_snprintf(buf, buf_size, fmt, f, f, f, f);
    cy_printf("%s", buf);

    elapsed = cy_ticks_elapsed(start, cy_ticks_query());
    cy_printf(
        "time elapsed: %.3fμs\n",
        cy_ticks_to_time_unit(elapsed, CY_MICROSECONDS)
    );
    cy_printf("\n");

    unsigned oct = 1233;
    cy_printf("octal:        %o\n", oct);
    cy_printf("octal#:       %#o\n", oct);

    cy_printf("octal(libc):  %o\n", oct);
    cy_printf("octal#(libc): %#o\n", oct);

    f32 flt = 3.00;
    cy_printf("float:        %g\n", flt);
    cy_printf("float#:       %#g\n", flt);

    cy_printf("float(libc):  %g\n", flt);
    cy_printf("float#(libc): %#g\n", flt);

    len = cy_sprintf(NULL, 0, "%010d%n", 314, &n);
    cy_printf("len: %zd, n: %d\n", len, n);

    CyString s = cy_string_create_reserve(cy_heap_allocator(), len);
    cy_sprintf(s, cy_string_cap(s) + 1, "%010d%n", 314, &n);

    cy_printf("string: `%s`\n", s);

    cy_printf("bool: %q\n", true);
    cy_printf("binary int: %#b\n", 1024);

    CyStringView v = cy_string_view_create_c("hi i'm a stringview");
    cy_printf("stringview: `%v`\n", v);

    u32 u = 24;
    cy_printf("sized int: %32\n", u);

    return 0;
}
