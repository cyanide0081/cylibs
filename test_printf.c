#define CY_IMPLEMENTATION
#include "cy.h"

int main(int argc, char *argv[])
{
    const char *str = "hello!";
    char buf[1024];
    isize buf_size = CY_ARRAY_LEN(buf);
    int written = -1;
    f64 f = 766.3e8;
    isize ret = cy_sprintf(
        buf, buf_size,
        "i am a text buffer (not emacs!!)\n"
        "string:        %26.*s\n"
        "char:          %26c\n"
        "unsigned long: %+26.5lo\n"
        "exp float:     %0+26e\n"
        "hex float:     %26a\n"
        "%n",
        4, str, 'W', 07131UL, f, f,
        &written
    );
    printf("%s\n", buf);
    printf("cy_sprintf returned:   %18zd\n", ret);
    printf("strlen(buf):           %18zd\n", cy_str_len(buf));
    printf("written:               %18d\n", written);
    printf("\n");

    CyTicks start, elapsed;
    const char *fmt = "|%13.4a|%13.4f|%13.4e|%13.4g|\n";

    printf("|------------------- CONVERSION TEST -------------------|\n\n");
    printf("cy_printf:\n");

    start = cy_ticks_query();

    f = 0.0000;
    cy_sprintf(buf, buf_size, fmt, f, f, f, f);
    printf("%s", buf);
    f = 0.5000;
    cy_sprintf(buf, buf_size, fmt, f, f, f, f);
    printf("%s", buf);
    f = 1.0000;
    cy_sprintf(buf, buf_size, fmt, f, f, f, f);
    printf("%s", buf);
    f = -1.0000;
    cy_sprintf(buf, buf_size, fmt, f, f, f, f);
    printf("%s", buf);
    f = 100.0000;
    cy_sprintf(buf, buf_size, fmt, f, f, f, f);
    printf("%s", buf);
    f = 1000.0000;
    cy_sprintf(buf, buf_size, fmt, f, f, f, f);
    printf("%s", buf);
    f = 10000.0000;
    cy_sprintf(buf, buf_size, fmt, f, f, f, f);
    printf("%s", buf);
    f = 12345.0000;
    cy_sprintf(buf, buf_size, fmt, f, f, f, f);
    printf("%s", buf);
    f = 100000.0000;
    cy_sprintf(buf, buf_size, fmt, f, f, f, f);
    printf("%s", buf);
    f = 123456.0000;
    cy_sprintf(buf, buf_size, fmt, f, f, f, f);
    printf("%s", buf);

    elapsed = cy_ticks_elapsed(start, cy_ticks_query());
    printf(
        "time elapsed: %.3fμs\n",
        cy_ticks_to_time_unit(elapsed, CY_MICROSECONDS)
    );

    printf("\n");
    printf("printf(glibc):\n");

    start = cy_ticks_query();

    f = 0.0000;
    snprintf(buf, buf_size, fmt, f, f, f, f);
    printf("%s", buf);
    f = 0.5000;
    snprintf(buf, buf_size, fmt, f, f, f, f);
    printf("%s", buf);
    f = 1.0000;
    snprintf(buf, buf_size, fmt, f, f, f, f);
    printf("%s", buf);
    f = -1.0000;
    snprintf(buf, buf_size, fmt, f, f, f, f);
    printf("%s", buf);
    f = 100.0000;
    snprintf(buf, buf_size, fmt, f, f, f, f);
    printf("%s", buf);
    f = 1000.0000;
    snprintf(buf, buf_size, fmt, f, f, f, f);
    printf("%s", buf);
    f = 10000.0000;
    snprintf(buf, buf_size, fmt, f, f, f, f);
    printf("%s", buf);
    f = 12345.0000;
    snprintf(buf, buf_size, fmt, f, f, f, f);
    printf("%s", buf);
    f = 100000.0000;
    snprintf(buf, buf_size, fmt, f, f, f, f);
    printf("%s", buf);
    f = 123456.0000;
    snprintf(buf, buf_size, fmt, f, f, f, f);
    printf("%s", buf);

    elapsed = cy_ticks_elapsed(start, cy_ticks_query());
    printf(
        "time elapsed: %.3fμs\n",
        cy_ticks_to_time_unit(elapsed, CY_MICROSECONDS)
    );

    return 0;
}
