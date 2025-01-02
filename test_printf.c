#define CY_IMPLEMENTATION
#include "cy.h"

extern int printf(const char*, ...);

int main(void)
{
    int written = -1;
    f64 f = 766.3e8;

    char buf[1024];
    const char *str = "hello :)";
    cy_str_copy(buf, str);
    cy_printf("str:              %19c`%s`\n", ' ', buf);
    cy_printf("cy_str_copy(str): %19c`%s`\n", ' ', buf);

    cy_printf("&str:       %19c%p\n", ' ', (void*)str);
    cy_printf("&str(libc): %19c%p\n", ' ', (void*)str);

    cy_printf("ext int:             % 026i\n", 12345);
    cy_printf("ext int(libc):       % 026i\n", 12345);

    cy_printf("ext float:           %+026e\n", f);
    cy_printf("ext float(libc):     %+026e\n", f);
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

    return 0;
}
