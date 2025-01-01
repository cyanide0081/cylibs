#define CY_IMPLEMENTATION
#include "cy.h"

int main(void)
{
    char buf[1024];
    isize buf_size = CY_ARRAY_LEN(buf);
    int written = -1;
    f64 f = 766.3e8;

    const char *str = "hello :)";
    cy_str_copy(buf, str);
    printf("str:              `%s`\n", buf);
    printf("cy_str_copy(str): `%s`\n", buf);
    
    cy_sprintf(buf, buf_size, "&str:       %p", (void*)str);
    printf("%s\n", buf);
    
    printf("&str(libc): %p\n", (void*)str);
    
    cy_sprintf(buf, buf_size, "ext int:             % 026i", 12345);
    printf("%s\n", buf);
    printf("ext int(libc):       % 026i\n", 12345);
        
    cy_sprintf(buf, buf_size, "ext float:           %+026e", f);
    printf("%s\n", buf);
    printf("ext float(libc):     %+026e\n", f);
    
    isize ret = cy_sprintf(
        buf, buf_size,
        "i am a text buffer (not emacs!!)\n"
        "string:        %26.*s\n"
        "char:          %26c\n"
        "unsigned long: %26.5lo\n"
        "exp float:     %+26e\n"
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
    printf("printf(libc):\n");

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
    printf("\n");

    unsigned oct = 1233;
    cy_sprintf(buf, buf_size, "octal:        %o", oct);
    printf("%s\n", buf);
    cy_sprintf(buf, buf_size, "octal#:       %#o", oct);
    printf("%s\n", buf);

    printf("octal(libc):  %o\n", oct);
    printf("octal(libc)#: %#o\n", oct);
    
    f32 flt = 3.00;
    cy_sprintf(buf, buf_size, "float:        %g", flt);
    printf("%s\n", buf);
    cy_sprintf(buf, buf_size, "float#:       %#g", flt);
    printf("%s\n", buf);

    printf("float(libc):  %g\n", flt);
    printf("float(libc)#: %#g\n", flt);

    return 0;
}
