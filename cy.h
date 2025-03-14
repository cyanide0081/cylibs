#ifndef CY__H_INCLUDE
#define CY__H_INCLUDE

/**
 * A single-header libc replacement for modern systems (32/64-bit machines)
 * (this will be better documented eventually)
 *
 * NOTE: many foundational components/interfaces of this library
 * (allocators, strings, file I/O, etc.) are borrowed from/inspired by
 *  GingerBill's gb library -> https://github.com/gingerBill/gb
 */

/******************************************************************************
 *                               DECLARATIONS                                 *
 ******************************************************************************/
#if defined(_MSC_VER)
    #define CY_COMPILER_MSVC 1
#elif defined(__GNUC__)
    #define CY_COMPILER_GCC 1
#elif defined(__clang__)
    #define CY_COMPILER_CLANG 1
#else
    #error Unsupported compiler
#endif

#if defined(_WIN32) || defined(_WIN64)
    #define CY_OS_WINDOWS 1
    #ifndef __MINGW32__
        #ifndef _CRT_SECURE_NO_WARNINGS
            #define _CRT_SECURE_NO_WARNINGS 1
        #endif
    #endif

    #define WIN32_LEAN_AND_MEAN 1
    #define WIN32_MEAN_AND_LEAN 1
    #define VC_EXTRALEAN 1
    #define NOMINMAX 1

    #include <windows.h>

    #undef NOMINMAX
    #undef VC_EXTRALEAN
    #undef WIN32_MEAN_AND_LEAN
    #undef WIN32_LEAN_AND_MEAN
#elif defined(__APPLE__) && defined(__MACH__)
    #define CY_OS_MACOSX 1
#elif defined(__unix__)
    #define CY_OS_UNIX 1
    #ifndef _DEFAULT_SOURCE
        #define _DEFAULT_SOURCE 1
    #endif

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

#if defined(_WIN64) || defined(__x86_64__) || defined(_M_X64) || \
    defined(__64BIT__) || defined(__powerpc64__) || defined(__ppc64__)
    #ifndef CY_ARCH_64_BIT
        #define CY_ARCH_64_BIT 1
    #endif
#else
    #ifndef CY_ARCH_32_BIT
        #define CY_ARCH_32_BIT 1
    #endif
#endif

#define CY_NOOP() ((void)0)

#ifndef CY_STATIC_ASSERT
    // NOTE(cya): because C macro expansion bizarreness
    #define CY__STATIC_ASSERT_INTERNAL_EX(cond, line) \
        typedef char STATIC_ASSERTION_FAILED_AT_LINE_##line[!!(cond) * 2 - 1]
    #define CY__STATIC_ASSERT_INTERNAL(cond, line) \
        CY__STATIC_ASSERT_INTERNAL_EX(cond, line)
    #define CY_STATIC_ASSERT(cond) CY__STATIC_ASSERT_INTERNAL(cond, __LINE__)
#endif

#ifndef NDEBUG
    #define CY_DEBUG 1
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

#ifndef CY_ASSERT_MSG
    #if defined(CY_DEBUG)
        #define CY_ASSERT_MSG(cond, ...) { \
            if (!(cond)) { \
                cy_handle_assertion( \
                    "Assertion failed",#cond, \
                    __FILE__, (i64)__LINE__, __VA_ARGS__ \
                ); \
                CY_DEBUG_TRAP(); \
            } \
        } CY_NOOP()
    #else
        #define CY_ASSERT_MSG(cond, ...) (void)(cond)
    #endif
#endif

#ifndef CY_ASSERT
    #define CY_ASSERT(cond) CY_ASSERT_MSG(cond, NULL)
    #define CY_ASSERT_NOT_NULL(ptr) CY_ASSERT(ptr != NULL)
#endif

#ifndef CY_PANIC
    #define CY_PANIC(...) { \
        cy_handle_assertion("PANIC", NULL, __FILE__, __LINE__, __VA_ARGS__); \
        CY_DEBUG_TRAP(); \
    } CY_NOOP()
#endif

#if defined(CY_STATIC)
    #define CY_DEF static
#else
    #define CY_DEF extern
#endif

#if defined(CY_STD_C_PRINTF) && (defined(__clang__) || defined(__GNUC__))
    #define CY__FMT_ATTR(fmt) __attribute__((format(printf, fmt, (fmt) + 1)))
#else
    #define CY__FMT_ATTR(fmt)
#endif

/* ================================= Types ================================== */
#include <stdint.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

CY_STATIC_ASSERT(sizeof(i8) == sizeof(u8));
CY_STATIC_ASSERT(sizeof(i16) == sizeof(u16));
CY_STATIC_ASSERT(sizeof(i32) == sizeof(u32));
CY_STATIC_ASSERT(sizeof(i64) == sizeof(u64));

CY_STATIC_ASSERT(sizeof(i8) == 1);
CY_STATIC_ASSERT(sizeof(i16) == 2);
CY_STATIC_ASSERT(sizeof(i32) == 4);
CY_STATIC_ASSERT(sizeof(i64) == 8);

typedef float f32;
typedef double f64;

CY_STATIC_ASSERT(sizeof(f32) == 4);
CY_STATIC_ASSERT(sizeof(f64) == 8);

#if defined(CY_OS_UNIX)
    #include <stddef.h>
#endif

typedef ptrdiff_t isize;
typedef size_t usize;

CY_STATIC_ASSERT(sizeof(isize) == sizeof(usize));

typedef intptr_t intptr;
typedef uintptr_t uintptr;

CY_STATIC_ASSERT(sizeof(intptr) == sizeof(uintptr));

typedef i8 b8;
typedef i16 b16;
typedef i32 b32; // NOTE(cya): should be the default ones

#ifndef true
    #define true (0 == 0)
#endif
#ifndef false
    #define false (0 != 0)
#endif

#define CY_KB(n) ((n) * 1024)
#define CY_MB(n) (CY_KB(n) * 1024)
#define CY_GB(n) (CY_MB(n) * 1024)

// NOTE(cya): forward declarations for using the types in other procedures
typedef char *CyString;
typedef struct CyStringView CyStringView;
typedef struct CyAllocator CyAllocator;

/* --------------------------------- Limits --------------------------------- */
#define U8_MIN 0U
#define U8_MAX 0xFFU
#define I8_MIN (-0x7F - 1)
#define I8_MAX 0x7F

#define U16_MIN 0U
#define U16_MAX 0xFFFFU
#define I16_MIN (-0x7FFF - 1)
#define I16_MAX 0x7FFF

#define U32_MIN 0U
#define U32_MAX 0xFFFFFFFFU
#define I32_MIN (-0x7FFFFFFF - 1)
#define I32_MAX 0x7FFFFFFF

#define U64_MIN 0U
#define U64_MAX 0xFFFFFFFFFFFFFFFFULL
#define I64_MIN (-0x7FFFFFFFFFFFFFFFLL - 1)
#define I64_MAX 0x7FFFFFFFFFFFFFFFFLL

#define F32_MIN 1.17549435e-38F
#define F32_MAX 3.40282347e+38F

#define F64_MIN 2.2250738585072014e-308
#define F64_MAX 1.7976931348623157e+308

#if defined(CY_ARCH_64_BIT)
    #define ISIZE_MIN I64_MIN
    #define ISIZE_MAX I64_MAX

    #define USIZE_MIN U64_MIN
    #define USIZE_MAX U64_MAX
#else
    #define ISIZE_MIN I32_MIN
    #define ISIZE_MAX I32_MAX

    #define USIZE_MIN U32_MIN
    #define USIZE_MAX U32_MAX
#endif

typedef i32 Rune; // Unicode codepoint

#define CY_RUNE_INVALID (Rune)(0XFFFD)
#define CY_RUNE_MAX (Rune)(0X0010FFFF)
#define CY_RUNE_BOM (Rune)(0XFEFF)
#define CY_RUNE_EOF (Rune)(-1)

// NOTE(cya): should clarify the meaning behind keyword usage
#define cy_global static
#define cy_internal static
#define cy_persist static

// TODO(cya): force inline
#define cy_inline inline

// NOTE(cya): so we don't have implicit changes in signedness
#define cy_sizeof(x) (isize)(sizeof(x))

/* ================================= Runtime ================================ */
#ifndef CY_OS_WINDOWS
    #include <stdarg.h>  // va_args
#endif

#define CY_BIT(n) (1 << (n))
#define CY_UNUSED(param) (void)(param)
#define CY_SWAP(Type, a, b) { \
    Type _tmp = a; \
    a = b; \
    b = _tmp; \
} CY_NOOP()

#define CY_ABS(n) (n < 0 ? -(n) : n)
#define CY_IS_IN_RANGE_INCL(n, lo, hi) (n >= lo && n <= hi)
#define CY_IS_IN_RANGE_EXCL(n, lo, hi) (n >= lo && n < hi)
#define CY_IS_IN_BETWEEN(n, lo, hi) (n > lo && n < hi)

#if defined(__builtin_inff64)
    #define CY__INF __builtin_inff64()
#else
    #define CY__HUGE_VAL 1e+300
    #define CY__INF (CY__HUGE_VAL * CY__HUGE_VAL)
#endif

#define CY_IS_INF(n) (n >= +CY__INF || n <= -CY__INF)

#if defined(__builtin_isnan)
    #define CY__IS_NAN(n) __builtin_is_nan(n)
#else
    #define CY__IS_NAN(n) (n != n)
#endif

#define CY_IS_NAN(n) CY__IS_NAN(n)

CY_DEF void cy_handle_assertion(
    const char *prefix, const char *cond, const char *file,
    i32 line, const char *msg, ...
);

CY_DEF void *cy_mem_copy(
    void *restrict dst, const void *restrict src, isize bytes
);
CY_DEF void *cy_mem_move(void *dst, const void *src, isize bytes);
CY_DEF void *cy_mem_set(void *dst, u8 val, isize bytes);
CY_DEF void *cy_mem_zero(void *dst, isize bytes);
CY_DEF isize cy_mem_compare(const void *a, const void *b, isize bytes);

CY_DEF b32 cy_is_power_of_two(isize n);

CY_DEF isize cy_align_forward_size(isize size, isize align);
CY_DEF uintptr cy_align_forward(uintptr ptr, isize align);
CY_DEF void *cy_align_forward_ptr(void *ptr, isize align);

/* Aligns a pointer forward accounting for both the size
 * of a header and the alignment */
CY_DEF isize cy_calc_header_padding(
    uintptr ptr, isize align, isize header_size
);

typedef struct CyTicks CyTicks;

typedef enum {
    CY_SECONDS = 1ULL,
    CY_MILISECONDS = 1000ULL,
    CY_MICROSECONDS = 1000000ULL,
    CY_NANOSECONDS = 1000000000ULL,
} CyTimeUnit;

CY_DEF CyTicks cy_ticks_query(void);
CY_DEF CyTicks cy_ticks_elapsed(CyTicks start, CyTicks end);
CY_DEF f64 cy_ticks_to_time_unit(CyTicks ticks, CyTimeUnit unit);

/* ================================== Files ================================= */
typedef enum {
    CY_FILE_MODE_READ = CY_BIT(0),
    CY_FILE_MODE_WRITE = CY_BIT(1),
    CY_FILE_MODE_APPEND = CY_BIT(2),
    CY_FILE_MODE_READ_WRITE = CY_BIT(3),

    CY__FILE_MODE_MODES =  CY_FILE_MODE_READ | CY_FILE_MODE_WRITE |
        CY_FILE_MODE_APPEND | CY_FILE_MODE_READ_WRITE
} CyFileMode;

typedef enum {
    CY_SEEK_WHENCE_BEGIN,
    CY_SEEK_WHENCE_CURRENT,
    CY_SEEK_WHENCE_END,
} CySeekWhenceType;

// TODO(cya): maybe change some of these names later
typedef enum {
    CY_FILE_ERROR_OUT_OF_MEMORY = -1,
    CY_FILE_ERROR_NONE,
    CY_FILE_ERROR_INVALID,
    CY_FILE_ERROR_INVALID_FILENAME,
    CY_FILE_ERROR_EXISTS,
    CY_FILE_ERROR_NOT_FOUND,
    CY_FILE_ERROR_ACCESS_DENIED,
    CY_FILE_ERROR_TRUNCATION_FAILED,
} CyFileError;

typedef union {
    void *p;
    uintptr u;
    intptr i;
} CyFileDescriptor;

typedef struct CyFileOps CyFileOps;

#define CY_FILE_OPEN_PROC(name) CyFileError name( \
    CyFileDescriptor *fd, CyFileOps *ops, \
    i32 mode, const char *filename \
)
#define CY_FILE_READ_AT_PROC(name) b32 name( \
    CyFileDescriptor fd, void *buf, \
    isize size, isize offset, isize *bytes_read \
)
#define CY_FILE_WRITE_AT_PROC(name) b32 name( \
    CyFileDescriptor fd, const void *buf, \
    isize size, isize offset, isize *bytes_written \
)
#define CY_FILE_SEEK_PROC(name) b32 name( \
    CyFileDescriptor fd, isize offset, \
    CySeekWhenceType whence, isize *new_offset \
)
#define CY_FILE_CLOSE_PROC(name) void name(CyFileDescriptor fd)

typedef CY_FILE_OPEN_PROC(CyFileOpenProc);
typedef CY_FILE_READ_AT_PROC(CyFileReadAtProc);
typedef CY_FILE_WRITE_AT_PROC(CyFileWriteAtProc);
typedef CY_FILE_SEEK_PROC(CyFileSeekProc);
typedef CY_FILE_CLOSE_PROC(CyFileCloseProc);

struct CyFileOps {
    CyFileReadAtProc *read_at;
    CyFileWriteAtProc *write_at;
    CyFileSeekProc *seek;
    CyFileCloseProc *close;
};

extern const CyFileOps cy__default_file_ops;

typedef u64 CyFileTime;

typedef struct {
    CyFileOps ops;
    CyFileDescriptor fd;
    const char *filename;
    CyFileTime last_write_time;
} CyFile;

// TODO(cya): async file and dir info (?)

typedef enum {
    CY_FILE_STD_IN,
    CY_FILE_STD_OUT,
    CY_FILE_STD_ERR,
    CY__FILE_STD_COUNT
} CyFileStdType;

CY_DEF const char *cy_file_error_as_str(CyFileError err);

CY_DEF CyFile *cy_file_get_std_handle(CyFileStdType type);

// TODO(cya): temp file proc (?)

CY_DEF CyFileError cy_file_create(CyFile *f, const char *filename);
CY_DEF CyFileError cy_file_open(CyFile *f, const char *filename);
CY_DEF CyFileError cy_file_open_with_mode(
    CyFile *f, i32 mode, const char *filename
);
CY_DEF CyFileError cy_file_new(
    CyFile *f, CyFileDescriptor fd, CyFileOps ops, const char *filename
);
CY_DEF CyFileError cy_file_close(CyFile *f);
CY_DEF CyFileError cy_file_truncate(CyFile *f, isize size);

CY_DEF b32 cy_file_read_at_report(
    CyFile *f, void *buf,
    isize size, isize offset, isize *bytes_read
);
CY_DEF b32 cy_file_write_at_report(
    CyFile *f, const void *buf,
    isize size, isize offset, isize *bytes_written
);
CY_DEF b32 cy_file_read_at(CyFile *f, void *buf, isize size, isize offset);
CY_DEF b32 cy_file_write_at(
    CyFile *f, const void *buf, isize size, isize offset
);
CY_DEF b32 cy_file_read(CyFile *f, void *buf, isize size);
CY_DEF b32 cy_file_write(CyFile *f, const void *buf, isize size);

CY_DEF isize cy_file_seek(CyFile *f, isize offset);
CY_DEF isize cy_file_seek_to_end(CyFile *f);
CY_DEF isize cy_file_skip(CyFile *f, isize bytes);
CY_DEF isize cy_file_tell(CyFile *f);

CY_DEF isize cy_file_size(CyFile *f);
CY_DEF const char *cy_file_name(CyFile *f);
CY_DEF b32 cy_file_has_changed(CyFile *f);


CY_DEF b32 cy_file_path_exists(const char *filename);
CY_DEF CyFileTime cy_file_path_last_write_time(const char *filename);
CY_DEF b32 cy_file_path_copy(
    const char *cur_filename, const char *new_filename, b32 fail_if_exists
);
CY_DEF b32 cy_file_path_move(
    const char *cur_filename, const char *new_filename
);
CY_DEF b32 cy_file_path_remove(const char *filename);

#if defined(CY_OS_WINDOWS)
    #define CY_PATH_SEPARATOR '\\'
#else
    #define CY_PATH_SEPARATOR '/'
#endif

CY_DEF b32 cy_path_is_absolute(const char *path);
CY_DEF b32 cy_path_is_relative(const char *path);
CY_DEF b32 cy_path_is_root(const char *path);

CY_DEF const char *cy_path_base_name(const char *path);
CY_DEF const char *cy_path_extension(const char *path);

CY_DEF CyString cy_path_full_version(const char *path);

/* =================================== I/O ================================== */
CY_DEF isize cy_printf(const char *fmt, ...) CY__FMT_ATTR(1);
CY_DEF isize cy_printf_va(const char *fmt, va_list va);

// NOTE(cya): equivalent to fprintf(stderr, ...)
CY_DEF isize cy_eprintf(const char *fmt, ...) CY__FMT_ATTR(1);
CY_DEF isize cy_eprintf_va(const char *fmt, va_list va);

CY_DEF isize cy_fprintf(CyFile *f, const char *fmt, ...) CY__FMT_ATTR(2);
CY_DEF isize cy_fprintf_va(CyFile *f, const char *fmt, va_list va);

CY_DEF char *cy_aprintf(CyAllocator a, const char *fmt, ...) CY__FMT_ATTR(2);
CY_DEF char *cy_aprintf_va(CyAllocator a, const char *fmt, va_list va);

/**
 * Extended implementation of the snprintf function from the C99 standard
 * (no sprintf without a buffer size parameter because that'd be a bit silly)
 * NOTE: you can disable the extended formats by defining CY_STD_C_PRINTF
 * (you'll additionally get the compiler warnings for the format string in case
 * your compiler supports them, since enabling this with the extended version
 * would give you warnings when using well-defined format strings)
 *
 * This implementation should work as defined by the specification, so you can
 * expect every feature of snprintf, including the return value giving you the
 * theoretical amount of chars necessary for this conversion even if the buffer
 * you provided isn't large enough for the string to be stored in it
 *
 * If any conversion error occurs because of bad or missing parameters, an error
 * message will be written to the buffer, an argument will be consumed and
 * discarded and the format string parsing will continue
 *
 * Extended format specifiers:
 *   %q, %Q: takes in a b32, b16 or b8 value, outputs `true` or `false`
 *     (lower/uppercase)
 *   %v: takes in a CyStringView, outputs its text
 *   %b: takes in an unsigned int, outputs it in base 2 (binary)
 *
 * Explicit-sized integers:
 *   You can explicitly size any integer input flag by appending its size to it
 *   (e.g.: %u32, %i64, %b8, etc.) NOTE: if you specify both C-style length
 *   modifiers and the bitsize specifiers, the C-style ones will be ignored
 */
CY_DEF isize cy_sprintf(
    char *buf, isize size, const char *fmt, ...
) CY__FMT_ATTR(3);
CY_DEF isize cy_sprintf_va(char *buf, isize size, const char *fmt, va_list va);

/* ============================= Virtual memory ============================= */
typedef struct CyMemoryBlock CyMemoryBlock;
struct CyMemoryBlock {
    CyMemoryBlock *prev;
    void *start;
    isize size;
    isize offset; // NOTE(cya): used memory
    isize prev_offset; // NOTE(cya): for linear allocators
};

typedef struct CyOSMemoryBlock CyOSMemoryBlock;
struct CyOSMemoryBlock {
    CyMemoryBlock block;
    isize commit_size;
    isize total_size;
    CyOSMemoryBlock *prev, *next;
};

CY_DEF isize cy_virtual_memory_page_size(isize *align_out);
CY_DEF CyMemoryBlock *cy_virtual_memory_alloc(isize size);
CY_DEF CyMemoryBlock *cy_virtual_memory_alloc_reserve(
    isize size, isize commit_size
);
CY_DEF void cy_virtual_memory_free(CyMemoryBlock *block);
CY_DEF CyMemoryBlock *cy_virtual_memory_resize(
    CyMemoryBlock *block, isize new_size
);


/* =============================== Allocators =============================== */
typedef enum {
    CY_ALLOCATION_ALLOC,
    CY_ALLOCATION_ALLOC_ALL,
    CY_ALLOCATION_FREE,
    CY_ALLOCATION_FREE_ALL,
    CY_ALLOCATION_RESIZE,
} CyAllocationType;

typedef enum {
    CY_ALLOCATOR_CLEAR_TO_ZERO = CY_BIT(0),
} CyAllocatorFlags;

#define CY_ALLOCATOR_PROC(name) void *name(      \
    void *allocator_data, CyAllocationType type, \
    isize size, isize align,                     \
    void *old_mem, isize old_size,               \
    u64 flags                                    \
)

typedef CY_ALLOCATOR_PROC(CyAllocatorProc);

struct CyAllocator {
    CyAllocatorProc *proc;
    void *data;
};

#define CY_DEFAULT_ALIGNMENT (2 * cy_sizeof(void*))
#define CY_DEFAULT_ALLOCATOR_FLAGS (CY_ALLOCATOR_CLEAR_TO_ZERO)

#define cy_alloc_item(allocator, type) cy_alloc(allocator, cy_sizeof(type))
#define cy_alloc_array(allocator, type, count) \
    cy_alloc(allocator, (count) * cy_sizeof(type))
#define cy_resize_array(allocator, arr, type, old_count, new_count) cy_resize( \
    allocator, arr, (old_count) * cy_sizeof(type), \
    (new_count) * cy_sizeof(type) \
)

CY_DEF void *cy_alloc_align(CyAllocator a, isize size, isize align);
CY_DEF void *cy_alloc(CyAllocator a, isize size);
CY_DEF void cy_free(CyAllocator a, void *ptr);
CY_DEF void cy_free_all(CyAllocator a);
CY_DEF void *cy_resize_align(
    CyAllocator a, void *ptr, isize old_size, isize new_size, isize align
);
CY_DEF void *cy_resize(
    CyAllocator a, void *ptr, isize old_size, isize new_size
);
// NOTE(cya): a simple resize that should fit most use cases
CY_DEF void *cy_default_resize_align(
    CyAllocator a, void *ptr, isize old_size, isize new_size, isize align
);
CY_DEF void *cy_default_resize(
    CyAllocator a, void *ptr, isize old_size, isize new_size
);
CY_DEF void *cy_alloc_copy_align(
    CyAllocator a, const void *src, isize size, isize align
);
CY_DEF void *cy_alloc_copy(CyAllocator a, const void *src, isize size);
CY_DEF char *cy_alloc_string_len(CyAllocator a, const char *str, isize len);
CY_DEF char *cy_alloc_string(CyAllocator a, const char *str);

// TODO(cya): figure out this sorcery
#define CY__LO_ONES ((usize)-1 / U8_MAX)
#define CY__HI_ONES (CY__LO_ONES * (U8_MAX / 2 + 1))
#define CY__HAS_ZERO_BYTE(n) \
    !!(((usize)(n) - CY__LO_ONES) & ~(usize)(n) & CY__HI_ONES)

#define CY_MIN(a, b) (a < b ? a : b)
#define CY_MAX(a, b) (a > b ? a : b)

// NOTE(cya): works with power-of-two alignments
#define CY__IS_ALIGNED(p, a) (((uintptr)(p) & (uintptr)((a) - 1)) == 0)
#define CY__IS_WORD_ALIGNED(p) CY__IS_ALIGNED(p, cy_sizeof(usize))

#define CY_VALIDATE_PTR(p) if (p == NULL) return NULL


// NOTE(cya): only works on arrays and literals (not on pointers)
#define CY_STR_LIT_LEN(str) (isize)((cy_sizeof(str) - 1) / cy_sizeof(*(str)))
#define CY_ARRAY_LEN(arr) (isize)((cy_sizeof(arr)) / cy_sizeof(*(arr)))

// NOTE(cya): for testing allocators and program behavior on OOM errors
CY_DEF CyAllocatorProc cy_null_allocator_proc;
CY_DEF CyAllocator cy_null_allocator(void);

// NOTE(cya): the default malloc-style heap allocator
CY_DEF CyAllocatorProc cy_heap_allocator_proc;
CY_DEF CyAllocator cy_heap_allocator(void);

#define cy_heap_alloc(size) cy_alloc(cy_heap_allocator(), size)
#define cy_heap_free(ptr) cy_free(cy_heap_allocator(), ptr)

/* ---------------------------- Static Allocator ---------------------------- */
typedef struct {
    void *memory;
    isize size;
} CyBuffer;

CY_DEF CyAllocatorProc cy_static_allocator_proc;
CY_DEF CyAllocator cy_static_allocator(CyBuffer *buf);

CY_DEF CyBuffer cy_static_buf_init(void *backing_buf, isize size);

/* ---------------------- Virtual Memory (VM) Allocator --------------------- */
CY_DEF CyAllocatorProc cy_virtual_memory_allocator_proc;
CY_DEF CyAllocator cy_virtual_memory_allocator(void);

/* ---------------------------- Arena Allocator ----------------------------- */
typedef struct {
    CyMemoryBlock *cur_block;
    CyAllocator backing;
} CyArena;

CY_DEF CyAllocatorProc cy_arena_allocator_proc;
CY_DEF CyAllocator cy_arena_allocator(CyArena *arena);

CY_DEF CyMemoryBlock *cy_arena_insert_block(CyArena *arena, isize size);
CY_DEF CyArena cy_arena_init(CyAllocator backing, isize initial_size);
CY_DEF void cy_arena_deinit(CyArena *arena);

/* ----------------------------- Stack Allocator ---------------------------- */
typedef struct CyStackNode {
    u8 *buf;
    isize size;
    isize prev_offset;
    isize offset;
    struct CyStackNode *next;
} CyStackNode;

typedef struct {
    CyStackNode *first_node;
} CyStackState;

typedef struct {
    CyAllocator backing;
    CyStackState state;
} CyStack;

typedef struct {
    isize prev_offset;
    isize padding;
} CyStackHeader;

CY_DEF CyAllocatorProc cy_stack_allocator_proc;
CY_DEF CyAllocator cy_stack_allocator(CyStack *stack);

CY_DEF CyStackNode *cy_stack_insert_node(CyStack *stack, isize size);
CY_DEF CyStack cy_stack_init(CyAllocator backing, isize initial_size);
CY_DEF void cy_stack_deinit(CyStack *stack);

/* ----------------------------- Pool Allocator ----------------------------- */
typedef struct {
    CyAllocator backing;
    void *memory;
    void *free_list_head;
    isize chunks;
    isize chunk_size;
    isize chunk_align;
} CyPool;

CY_DEF CyAllocatorProc cy_pool_allocator_proc;
CY_DEF CyAllocator cy_pool_allocator(CyPool *pool);

CY_DEF CyPool cy_pool_init(CyAllocator backing, isize chunks, isize chunk_size);
CY_DEF CyPool cy_pool_init_align(
    CyAllocator backing, isize chunks, isize chunk_size, isize chunk_align
);
CY_DEF void cy_pool_deinit(CyPool *pool);

// TODO(cya):
// * discover a nice interace to implement different OOM handling behaviors to
// * the basic allocators (static buf, linked list and pre-reserve/commit)
// * scratch allocator (ring buffer arena)
// * free-list allocator
// * buddy allocator
// * bitmap allocator ([bitmap of free blocks][contiguous pool of memory...])
// * virtual memory allocator (includes commit and reserve helper functions)
// * extra allocator strategies from https://www.youtube.com/watch?v=LIb3L4vKZ7U
// * general-purpose heap allocator composed of the basic ones to replace malloc
//   (final boss i guess - maybe implement one like TCMalloc?)

/* ============================== Char procs ================================ */
CY_DEF const char *cy_char_first_occurence(const char *str, char c);
CY_DEF const char *cy_char_last_occurence(const char *str, char c);

CY_DEF b32 cy_char_is_digit(char c);
CY_DEF b32 cy_char_is_hex_digit(char c);

// NOTE(cya): lower/uppercase related procs are ASCII only
CY_DEF b32 cy_char_is_lower(char c);
CY_DEF b32 cy_char_is_upper(char c);

CY_DEF b32 cy_char_is_in_set(char c, const char *cut_set);

CY_DEF char cy_char_to_lower(char c);
CY_DEF char cy_char_to_upper(char c);

CY_DEF i64 cy_digit_to_i64(char digit);
CY_DEF i64 cy_hex_digit_to_i64(char digit);

/* ============================ C-String procs ============================== */
CY_DEF isize cy_str_len(const char *str);
CY_DEF isize cy_wcs_len(const wchar_t *str);

CY_DEF char *cy_str_copy(char *dst, const char *src);

CY_DEF isize cy_str_compare(const char *a, const char *b);
CY_DEF isize cy_str_compare_n(const char *a, const char *b, isize len);

CY_DEF b32 cy_str_has_prefix(const char *str, const char *prefix);

CY_DEF char *cy_str_reverse(char *str);
CY_DEF char *cy_str_reverse_n(char *str, isize len);

CY_DEF void cy_str_to_lower(char *str);
CY_DEF void cy_str_to_upper(char *str);

CY_DEF u64 cy_str_to_u64(const char *str, i32 base, isize *len_out);
CY_DEF i64 cy_str_to_i64(const char *str, i32 base, isize *len_out);

CY_DEF isize cy_str_parse_u64(u64 n, i32 base, char *dst);
CY_DEF isize cy_str_parse_i64(i64 n, i32 base, char *dst);

CY_DEF f32 cy_str_to_f32(const char *str, isize *len_out);
CY_DEF f64 cy_str_to_f64(const char *str, isize *len_out);

CY_DEF isize cy_str_parse_f32(f32 n, char *dst);
CY_DEF isize cy_str_parse_f64(f64 n, char *dst);

/* ======================= Strings (and StringViews) ======================== */
typedef struct {
    CyAllocator alloc;
    isize len;
    isize cap;
} CyStringHeader;

#define CY_STRING_HEADER(str) ((CyStringHeader*)((uintptr)str) - 1)

struct CyStringView {
    const u8 *text;
    isize len;
};

CY_DEF isize cy_string_len(CyString str);
CY_DEF isize cy_string_cap(CyString str);
CY_DEF isize cy_string_alloc_size(CyString str);
CY_DEF isize cy_string_available_space(CyString str);

CY_DEF void cy__string_set_len(CyString str, isize len);
CY_DEF void cy__string_set_cap(CyString str, isize cap);

CY_DEF CyString cy_string_create_reserve(CyAllocator a, isize cap);
CY_DEF CyString cy_string_create_len(CyAllocator a, const char *str, isize len);
CY_DEF CyString cy_string_create(CyAllocator a, const char *str);
CY_DEF CyString cy_string_create_view(CyAllocator a, CyStringView str);
CY_DEF CyString cy_string_reserve_space_for(CyString str, isize extra_len);
CY_DEF CyString cy_string_shrink(CyString str);
CY_DEF void cy_string_free(CyString str);

CY_DEF CyString cy_string_append_len(
    CyString str, const char *other, isize len
);
CY_DEF CyString cy_string_append(CyString str, const CyString other);
CY_DEF CyString cy_string_append_c(CyString str, const char *other);
CY_DEF CyString cy_string_append_rune(CyString str, Rune r);
CY_DEF CyString cy_string_append_fmt(
    CyString str, const char *fmt, ...
) CY__FMT_ATTR(2);
CY_DEF CyString cy_string_append_view(CyString str, CyStringView view);
CY_DEF CyString cy_string_prepend_len(
    CyString str, const char *other, isize len
);
CY_DEF CyString cy_string_prepend(CyString str, const CyString other);
CY_DEF CyString cy_string_prepend_c(CyString str, const char *other);
CY_DEF CyString cy_string_prepend_rune(CyString str, Rune r);
CY_DEF CyString cy_string_prepend_fmt(
    CyString str, const char *fmt, ...
) CY__FMT_ATTR(2);
CY_DEF CyString cy_string_prepend_view(CyString str, CyStringView view);
CY_DEF CyString cy_string_pad_right(CyString str, isize width, Rune r);
CY_DEF CyString cy_string_set(CyString str, const char *c_str);
CY_DEF CyString cy_string_dup(CyAllocator a, const CyString src);
CY_DEF b32 cy_string_are_equal(const CyString a, const CyString b);

typedef enum {
    CY__STRING_TRIM_LEADING = CY_BIT(0),
    CY__STRING_TRIM_TRAILING = CY_BIT(1),
} CyStringTrimFlags;

CY_DEF CyString cy_string_trim(CyString str, const char *char_set);
CY_DEF CyString cy_string_trim_leading(CyString str, const char *char_set);
CY_DEF CyString cy_string_trim_trailing(CyString str, const char *char_set);
CY_DEF CyString cy__string_trim_internal(
    CyString str, const char *char_set, i32 flags
);
CY_DEF CyString cy_string_trim_whitespace(CyString str);
CY_DEF CyString cy_string_trim_leading_whitespace(CyString str);
CY_DEF CyString cy_string_trim_trailing_whitespace(CyString str);

CY_DEF CyStringView cy_string_view_create_len(const char *str, isize len);
CY_DEF CyStringView cy_string_view_create(CyString str);
CY_DEF CyStringView cy_string_view_create_c(const char *str);
// NOTE(cya): exclusive range
CY_DEF CyStringView cy_string_view_substring(
    CyStringView str, isize begin_idx, isize end_idx
);
CY_DEF b32 cy_string_view_are_equal(CyStringView a, CyStringView b);
CY_DEF b32 cy_string_view_has_prefix(CyStringView str, const char *prefix);
CY_DEF b32 cy_string_view_contains(CyStringView str, const char *char_set);

/* ================== Strings (and StringViews) (UTF-16) ==================== */
typedef wchar_t *CyString16;

typedef struct {
    const u16 *text;
    isize len;
} CyString16View;

CY_DEF isize cy_string_16_len(CyString16 str);
CY_DEF isize cy_string_16_cap(CyString16 str);
CY_DEF isize cy_string_16_alloc_size(CyString16 str);
CY_DEF isize cy_string_16_available_space(CyString16 str);

CY_DEF void cy__string_16_set_len(CyString16 str, isize len);
CY_DEF void cy__string_16_set_cap(CyString16 str, isize cap);

CY_DEF CyString16 cy_string_16_create_reserve(CyAllocator a, isize cap);
CY_DEF CyString16 cy_string_16_create_len(
    CyAllocator a, const wchar_t *str, isize len
);
CY_DEF CyString16 cy_string_16_create(CyAllocator a, const wchar_t *str);
CY_DEF CyString16 cy_string_16_create_view(CyAllocator a, CyString16View str);
CY_DEF CyString16 cy_string_16_reserve_space_for(
    CyString16 str, isize extra_len
);
CY_DEF CyString16 cy_string_16_shrink(CyString16 str);
CY_DEF CyString16 cy_string_16_resize(CyString16 str, isize new_cap);
CY_DEF void cy_string_16_free(CyString16 str);

CY_DEF void cy_string_16_clear(CyString16 str);
CY_DEF CyString16 cy_string_16_append_len(
    CyString16 str, const wchar_t *other, isize len
);
CY_DEF CyString16 cy_string_16_append(CyString16 str, const CyString16 other);
CY_DEF CyString16 cy_string_16_append_c(CyString16 str, const wchar_t *other);
CY_DEF CyString16 cy_string_16_append_view(CyString16 str, CyString16View view);

CY_DEF CyString16View cy_string_16_view_create_len(
    const wchar_t *str, isize len
);
CY_DEF CyString16View cy_string_16_view_create(CyString16 str);
CY_DEF CyString16View cy_string_16_view_create_c(const wchar_t *str);
// NOTE(cya): exclusive range
CY_DEF CyString16View cy_string_16_view_substring(
    CyString16View str, isize begin_idx, isize end_idx
);
CY_DEF b32 cy_string_16_view_are_equal(CyString16View a, CyString16View b);
CY_DEF b32 cy_string_16_view_has_prefix(
    CyString16View str, const wchar_t *prefix
);
CY_DEF b32 cy_string_16_view_contains(
    CyString16View str, const wchar_t *char_set
);

/* ============================ Unicode helpers ============================= */
CY_DEF isize cy_utf8_codepoints(const char *str);

#endif // CY__H_INCLUDE

#ifdef CY_IMPLEMENTATION
#undef CY_IMPLEMENTATION

/******************************************************************************
 *                               IMPLEMENTATION                               *
 ******************************************************************************/
#if defined(CY_COMPILER_MSVC)
    #pragma warning(push)
    #pragma warning(disable:4061) // enum cases
    #pragma warning(disable:4710) // not inlined
    #pragma warning(disable:5045) // spectre mitigation??
#elif defined(CY_COMPILER_GCC)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wimplicit-fallthrough" // gcc smh...
#endif

/* ================================= Runtime ================================ */
void cy_handle_assertion(
    const char *prefix, const char *cond, const char *file,
    i32 line, const char *msg, ...
) {
    cy_eprintf("%s(%d): %s: ", file, line, prefix);
    if (cond != NULL) {
        cy_eprintf( "`%s` ", cond);
    }
    if (msg != NULL) {
        va_list va;
        va_start(va, msg);
        cy_eprintf_va(msg, va);
        va_end(va);
    }

    cy_eprintf("\n");
}

// TODO(cya): remove header and implement mem functions
#ifndef CY_OS_WINDOWS
    #include <string.h>
#endif

cy_inline void *cy_mem_copy(
    void *restrict dst, const void *restrict src, isize bytes
) {
    return memcpy(dst, src, (usize)bytes);
}

cy_inline void *cy_mem_move(void *dst, const void *src, isize bytes)
{
    return memmove(dst, src, (usize)bytes);
}

cy_inline void *cy_mem_set(void *dst, u8 val, isize bytes)
{
    return memset(dst, val, (usize)bytes);
}

cy_inline void *cy_mem_zero(void *dst, isize bytes)
{
    return cy_mem_set(dst, 0, bytes);
}

cy_inline isize cy_mem_compare(const void *a, const void *b, isize bytes)
{
    return memcmp(a, b, (usize)bytes);
}

cy_inline b32 cy_is_power_of_two(isize n)
{
    return (n & (n - 1)) == 0;
}

cy_inline isize cy_align_forward_size(isize size, isize align)
{
    CY_ASSERT(align > 0 && cy_is_power_of_two(align));

    isize mod = size & (align - 1);
    return mod != 0 ? size + align - mod : size;
}

cy_inline uintptr cy_align_forward(uintptr ptr, isize align)
{
    CY_ASSERT(align > 0 && cy_is_power_of_two(align));

    uintptr _align = (uintptr)align;
    uintptr mod = ptr & (_align - 1);
    return mod != 0 ? ptr + _align - mod : ptr;
}

cy_inline void *cy_align_forward_ptr(void *ptr, isize align)
{
    return (void*)cy_align_forward((uintptr)ptr, align);
}

cy_inline isize cy_calc_header_padding(uintptr ptr, isize align, isize header_size)
{
    CY_ASSERT(cy_is_power_of_two(align));

    uintptr a = (uintptr)align;
    uintptr mod = ptr & (a - 1);
    uintptr padding = mod ? a - mod : 0;
    if (padding < (uintptr)header_size) {
        uintptr needed_space = (uintptr)header_size - padding;
        padding += (needed_space & (a - 1)) ?
            a * (needed_space / a + 1) : a * (needed_space / a);
    }

    return (isize)padding;
}

#ifndef CY_OS_WINDOWS
    #include <time.h>
#endif

struct CyTicks {
#if defined(CY_OS_WINDOWS)
    LARGE_INTEGER counter;
#else
    struct timespec counter;
#endif
};

CyTicks cy_ticks_query(void)
{
    CyTicks ticks;
#if defined(CY_OS_WINDOWS)
    QueryPerformanceCounter(&ticks.counter);
#else
    clock_gettime(CLOCK_MONOTONIC_RAW, &ticks.counter);
#endif

    return ticks;
}

cy_inline CyTicks cy_ticks_elapsed(CyTicks start, CyTicks end)
{
    return (CyTicks){
#if defined(CY_OS_WINDOWS)
        .counter.QuadPart = end.counter.QuadPart - start.counter.QuadPart,
#else
        .counter.tv_sec = end.counter.tv_sec - start.counter.tv_sec,
        .counter.tv_nsec = end.counter.tv_nsec - start.counter.tv_nsec,
#endif
    };
}

cy_inline f64 cy_ticks_to_time_unit(CyTicks ticks, CyTimeUnit unit)
{
#if defined(CY_OS_WINDOWS)
    cy_persist LARGE_INTEGER perf_freq;
    if (perf_freq.QuadPart == 0) {
        QueryPerformanceFrequency(&perf_freq);
    }

    f64 unit_f = (f64)unit;

    return (f64)ticks.counter.QuadPart * unit_f / (f64)perf_freq.QuadPart;
#else
    return (f64)ticks.counter.tv_sec * (f64)unit +
        (f64)ticks.counter.tv_nsec / (1.0e9 / unit_f);
#endif
}

/* ================================== Files ================================= */
// TODO(cya): maybe don't do this (?)
cy_global CyFile cy__std_files[CY__FILE_STD_COUNT];
cy_global b32 cy__std_files_set;

#if defined(CY_OS_WINDOWS)
cy_internal CyFileReadAtProc cy__win32_file_read;
cy_internal CyFileWriteAtProc cy__win32_file_write;
cy_internal CyFileSeekProc cy__win32_file_seek;
cy_internal CyFileCloseProc cy__win32_file_close;

const CyFileOps cy__default_file_ops = {
    cy__win32_file_read,
    cy__win32_file_write,
    cy__win32_file_seek,
    cy__win32_file_close,
};

cy_internal CyString16 cy__win32_utf8_to_utf16(CyAllocator a, const char *str)
{
    CY_VALIDATE_PTR(str);

    isize len = cy_str_len(str);
    isize size_utf16 = MultiByteToWideChar(
        CP_UTF8, 0,
        str, (int)(len + 1),
        NULL, 0
    );
    isize len_utf16 = size_utf16 - 1;
    CyString16 str_utf16 = cy_string_16_create_reserve(a, len_utf16);
    CY_VALIDATE_PTR(str_utf16);

    isize res = MultiByteToWideChar(
        CP_UTF8, 0,
        str, (int)(len + 1),
        str_utf16, (int)(size_utf16)
    );
    if (res == 0) {
        cy_string_16_free(str_utf16);
        return NULL;
    }

    cy__string_16_set_len(str_utf16, len_utf16);
    return str_utf16;
}

cy_internal CY_FILE_OPEN_PROC(cy__win32_file_open)
{
    if (filename == NULL) {
        return CY_FILE_ERROR_OUT_OF_MEMORY;
    }

    DWORD desired_access, creation_disposition;
    switch (mode & CY__FILE_MODE_MODES) {
    case CY_FILE_MODE_READ: {
        desired_access = GENERIC_READ;
        creation_disposition = OPEN_EXISTING;
    } break;
    case CY_FILE_MODE_WRITE: {
        desired_access = GENERIC_WRITE;
        creation_disposition = CREATE_ALWAYS;
    } break;
    case CY_FILE_MODE_APPEND: {
        desired_access = GENERIC_WRITE;
        creation_disposition = OPEN_ALWAYS;
    } break;
    case CY_FILE_MODE_READ | CY_FILE_MODE_READ_WRITE: {
        desired_access = GENERIC_READ | GENERIC_WRITE;
        creation_disposition = OPEN_EXISTING;
    } break;
    case CY_FILE_MODE_WRITE | CY_FILE_MODE_READ_WRITE: {
        desired_access = GENERIC_READ | GENERIC_WRITE;
        creation_disposition = CREATE_ALWAYS;
    } break;
    case CY_FILE_MODE_APPEND | CY_FILE_MODE_READ_WRITE: {
        desired_access = GENERIC_READ | GENERIC_WRITE;
        creation_disposition = OPEN_ALWAYS;
    } break;
    default: {
        // TODO(cya): panic
        return CY_FILE_ERROR_INVALID;
    } break;
    }

    CyAllocator a = cy_heap_allocator();
    CyString16 filename_16 = cy__win32_utf8_to_utf16(a, filename);
    if (filename_16 == NULL) {
        return CY_FILE_ERROR_OUT_OF_MEMORY;
    }

    DWORD file_share_mode = FILE_SHARE_READ | FILE_SHARE_DELETE;
    HANDLE handle = CreateFileW(
        filename_16,desired_access, file_share_mode, NULL,
        creation_disposition, FILE_ATTRIBUTE_NORMAL, NULL
    );

    cy_string_16_free(filename_16);
    if (handle == INVALID_HANDLE_VALUE) {
        switch (GetLastError()) {
        case ERROR_FILE_NOT_FOUND: {
            return CY_FILE_ERROR_NOT_FOUND;
        } break;
        case ERROR_FILE_EXISTS:
        case ERROR_ALREADY_EXISTS: {
            return CY_FILE_ERROR_EXISTS;
        } break;
        case ERROR_ACCESS_DENIED: {
            return CY_FILE_ERROR_ACCESS_DENIED;
        } break;
        default: {
            return CY_FILE_ERROR_INVALID;
        } break;
        }
    }

    if (mode & CY_FILE_MODE_APPEND) {
        LARGE_INTEGER ofs = {0};
        if (!SetFilePointerEx(handle, ofs, NULL, FILE_END)) {
            CloseHandle(handle);
            return CY_FILE_ERROR_INVALID;
        }
    }

    fd->p = handle;
    *ops = cy__default_file_ops;

    return CY_FILE_ERROR_NONE;
}

cy_internal CY_FILE_READ_AT_PROC(cy__win32_file_read)
{
    DWORD truncated_size = (DWORD)CY_MIN(size, U32_MAX);
    DWORD _bytes_read;
    cy__win32_file_seek(fd, offset, CY_SEEK_WHENCE_BEGIN, NULL);
    if (ReadFile(fd.p, buf, truncated_size, &_bytes_read, NULL)) {
        if (bytes_read != NULL) {
            *bytes_read = _bytes_read;
        }

        return true;
    }

    return false;
}

cy_internal CY_FILE_WRITE_AT_PROC(cy__win32_file_write)
{
    DWORD truncated_size = (DWORD)CY_MIN(size, U32_MAX);
    DWORD _bytes_written;
    cy__win32_file_seek(fd, offset, CY_SEEK_WHENCE_BEGIN, NULL);
    if (WriteFile(fd.p, buf, truncated_size, &_bytes_written, NULL)) {
        if (bytes_written != NULL) {
            *bytes_written = _bytes_written;
        }

        return true;
    }

    return false;
}

cy_internal CY_FILE_SEEK_PROC(cy__win32_file_seek)
{
    LARGE_INTEGER li_offset = {.QuadPart = offset};
    if (!SetFilePointerEx(fd.p, li_offset, &li_offset, (DWORD)whence)) {
        return false;
    }

    if (new_offset != NULL) {
        *new_offset = li_offset.QuadPart;
    }

    return true;
}

cy_internal CY_FILE_CLOSE_PROC(cy__win32_file_close)
{
    CloseHandle(fd.p);
}

cy_inline const char *cy_file_error_as_str(CyFileError err)
{
    const char *str = "";
    switch (err) {
    case CY_FILE_ERROR_OUT_OF_MEMORY: {
        str = "out of memory";
    } break;
    case CY_FILE_ERROR_NONE: {
        str = "success";
    } break;
    case CY_FILE_ERROR_INVALID: {
        str = "invalid handle";
    } break;
    case CY_FILE_ERROR_INVALID_FILENAME: {
        str = "invalid filename";
    } break;
    case CY_FILE_ERROR_EXISTS: {
        str = "already exists";
    } break;
    case CY_FILE_ERROR_NOT_FOUND: {
        str = "not found";
    } break;
    case CY_FILE_ERROR_ACCESS_DENIED: {
        str = "access denied";
    } break;
    case CY_FILE_ERROR_TRUNCATION_FAILED: {
        str = "truncation failed";
    } break;
    }

    return str;
}

CyFile *cy_file_get_std_handle(CyFileStdType type)
{
    if (!cy__std_files_set) {
#define CY__SET_STD_FILE(type, val) { \
    cy__std_files[type].fd.p = val; \
    cy__std_files[type].ops = cy__default_file_ops; \
} CY_NOOP()
        CY__SET_STD_FILE(CY_FILE_STD_IN, GetStdHandle(STD_INPUT_HANDLE));
        CY__SET_STD_FILE(CY_FILE_STD_OUT, GetStdHandle(STD_OUTPUT_HANDLE));
        CY__SET_STD_FILE(CY_FILE_STD_ERR, GetStdHandle(STD_ERROR_HANDLE));
#undef CY__SET_STD_FILE

        cy__std_files_set = true;
    }

    return &cy__std_files[type];
}

#else // POSIX files
//  TODO(cya): implement
#endif

cy_inline CyFileError cy_file_create(CyFile *f, const char *filename)
{
    return cy_file_open_with_mode(
        f, CY_FILE_MODE_WRITE | CY_FILE_MODE_READ_WRITE, filename
    );
}

cy_inline CyFileError cy_file_open(CyFile *f, const char *filename)
{
    return cy_file_open_with_mode(f, CY_FILE_MODE_READ, filename);
}

CyFileError cy_file_open_with_mode(
    CyFile *f, i32 mode, const char *filename
) {
    CyFileError err =
#if defined(CY_OS_WINDOWS)
        cy__win32_file_open(&f->fd, &f->ops, mode, filename);
#else
        cy__posix_file_open(&f->fd, &f->ops, mode, filename);
#endif

    return err != CY_FILE_ERROR_NONE ?
        err : cy_file_new(f, f->fd, f->ops, filename);
}

cy_inline CyFileError cy_file_new(
    CyFile *f, CyFileDescriptor fd, CyFileOps ops, const char *filename
) {
    isize len = cy_str_len(filename);
    // TODO(cya): maybe filename should be a CyString
    *f = (CyFile){
        .ops = ops,
        .fd = fd,
        .filename = cy_alloc_copy(cy_heap_allocator(), filename, len + 1),
        .last_write_time = cy_file_path_last_write_time(filename),
    };
    if (f->filename == NULL) {
        return CY_FILE_ERROR_OUT_OF_MEMORY;
    }

    return CY_FILE_ERROR_NONE;
}

cy_inline CyFileError cy_file_close(CyFile *f)
{
    if (f == NULL) {
        return CY_FILE_ERROR_INVALID;
    } else if (f->filename != NULL) {
        cy_free(cy_heap_allocator(), (void*)f->filename);
    }

    b32 invalid =
#if defined(CY_OS_WINDOWS)
        (f->fd.p == INVALID_HANDLE_VALUE);
#else
        (f->fd.i < 0);
#endif
    if (invalid) {
        return CY_FILE_ERROR_INVALID;
    }

    if (f->ops.read_at == NULL) {
        f->ops = cy__default_file_ops;
    }

    f->ops.close(f->fd);
    return CY_FILE_ERROR_NONE;
}

#if defined(CY_OS_WINDOWS)
cy_inline CyFileError cy_file_truncate(CyFile *f, isize size)
{
    CyFileError err = CY_FILE_ERROR_NONE;

    isize prev_offset = cy_file_tell(f);
    cy_file_seek(f, size);
    if (!SetEndOfFile(f->fd.p)) {
        err = CY_FILE_ERROR_TRUNCATION_FAILED;
    }

    cy_file_seek(f, prev_offset);
    return err;
}
#else

#endif

cy_inline b32 cy_file_read_at_report(
    CyFile *f, void *buf,
    isize size, isize offset, isize *bytes_read
) {
    if (f->ops.read_at == NULL) {
        f->ops = cy__default_file_ops;
    }

    return f->ops.read_at(f->fd, buf, size, offset, bytes_read);
}

cy_inline b32 cy_file_write_at_report(
    CyFile *f, const void *buf,
    isize size, isize offset, isize *bytes_written
) {
    if (f->ops.write_at == NULL) {
        f->ops = cy__default_file_ops;
    }

    return f->ops.write_at(f->fd, buf, size, offset, bytes_written);
}

cy_inline b32 cy_file_read_at(CyFile *f, void *buf, isize size, isize offset)
{
    return cy_file_read_at_report(f, buf, size, offset, NULL);
}

cy_inline b32 cy_file_write_at(
    CyFile *f, const void *buf, isize size, isize offset
) {
    return cy_file_write_at_report(f, buf, size, offset, NULL);
}

cy_inline b32 cy_file_read(CyFile *f, void *buf, isize size)
{
    return cy_file_read_at(f, buf, size, cy_file_tell(f));
}

cy_inline b32 cy_file_write(CyFile *f, const void *buf, isize size)
{
    return cy_file_write_at(f, buf, size, cy_file_tell(f));
}

cy_inline isize cy_file_seek(CyFile *f, isize offset)
{
    isize new_offset = 0;
    if (f->ops.seek == NULL) {
        f->ops = cy__default_file_ops;
    }

    f->ops.seek(f->fd, offset, CY_SEEK_WHENCE_BEGIN, &new_offset);
    return new_offset;
}

cy_inline isize cy_file_seek_to_end(CyFile *f)
{
    isize new_offset = 0;
    if (f->ops.seek == NULL) {
        f->ops = cy__default_file_ops;
    }

    f->ops.seek(f->fd, 0, CY_SEEK_WHENCE_END, &new_offset);
    return new_offset;
}

cy_inline isize cy_file_skip(CyFile *f, isize bytes)
{
    isize new_offset = 0;
    if (f->ops.seek == NULL) {
        f->ops = cy__default_file_ops;
    }

    f->ops.seek(f->fd, bytes, CY_SEEK_WHENCE_CURRENT, &new_offset);
    return new_offset;
}

cy_inline isize cy_file_tell(CyFile *f)
{
    isize new_offset = 0;
    if (f->ops.seek == NULL) {
        f->ops = cy__default_file_ops;
    }

    f->ops.seek(f->fd, 0, CY_SEEK_WHENCE_CURRENT, &new_offset);
    return new_offset;
}

#if defined(CY_OS_WINDOWS)
cy_inline isize cy_file_size(CyFile *f)
{
    LARGE_INTEGER size;
    GetFileSizeEx(f->fd.p, &size);
    return (isize)size.QuadPart;
}

cy_inline const char *cy_file_name(CyFile *f)
{
    return f->filename == NULL ? "" : f->filename;
}

cy_inline b32 cy_file_has_changed(CyFile *f)
{
    CyFileTime last_write_time = cy_file_path_last_write_time(f->filename);
    b32 changed = (f->last_write_time != last_write_time);
    if (changed) {
        f->last_write_time = last_write_time;
    }

    return changed;
}

b32 cy_file_path_exists(const char *filename)
{
    WIN32_FIND_DATAW data;
    CyAllocator a = cy_heap_allocator();
    CyString16 filename_16 = cy__win32_utf8_to_utf16(a, filename);
    if (filename_16 == NULL) {
        return false;
    }

    HANDLE handle = FindFirstFileW(filename_16, &data);
    cy_string_16_free(filename_16);

    b32 found = (handle != INVALID_HANDLE_VALUE);
    if (found) {
        FindClose(handle);
    }

    return found;
}

CyFileTime cy_file_path_last_write_time(const char *filename)
{
    ULARGE_INTEGER li = {0};
    FILETIME last_write_time = {0};
    WIN32_FILE_ATTRIBUTE_DATA attr = {0};

    CyAllocator a = cy_heap_allocator();
    CyString16 filename_16 = cy__win32_utf8_to_utf16(a, filename);
    if (filename_16 == NULL) {
        return 0;
    }

    if (GetFileAttributesExW(filename_16, GetFileExInfoStandard, &attr)) {
        last_write_time = attr.ftLastWriteTime;
    }

    cy_string_16_free(filename_16);

    li.LowPart = last_write_time.dwLowDateTime;
    li.HighPart = last_write_time.dwHighDateTime;
    return (CyFileTime)li.QuadPart;
}

b32 cy_file_path_copy(
    const char *cur_filename, const char *new_filename, b32 fail_if_exists
) {
    b32 res = false;
    CyAllocator a = cy_heap_allocator();

    CyString16 cur_filename_16 = cy__win32_utf8_to_utf16(a, cur_filename);
    if (cur_filename_16 == NULL) {
        return false;
    }

    CyString16 new_filename_16 = cy__win32_utf8_to_utf16(a, new_filename);
    if (new_filename_16 != NULL) {
        res = CopyFileW(cur_filename_16, new_filename_16, fail_if_exists);
    }

    cy_string_16_free(new_filename_16);
    cy_string_16_free(cur_filename_16);
    return res;
}

b32 cy_file_path_move(const char *cur_filename, const char *new_filename)
{
    b32 res = false;
    CyAllocator a = cy_heap_allocator();

    CyString16 cur_filename_16 = cy__win32_utf8_to_utf16(a, cur_filename);
    if (cur_filename_16 == NULL) {
        return false;
    }

    CyString16 new_filename_16 = cy__win32_utf8_to_utf16(a, new_filename);
    if (new_filename_16 != NULL) {
        res = MoveFileW(cur_filename_16, new_filename_16);
    }

    cy_string_16_free(new_filename_16);
    cy_string_16_free(cur_filename_16);
    return res;
}

b32 cy_file_path_remove(const char *filename)
{
    CyAllocator a = cy_heap_allocator();
    CyString16 filename_16 = cy__win32_utf8_to_utf16(a, filename);
    if (filename_16 == NULL) {
        return false;
    }

    b32 res = DeleteFileW(filename_16);
    cy_string_16_free(filename_16);
    return res;
}

#else // POSIX

#endif

/* =================================== I/O ================================== */
cy_inline isize cy_printf(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    isize len = cy_printf_va(fmt, va);
    va_end(va);

    return len;
}

cy_inline isize cy_printf_va(const char *fmt, va_list va)
{
    return cy_fprintf_va(cy_file_get_std_handle(CY_FILE_STD_OUT), fmt, va);
}

cy_inline isize cy_eprintf(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    isize len = cy_eprintf_va(fmt, va);
    va_end(va);

    return len;
}

cy_inline isize cy_eprintf_va(const char *fmt, va_list va)
{
    return cy_fprintf_va(cy_file_get_std_handle(CY_FILE_STD_ERR), fmt, va);
}

cy_inline isize cy_fprintf(CyFile *f, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    isize len = cy_fprintf_va(f, fmt, va);
    va_end(va);

    return len;
}

#define CY__FMT_STATIC_BUF_SIZE 4096

cy_inline isize cy_fprintf_va(CyFile *f, const char *fmt, va_list va)
{
    va_list va_dup;
    va_copy(va_dup, va);

    char static_buf[CY__FMT_STATIC_BUF_SIZE];
    isize static_size = CY_ARRAY_LEN(static_buf);
    isize len = cy_sprintf_va(static_buf, static_size, fmt, va);

    char *src = static_buf;
    b32 use_heap = (len >= static_size);
    if (use_heap) {
        isize heap_size = len + 1;
        char *heap_buf = cy_alloc_array(cy_heap_allocator(), char, heap_size);
        if (heap_buf != NULL) {
            len = cy_sprintf_va(heap_buf, heap_size, fmt, va_dup);
            CY_ASSERT(len < heap_size);

            src = heap_buf;
        } else {
            use_heap = false;
        }
    }

    cy_file_write(f, src, len);
    if (use_heap) {
        cy_free(cy_heap_allocator(), src);
    }

    return len;
}

cy_inline char *cy_aprintf(CyAllocator a, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    char *res = cy_aprintf_va(a, fmt, va);
    va_end(va);

    return res;
}

cy_inline char *cy_aprintf_va(CyAllocator a, const char *fmt, va_list va)
{
    va_list va_dup;
     va_copy(va_dup, va);

    isize init_size = 4096;
    char *buf = cy_alloc(a, init_size);
    isize len = cy_sprintf_va(buf, init_size, fmt, va);

    isize new_size = len + 1;
    char *new_buf = cy_resize(a, buf, init_size, new_size);
    if (new_buf == NULL) {
        cy_free(a, buf);
        return NULL;
    }

    buf = new_buf;
    if (new_size > init_size) {
        cy_sprintf_va(buf, new_size, fmt, va_dup);
    }

    return buf;
}

cy_inline isize cy_sprintf(char *buf, isize size, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    isize len = cy_sprintf_va(buf, size, fmt, va);
    va_end(va);

    return len;
}

enum {
    // NOTE(cya): flags
    CY__FMT_MINUS = CY_BIT(0),
    CY__FMT_PLUS = CY_BIT(1),
    CY__FMT_SPACE = CY_BIT(2),
    CY__FMT_HASH = CY_BIT(3),
    CY__FMT_ZERO = CY_BIT(4),

    // NOTE(cya): length modifiers
    CY__FMT_LEN_CHAR = CY_BIT(5),
    CY__FMT_LEN_SHORT = CY_BIT(6),
    CY__FMT_LEN_LONG = CY_BIT(7),
    CY__FMT_LEN_LONG_LONG = CY_BIT(8),
    CY__FMT_LEN_INTMAX = CY_BIT(9),
    CY__FMT_LEN_SIZE = CY_BIT(10),
    CY__FMT_LEN_PTRDIFF = CY_BIT(11),
    CY__FMT_LEN_LONG_DOUBLE = CY_BIT(12),

    CY__FMT_LEN_MODS =
        CY__FMT_LEN_CHAR | CY__FMT_LEN_SHORT | CY__FMT_LEN_LONG |
        CY__FMT_LEN_LONG_LONG | CY__FMT_LEN_INTMAX | CY__FMT_LEN_SIZE |
        CY__FMT_LEN_PTRDIFF | CY__FMT_LEN_LONG_DOUBLE,

    // NOTE(cya): conversion specifiers
    CY__FMT_INT = CY_BIT(13),
    CY__FMT_UNSIGNED = CY_BIT(14),
    CY__FMT_FLOAT = CY_BIT(15),

    CY__FMT_INTS = CY__FMT_UNSIGNED | CY__FMT_INT,

    // NOTE(cya): conversion style modifiers
    CY__FMT_STYLE_EXP = CY_BIT(20), // e, E
    CY__FMT_STYLE_AUTO = CY_BIT(21), // g, G
    CY__FMT_STYLE_UPPER = CY_BIT(22),
};

typedef struct {
    i32 base;
    i32 flags;
    i32 width;
    i32 precision;
    union {
        u64 u;
        i64 i;
        f64 f;
        CyStringView s;
    } value;
} CyPrivFmtInfo;

cy_internal isize cy__scan_u64(const char *str, i32 base, u64 *value)
{
    const char *s = str;
    u64 res = 0;
    if (base == 16 && cy_str_has_prefix(s, "0x")) {
        s += 2;
    }

    for (;;) {
        i64 d;
        char c = *s;
        if (cy_char_is_digit(c)) {
            d = c - '0';
        } else if (base == 16 && cy_char_is_hex_digit(c)) {
            d = cy_hex_digit_to_i64(c);
        } else {
            break;
        }

        res = res * (u64)base + (u64)d;
        s += 1;
    }

    if (value != NULL) {
        *value = res;
    }

    return s - str;
}

cy_internal isize cy__scan_i64(const char *str, i32 base, i64 *value)
{
    const char *s = str;
    i64 res = 0;
    b32 negative = false;
    if (*s == '-') {
        negative = true;
        s += 1;
    }

    if (base == 16 && cy_str_has_prefix(s, "0x")) {
        s += 2;
    }

    for (;;) {
        i64 d;
        if (cy_char_is_digit(*s)) {
            d = *s - '0';
        } else if (base == 16 && cy_char_is_hex_digit(*s)) {
            d = cy_hex_digit_to_i64(*s);
        } else {
            break;
        }

        res = res * base + d;
        s += 1;
    }

    if (negative) {
        res = -res;
    }

    if (value != NULL) {
        *value = res;
    }

    return s - str;
}

cy_global const char cy__num_to_char_table_upper[] = "0123456789ABCDEF";
cy_global const char cy__num_to_char_table_lower[] = "0123456789abcdef";

cy_internal cy_inline char cy__fmt_get_base_char(i32 base)
{
    switch (base) {
    case 2: {
        return 'b';
    } break;
    case 16: {
        return 'x';
    } break;
    }

    return '?';
}

cy_internal cy_inline char cy__fmt_char_to_case(char c, b32 uppercase)
{
    return uppercase ? cy_char_to_upper(c) : c;
}

#define CY__PRINT_PROC(name) \
    isize name(char *dst, isize cap, CyPrivFmtInfo info)
typedef CY__PRINT_PROC(CyPrivPrintProc);

cy_internal CY__PRINT_PROC(cy__print_u64)
{
    u64 n = info.value.u;

    const isize limit = cap - 1;
    isize written = 0;
    isize written_total = 0;

    i32 base = info.base;
    b32 style_alt = (info.flags & CY__FMT_HASH);
    b32 add_prefix = style_alt &&
        (base == 2 || base == 8 || base == 16) && n != 0;
    b32 style_upper = (info.flags & CY__FMT_STYLE_UPPER);
    const char *table = style_upper ?
        cy__num_to_char_table_upper : cy__num_to_char_table_lower;

    char *c = dst;
    if (n == 0) {
        if (info.precision == 0) {
            return 0; // NOTE(cya): edge case from the spec
        } if (written < limit) {
            *c++ = '0';
            written += 1;
        }

        written_total += 1;
    } else {
        while (n > 0) {
            if (written < limit) {
                *c++ = table[n % (u64)base];
                written += 1;
            }

            n /= (u64)base;
            written_total += 1;
        }
    }

    b32 use_precision = (info.precision != -1) &&
        (info.flags & CY__FMT_INTS);
    if (use_precision) {
        while (written < info.precision) {
            if (written < limit) {
                *c++ = '0';
                written += 1;
            }

            written_total += 1;
        }
    }

    b32 fill_with_zeros = !use_precision && (written < info.width) &&
        (info.flags & CY__FMT_ZERO) && !(info.flags & CY__FMT_MINUS);
    b32 print_sign = (info.flags & CY__FMT_PLUS) ||
        (info.flags & CY__FMT_SPACE);
    isize sign_len = !!print_sign;
    if (fill_with_zeros) {
        isize extra = info.width - written - sign_len;
        while (extra-- > 0) {
            if (written < limit) {
                *c++ = '0';
                written += 1;
            }

            written_total += 1;
        }
    }

    if (add_prefix) {
        switch (base) {
        case 2:
        case 16: {
            if (written < limit) {
                char p = cy__fmt_get_base_char(base);
                *c++ = cy__fmt_char_to_case(p, style_upper);
                written += 1;
            }

            written_total += 1;
        } // NOTE(cya): fallthrough
        case 8: {
            if (written < limit) {
                *c++ = '0';
                written += 1;
            }

            written_total += 1;
        } break;
        default: break;
        }
    }

    if (print_sign) {
        char sign = (info.flags & CY__FMT_SPACE) ? ' ' : '+';
        if (written < limit) {
            *c++ = sign;
            written += 1;
        }

        written_total += 1;
    }

    isize len = written;
    cy_str_reverse_n(dst, len);

    return written_total;
}

cy_internal CY__PRINT_PROC(cy__print_i64)
{
    i64 n = info.value.i;

    const isize limit = cap - 1;
    isize written = 0;
    isize written_total = 0;

    i32 base = info.base;
    b32 style_upper = (info.flags & CY__FMT_STYLE_UPPER);
    const char *table = style_upper ?
        cy__num_to_char_table_upper : cy__num_to_char_table_lower;

    b32 negative = false;
    if (n < 0) {
        negative = true;
        n = -n;
    }

    char *c = dst;
    u64 v = (u64)n;
    if (v == 0) {
        if (info.precision == 0) {
            return 0; // NOTE(cya): edge case from the spec
        } else if (written < limit) {
            *c++ = '0';
            written += 1;
        }

        written_total += 1;
    } else {
        while (v > 0) {
            if (written < limit) {
                *c++ = table[v % (u64)base];
                written += 1;
            }

            v /= (u64)base;
            written_total += 1;
        }
    }

    b32 use_precision = (info.precision != -1) &&
        (info.flags & CY__FMT_INTS);
    if (use_precision) {
        isize cur_len = c - dst;
        while (cur_len < info.precision) {
            if (written < limit) {
                *c++ = '0';
                written += 1;
            }

            written_total += 1;
        }
    }

    b32 fill_with_zeros = !use_precision && (written < info.width) &&
        (info.flags & CY__FMT_ZERO) && !(info.flags & CY__FMT_MINUS);
    b32 print_sign = negative ||
        (info.flags & CY__FMT_PLUS) || (info.flags & CY__FMT_SPACE);
    isize sign_len = !!print_sign;
    if (fill_with_zeros) {
        isize extra = info.width - written - sign_len;
        while (extra-- > 0) {
            if (written < limit) {
                *c++ = '0';
                written += 1;
            }

            written_total += 1;
        }
    }

    if (print_sign) {
        char sign = '-';
        if (info.flags & CY__FMT_PLUS) {
            sign = '+';
        } else if (info.flags & CY__FMT_SPACE) {
            sign = ' ';
        }

        if (written < limit) {
            *c++ = sign;
            written += 1;
        }

        written_total += 1;
    }

    isize len = written;
    cy_str_reverse_n(dst, len);

    return written_total;
}

cy_internal CY__PRINT_PROC(cy__print_str)
{
    const char *src = (const char*)info.value.s.text;
    isize len = info.value.s.len;

    const isize limit = cap - 1;
    isize written = 0;
    isize written_total = 0;

    i32 precision = info.precision;
    if (precision > 0) {
        while (len-- > 0 && precision-- > 0) {
            if (written < limit) {
                *dst++ = *src++;
                written += 1;
            }

            written_total += 1;
        }
    } else {
        while (len-- > 0) {
            if (written < limit) {
                *dst++ = *src++;
                written += 1;
            }

            written_total += 1;
        }
    }

    return written_total;
}

// NOTE(cya): binary exponentiation for floats
#define CY__CALC_BIN_EXPS(_kind, base, prefix, val, exp) \
    CY__BIN_EXP_##_kind(base, prefix, val, exp, 256); \
    CY__BIN_EXP_##_kind(base, prefix, val, exp, 128); \
    CY__BIN_EXP_##_kind(base, prefix, val, exp, 64); \
    CY__BIN_EXP_##_kind(base, prefix, val, exp, 32); \
    CY__BIN_EXP_##_kind(base, prefix, val, exp, 16); \
    CY__BIN_EXP_##_kind(base, prefix, val, exp, 8); \
    CY__BIN_EXP_##_kind(base, prefix, val, exp, 4); \
    CY__BIN_EXP_##_kind(base, prefix, val, exp, 2); \
    CY__BIN_EXP_##_kind(base, prefix, val, exp, 1) \

#define CY__BIN_EXP_INC(base, prefix, val, exp, b) { \
    if (val >= prefix##b) { \
        val /= prefix##b; \
        exp += b; \
    } \
} CY_NOOP()
#define CY__BIN_EXP_DEC(base, prefix, val, exp, b) { \
    if (val < (prefix##-##b * base)) { \
        val *= prefix##b; \
        exp -= b; \
    } \
} CY_NOOP()

cy_global const f64 cy__pow_of_10_table[] = {
    1e00, 1e01, 1e02, 1e03, 1e04,
    1e05, 1e06, 1e07, 1e08, 1e09,
    1e10, 1e11, 1e12, 1e13, 1e14,
    1e15, 1e16, 1e16, 1e16, 1e16,
};

cy_global const f64 cy__pow_of_16_table[] = {
    0x1p00, 0x1p04, 0x1p08, 0x1p12, 0x1p16,
    0x1p20, 0x1p24, 0x1p28, 0x1p32, 0x1p36,
    0x1p40, 0x1p44, 0x1p48, 0x1p52, 0x1p56,
    0x1p60, 0x1p60, 0x1p60, 0x1p60, 0x1p60,
};

// TODO(cya):
// * correct rounding errors on binary exponentiation (probably requires MPA)
cy_internal CY__PRINT_PROC(cy__print_f64)
{
    f64 n = info.value.f;

    const isize limit = cap - 1;
    isize written = 0;
    isize written_total = 0;

    b32 upper = info.flags & CY__FMT_STYLE_UPPER;
    char *cur = dst;
    if (CY_IS_NAN(n)) {
        info.value.s = cy_string_view_create_c(upper ? "NAN" : "nan");
        return cy__print_str(cur, cap, info);
    }

    if (n < 0.0) {
        n = -n;
        if (written < limit) {
            *cur++ = '-';
            written += 1;
        }

        written_total += 1;
    } else if (info.flags & CY__FMT_PLUS) {
        if (written < limit) {
            *cur++ = '+';
            written += 1;
        }

        written_total += 1;
    } else if (info.flags & CY__FMT_SPACE) {
        if (written < limit) {
            *cur++ = ' ';
            written += 1;
        }

        written_total += 1;
    }

    if (CY_IS_INF(n)) {
        info.value.s = cy_string_view_create_c(upper ? "INF" : "inf");
        isize len = cy__print_str(cur, cap, info);
        return written_total + len;
    }

    b32 style_hex = info.base == 16;
    b32 style_exp = info.flags & CY__FMT_STYLE_EXP;
    b32 style_auto = info.flags & CY__FMT_STYLE_AUTO;
    b32 style_alt = info.flags & CY__FMT_HASH;

    if (style_hex) {
        info.value.s = cy_string_view_create_c(upper ? "0X" : "0x");
        isize len_total = cy__print_str(cur, cap, info);
        isize len = CY_MIN(len_total, cap);

        cur += len, written += len;
        written_total += len_total;
    }

    char *num_start = cur;

    const i32 MAX_PRECISION = 16;
    i32 precision = info.precision;
    b32 custom_precision = (precision != -1);
    if (custom_precision) {
        precision = CY_MIN(precision, MAX_PRECISION);
    } else if (!style_hex) {
        precision = 6;
    }

    i16 exponent = 0;
    f64 normalized = n;
    if (style_exp || style_auto) {
        if (normalized >= 1e1) {
            CY__CALC_BIN_EXPS(INC, 10, 1e, normalized, exponent);
        } else if (normalized > 0.0 && normalized <= 1e0) {
            CY__CALC_BIN_EXPS(DEC, 10, 1e, normalized, exponent);
            if (normalized < 1e0) {
                normalized *= 1e1;
                exponent -= 1;
            }
        }
    } else if (style_hex) {
        if (normalized >= 0x1p1) {
            CY__CALC_BIN_EXPS(INC, 2, 0x1p, normalized, exponent);
        } else if (normalized > 0.0 && normalized <= 0x1p0) {
            CY__CALC_BIN_EXPS(DEC, 2, 0x1p, normalized, exponent);
            if (normalized < 0x1p0) {
                normalized *= 0x1p1;
                exponent -= 1;
            }
        }
    }

    b32 swap = style_hex || style_exp ||
        (style_auto && (exponent < -4 || exponent >= precision));
    if (swap) {
        n = normalized;
    }

    u64 integral = (u64)n;
    isize integral_len, len;
    isize remaining = limit - written;
    isize len_total = integral_len = cy__print_u64(
        cur, remaining, (CyPrivFmtInfo){
            .base = info.base,
            .precision = -1,
            .value.u = integral,
        }
    );

    len = integral_len = CY_MIN(len_total, remaining);
    cur += len, written += len;
    written_total += len_total;

    // NOTE(cya): because 'significant digits' includes integral part
    if (style_auto) {
        precision -= (i32)integral_len;
    }

    f64 remainder = (n - (f64)integral);
    if (style_hex && precision == -1) {
        precision = 0;
        while (remainder - (f64)(u64)remainder != 0.0) {
            remainder *= (f64)info.base;
            precision += 1;
        }
    } else {
        const f64 *table = info.base == 16 ?
            cy__pow_of_16_table : cy__pow_of_10_table;
        remainder *= table[precision];
    }

    u64 decimal = (u64)remainder;
    remainder -= (f64)decimal;
    if (remainder > 0.5) {
        decimal += 1;
        if (decimal >= (u64)1e16) {
            decimal = 0;
            integral += 1;
            if (exponent != 0 && integral >= 10) {
                exponent += 1;
                integral = 1;
            }
        }
    }

    b32 print_decimal = precision > 0 &&
        !(style_auto && !style_alt && decimal == 0);
    b32 print_dot = print_decimal || style_alt;
    if (print_dot) {
        if (written < limit) {
            *cur++ = '.';
            written += 1;
        }

        written_total += 1;
    }

    if (print_decimal) {
        char *c = cur;
        if (!style_auto || style_alt) {
            i32 extra_digits = info.precision - precision;
            while (extra_digits-- > 0) {
                if (written < limit) {
                    *c++ = '0';
                    written += 1;
                }

                written_total += 1;
            }
        } else {
            while (decimal % (u64)info.base == 0) {
                decimal /= (u64)info.base;
                precision -= 1;
            }
        }

        const char *table = upper ?
            cy__num_to_char_table_upper : cy__num_to_char_table_lower;
        while (precision-- > 0) {
            if (written < limit) {
                *c++ = table[decimal % (u64)info.base];
                written += 1;
            }

            decimal /= (u64)info.base;
            written_total += 1;
        }

        len = c - cur;
        cy_str_reverse_n(cur, len);
        cur += len;
    }

    b32 print_exponent = swap &&
        (style_exp || style_hex || (exponent != 0));
    if (print_exponent) {
        char c = style_hex ? 'p' : 'e';
        c = cy__fmt_char_to_case(c, upper);
        if (written < limit) {
            *cur++ = c;
            written += 1;
        }

        written_total += 1;
        c = (exponent < 0) ? '-' : '+';
        if (written < limit) {
            *cur++ = c;
            written += 1;
        }

        written_total += 1;
        exponent = CY_ABS(exponent);
        if (!style_hex && exponent < 10) {
            if (written < limit) {
                *cur++ = '0';
                written += 1;
            }

            written_total += 1;
        }

        remaining = limit - written;
        len_total = cy__print_i64(cur, remaining, (CyPrivFmtInfo){
            .base = 10,
            .precision = -1,
            .value.i = exponent,
        });

        len = CY_MIN(len_total, remaining);
        cur += len, written += len;
        written_total += len_total;
    }

    len = cur - dst;
    b32 fill_with_zeros = !custom_precision && (len < info.width) &&
        (info.flags & CY__FMT_ZERO) && !(info.flags & CY__FMT_MINUS);
    if (fill_with_zeros) {
        remaining = limit - written;

        isize prefix_len = num_start - dst;
        isize extra_total = info.width - len;
        isize extra = CY_MIN(extra_total, remaining);

        cy_mem_move(num_start + extra, num_start, len - prefix_len);
        cy_mem_set(num_start, '0', extra);

        written += extra;
        written_total += extra_total;
    }

    return written_total;
}

#ifndef CY_STD_C_PRINTF
cy_global const char *cy__b32_to_str_table[4] = {
    "false", "true",
    "FALSE", "TRUE",
};
#endif

isize cy_sprintf_va(char *buf, isize size, const char *fmt, va_list va)
{
    // NOTE(cya): written_total is how much _would_ be written in case the buf
    // wasn't large enough, while written is how much _was_ written which can
    // truncate if we reach the end of the buffer, that way we can report the
    // amount of bytes necessary for this whole conversion to work and the user
    // can allocate a suitable buffer if needed and then recall this procedure
    // (all the cy__print_* subprocs use this pattern to propagate this logic)
    const isize limit = size - 1;
    isize written_total = 0;
    isize written = 0;

    char conv_buf_static[CY__FMT_STATIC_BUF_SIZE];
    isize conv_cap_static = CY_ARRAY_LEN(conv_buf_static) - 1;
    char *conv_buf_heap = NULL;
    isize conv_cap_heap = 0;

    char *end = buf;
    const char *f = fmt;
    for (;;) {
        for (; *f != '\0' && *f != '%'; f++) {
            char c = *f;
#if defined(CY_OS_WINDOWS)
            // NOTE(cya): LF -> CRLF conversion
            if (c == '\n') {
                if (f - fmt < 1 || *(f - 1) != '\r') {
                    if (written < limit) {
                        *end++ = '\r';
                        written += 1;
                    }

                    written_total += 1;
                }
            }
#endif

            if (written < limit) {
                *end++ = c;
                written += 1;
            }

            written_total += 1;
        }

        if (*f++ == '\0') {
            break;
        }

        CyPrivFmtInfo info = {.precision = -1};
        b32 done = false;
        do {
            switch (*f++) {
            case '-': {
                info.flags |= CY__FMT_MINUS;
            } break;
            case '+': {
                info.flags |= CY__FMT_PLUS;
            } break;
            case ' ': {
                info.flags |= CY__FMT_SPACE;
            } break;
            case '#': {
                info.flags |= CY__FMT_HASH;
            } break;
            case '0': {
                info.flags |= CY__FMT_ZERO;
            } break;
            default: {
                done = true;
                f -= 1;
            } break;
            }
        } while (!done);

        if (cy_char_is_digit(*f)) {
            u64 width;
            isize len = cy__scan_u64(f, 10, &width);
            info.width = (i32)width;
            f += len;
        } else if (*f == '*') {
            info.width = va_arg(va, int);
            f += 1;
        }

        if (*f == '.') {
            isize len = 1;
            if (*++f == '*') {
                info.precision = va_arg(va, int);
            } else {
                i64 precision;
                len = cy__scan_i64(f, 10, &precision);
                if (precision >= 0) {
                    info.precision = (i32)precision;
                }
            }

            f += len;
        }

        switch (*f++) {
        case 'h': {
            if (*f == 'h') {
                info.flags |= CY__FMT_LEN_CHAR;
                f += 1;
            } else {
                info.flags |= CY__FMT_LEN_SHORT;
            }
        } break;
        case 'l': {
            if (*f == 'l') {
                info.flags |= CY__FMT_LEN_LONG_LONG;
                f += 1;
            } else {
                info.flags |= CY__FMT_LEN_LONG;
            }
        } break;
        case 'j': {
            info.flags |= CY__FMT_LEN_INTMAX;
        } break;
        case 'z': {
            info.flags |= CY__FMT_LEN_SIZE;
        } break;
        case 't': {
            info.flags |= CY__FMT_LEN_PTRDIFF;
        } break;
        case 'L': {
            info.flags |= CY__FMT_LEN_LONG_DOUBLE;
        } break;
        default: {
            f -= 1;
        } break;
        }

        char *conv_buf = conv_buf_static;
        isize conv_cap = conv_cap_static;
        CyPrivPrintProc *print_proc = NULL;
        isize conv_len = 0;
        b32 out_arg = false;
        switch (*f) {
        case 'd':
        case 'i': {
            info.flags |= CY__FMT_INT;
            info.base = 10;
        } break;
        case 'o':
        case 'u':
        case 'x':
        case 'X': {
            info.flags |= CY__FMT_UNSIGNED;
            switch (*f) {
            case 'o': {
                info.base = 8;
            } break;
            case 'u': {
                info.base = 10;
            } break;
            case 'x':
            case 'X': {
                info.base = 16;
                if (cy_char_is_upper(*f)) {
                    info.flags |= CY__FMT_STYLE_UPPER;
                }
            } break;
            }
        } break;
        case 'f':
        case 'F':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
        case 'a':
        case 'A': {
            info.flags |= CY__FMT_FLOAT;
            info.base = 10;
            switch (cy_char_to_lower(*f)) {
            case 'e': {
                info.flags |= CY__FMT_STYLE_EXP;
            } break;
            case 'g': {
                info.flags |= CY__FMT_STYLE_AUTO;
            } break;
            case 'a': {
                info.base = 16;
            } break;
            }

            if (cy_char_is_upper(*f)) {
                info.flags |= CY__FMT_STYLE_UPPER;
            }
        } break;
        case 'c': {
            if (info.flags & CY__FMT_LEN_LONG) {
                // TODO(cya): convert from wcs to mbs (is it even worth it?)
            } else {
                *(u8*)conv_buf = (u8)va_arg(va, int);
                conv_len = 1;
            }
        } break;
        case 's': {
            if (info.flags & CY__FMT_LEN_LONG) {
                // TODO(cya): same as the todo above
            } else {
                const char *str = va_arg(va, char*);
                info.value.s = cy_string_view_create_c(str);
                print_proc = cy__print_str;
            }
        } break;
        case 'p': {
            void *ptr = va_arg(va, void*);
            print_proc = cy__print_u64;
            info = (CyPrivFmtInfo){
                .base = 16,
                .flags = (CY__FMT_ZERO | CY__FMT_STYLE_UPPER | CY__FMT_HASH),
                .width = 16,
                .value.u = (u64)ptr,
            };
        } break;
        case 'n': {
            out_arg = true;
        } break;
        case '%': {
            *conv_buf = '%';
            conv_len = 1;
        } break;
#ifndef CY_STD_C_PRINTF // NOTE(cya): extended format specifiers
        case 'b':
        case 'B': { // NOTE(cya): binary ints
            info.flags |= CY__FMT_UNSIGNED;
            info.base = 2;
            if (cy_char_is_upper(*f)) {
                info.flags |= CY__FMT_STYLE_UPPER;
            }
        } break;
        case 'q':
        case 'Q': {
            isize ofs = cy_char_is_upper(*f) ? 2 : 0;

            b32 val = !!(b32)va_arg(va, int);
            const char *str = cy__b32_to_str_table[ofs + val];
            info.value.s = cy_string_view_create_c(str);
            print_proc = cy__print_str;
        } break;
        case 'v': {
            info.value.s = va_arg(va, CyStringView);
            print_proc = cy__print_str;
        } break;
#endif
        default: {
            (void)va_arg(va, uintptr);
            const char *msg = "%!(missing format specifier)";
            info.value.s = cy_string_view_create_c(msg);
            info.width = 0;
            print_proc = cy__print_str;
            goto fmt_convert;
        } break;
        }

        f += 1;

#ifndef CY_STD_C_PRINTF
        // NOTE(cya): parsing sized format specs (%u64, %d32, %i16, etc.)
        u8 bit_size = 0, skip = 1;
        if ((info.flags & CY__FMT_INTS) && cy_char_is_digit(*f)) {
            switch (*f) {
            case '8': {
                bit_size = 8;
            } break;
            case '1': {
                if (*(f + 1) == '6') {
                    bit_size = 16;
                    skip += 1;
                }
            } break;
            case '3': {
                if (*(f + 1) == '2') {
                    bit_size = 32;
                    skip += 1;
                }
            } break;
            case '6': {
                if (*(f + 1) == '4') {
                    bit_size = 64;
                    skip += 1;
                }
            } break;
            default: {
                skip = 0;
            } break;
            }

            f += skip;
        }

        if (bit_size > 0) {
            if (info.flags & CY__FMT_UNSIGNED) {
                u64 val = 0;
                switch (bit_size) {
                case 8: {
                    val = (u64)((u8)va_arg(va, int));
                } break;
                case 16: {
                    val = (u64)((u16)va_arg(va, int));
                } break;
                case 32: {
                    val = (u64)va_arg(va, u32);
                } break;
                case 64: {
                    val = va_arg(va, u64);
                } break;
                }

                info.value.u = val;
                print_proc = cy__print_u64;
                goto fmt_convert;
            } else if (info.flags & CY__FMT_INT) {
                i64 val = 0;
                switch (bit_size) {
                case 8: {
                    val = (i64)((i8)va_arg(va, int));
                } break;
                case 16: {
                    val = (i64)((i16)va_arg(va, int));
                } break;
                case 32: {
                    val = (i64)va_arg(va, i32);
                } break;
                case 64: {
                    val = va_arg(va, i64);
                } break;
                }

                info.value.i = val;
                print_proc = cy__print_i64;
                goto fmt_convert;
            } else {
                f -= skip;
            }
        }
#endif

        if (out_arg) {
            void *out = va_arg(va, void*);
            if (out != NULL) {
                isize val = written_total;
                switch (info.flags & CY__FMT_LEN_MODS) {
                case CY__FMT_LEN_CHAR: {
                    *((signed char*)out) = (signed char)val;
                } break;
                case CY__FMT_LEN_SHORT: {
                    *((short*)out) = (short)val;
                } break;
                case CY__FMT_LEN_LONG: {
                    *((long*)out) = (long)val;
                } break;
                case CY__FMT_LEN_LONG_LONG: {
                    *((long long*)out) = (long long)val;
                } break;
                case CY__FMT_LEN_INTMAX: {
                    *((intmax_t*)out) = (intmax_t)val;
                } break;
                case CY__FMT_LEN_SIZE:
                case CY__FMT_LEN_PTRDIFF: {
                    *((isize*)out) = val;
                } break;
                default: {
                    *((int*)out) = (int)val;
                } break;
                }
            }

            continue;
        } else if (info.base != 0) { // NOTE(cya): print a number (C notation)
            if (info.flags & CY__FMT_INTS) {
                if (info.flags & CY__FMT_UNSIGNED) {
                    u64 val;
                    switch (info.flags & CY__FMT_LEN_MODS) {
                    case CY__FMT_LEN_CHAR: {
                        val = (u64)((unsigned char)va_arg(va, int));
                    } break;
                    case CY__FMT_LEN_SHORT: {
                        val = (u64)((unsigned short)va_arg(va, int));
                    } break;
                    case CY__FMT_LEN_LONG: {
                        val = (u64)va_arg(va, unsigned long);
                    } break;
                    case CY__FMT_LEN_LONG_LONG: {
                        val = (u64)va_arg(va, unsigned long long);
                    } break;
                    case CY__FMT_LEN_INTMAX: {
                        val = (u64)va_arg(va, uintmax_t);
                    } break;
                    case CY__FMT_LEN_SIZE:
                    case CY__FMT_LEN_PTRDIFF: {
                        val = (u64)va_arg(va, usize);
                    } break;
                    default: {
                        val = (u64)va_arg(va, unsigned);
                    } break;
                    }

                    info.value.u = val;
                    print_proc = cy__print_u64;
                } else if (info.flags & CY__FMT_INT) {
                    i64 val;
                    switch (info.flags & CY__FMT_LEN_MODS) {
                    case CY__FMT_LEN_CHAR: {
                        val = (i64)((signed char)va_arg(va, int));
                    } break;
                    case CY__FMT_LEN_SHORT: {
                        val = (i64)((short)va_arg(va, int));
                    } break;
                    case CY__FMT_LEN_LONG: {
                        val = (i64)va_arg(va, long);
                    } break;
                    case CY__FMT_LEN_LONG_LONG: {
                        val = (i64)va_arg(va, long long);
                    } break;
                    case CY__FMT_LEN_INTMAX: {
                        val = (i64)va_arg(va, intmax_t);
                    } break;
                    case CY__FMT_LEN_SIZE:
                    case CY__FMT_LEN_PTRDIFF: {
                        val = (i64)va_arg(va, isize);
                    } break;
                    default: {
                        val = (i64)va_arg(va, int);
                    } break;
                    }

                    info.value.i = val;
                    print_proc = cy__print_i64;
                }
            } else if (info.flags & CY__FMT_FLOAT) {
                info.value.f = info.flags & CY__FMT_LEN_LONG_DOUBLE ?
                    (f64)va_arg(va, long double) : va_arg(va, f64);
                print_proc = cy__print_f64;
            }
        }

fmt_convert:
        if (print_proc != NULL) {
            conv_len = print_proc(conv_buf, conv_cap, info);
            isize remaining = limit - written;
            if (conv_len > conv_cap && conv_cap < remaining) {
                // TODO(cya): allocate heap buf and reapply conversion
                conv_cap = conv_len + 1;
                char *new_heap_buf = cy_default_resize(
                    cy_heap_allocator(), conv_buf_heap,
                    conv_cap_heap, conv_cap
                );
                if (new_heap_buf == NULL) {
                    // TODO(cya): maybe this could be handled better?
                    cy_free(cy_heap_allocator(), conv_buf_heap);
                    return -1;
                }

                conv_buf_heap = new_heap_buf;
                conv_cap_heap = conv_cap;
                conv_buf = conv_buf_heap;
                conv_len = print_proc(conv_buf, conv_cap, info);
            }
        }


        b32 right_pad = info.flags & CY__FMT_MINUS;
        b32 fill_with_zeros = !right_pad && (info.flags & CY__FMT_ZERO);
        if (!fill_with_zeros && conv_len < info.width) {
            // NOTE(cya): truncating to remaining length
            isize remaining = conv_cap - conv_len;
            char *conv_end = conv_buf + conv_len;

            isize extra = CY_MIN(info.width - conv_len, remaining);
            if (right_pad) {
                cy_mem_set(conv_end, ' ', extra);
            } else { // NOTE(cya): npm
                cy_mem_move(conv_buf + extra, conv_buf, conv_len);
                cy_mem_set(conv_buf, ' ', extra);
            }

            conv_len += extra;
        }

        if (written < limit) {
            isize remaining = limit - written;
            isize copy_len = CY_MIN(remaining, conv_len);

            cy_mem_copy(end, conv_buf, copy_len);
            end += copy_len, written += copy_len;
        }

        written_total += conv_len;
    }

    if (conv_buf_heap != NULL) {
        cy_free(cy_heap_allocator(), conv_buf_heap);
    }

    if (end != NULL) {
        *end = '\0';
    }

    return written_total;
}

/* ============================= Virtual memory ============================= */
cy_global CyOSMemoryBlock cy__os_memory_block_sentinel = {
    .prev = &cy__os_memory_block_sentinel,
    .next = &cy__os_memory_block_sentinel,
};

cy_internal CyOSMemoryBlock *cy__os_virtual_memory_reserve(isize size);
cy_internal void cy__os_virtual_memory_commit(void *mem, isize size);
cy_internal void cy__os_virtual_memory_free(CyOSMemoryBlock *block);
cy_internal void cy__os_virtual_memory_protect(void *memory, isize size);

cy_inline CyMemoryBlock *cy_virtual_memory_alloc(isize size)
{
    return cy_virtual_memory_alloc_reserve(size, size);
}

CyMemoryBlock *cy_virtual_memory_alloc_reserve(isize size, isize commit_size)
{
    CY_ASSERT(commit_size <= size);

    isize page_size = cy_virtual_memory_page_size(NULL);
    isize total_size = size + cy_sizeof(CyOSMemoryBlock);
    isize total_commit_size = commit_size + cy_sizeof(CyOSMemoryBlock);
    isize start_offset = cy_sizeof(CyOSMemoryBlock);

    isize aligned_size = cy_align_forward_size(total_size, page_size);
    isize aligned_commit_size = cy_align_forward_size(
        total_commit_size, page_size
    );
    total_size = aligned_size + 2 * page_size;
    total_commit_size = page_size + aligned_commit_size;

    start_offset = page_size + aligned_size - size;
    isize protect_offset = page_size + aligned_size;

    CyOSMemoryBlock *os_block = cy__os_virtual_memory_reserve(total_size);
    CY_ASSERT_MSG(os_block != NULL, "out of virtual memory D:");

    cy__os_virtual_memory_commit(os_block, total_commit_size);
    CY_ASSERT(os_block->block.start == NULL);

    os_block->block.start = (u8*)os_block + start_offset;
    os_block->block.size = size;
    os_block->commit_size = commit_size;
    os_block->total_size = total_size;

    cy__os_virtual_memory_protect((u8*)os_block + protect_offset, page_size);

    CyOSMemoryBlock *sentinel = &cy__os_memory_block_sentinel;
    os_block->next = sentinel;
    os_block->prev = sentinel->prev;
    os_block->next->prev = os_block;
    os_block->prev->next = os_block;

    return &os_block->block;
}

void cy_virtual_memory_free(CyMemoryBlock *block)
{
    CyOSMemoryBlock *os_block = (CyOSMemoryBlock*)block;
    if (os_block == NULL) {
        return;
    }

    os_block->prev->next = os_block->next;
    os_block->next->prev = os_block->prev;

    cy__os_virtual_memory_free(os_block);
}

cy_inline CyMemoryBlock *cy_virtual_memory_resize(
    CyMemoryBlock *block, isize new_size
) {
    CY_VALIDATE_PTR(block);

    isize old_size = block->size;
    if (new_size < old_size) {
        void *new_start = (u8*)block->start + (old_size - new_size);
        cy_mem_move(new_start, block->start, new_size);

        block->start = new_start;
        block->size = new_size;
        return block;
    }

    CyMemoryBlock *new_block = cy_virtual_memory_alloc(new_size);
    CY_VALIDATE_PTR(new_block);

    cy_mem_copy(new_block->start, block->start, block->size);
    return new_block;
}

#if defined(CY_OS_WINDOWS)
cy_inline isize cy_virtual_memory_page_size(isize *align_out)
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    if (align_out != NULL) {
        *align_out = (isize)info.dwAllocationGranularity;
    }

    return (isize)info.dwPageSize;
}

cy_internal cy_inline CyOSMemoryBlock *cy__os_virtual_memory_reserve(isize size)
{
    return VirtualAlloc(NULL, (usize)size, MEM_RESERVE, PAGE_READWRITE);
}

cy_internal cy_inline void cy__os_virtual_memory_commit(void *mem, isize size)
{
    VirtualAlloc(mem, (usize)size, MEM_COMMIT, PAGE_READWRITE);
}

cy_internal cy_inline void cy__os_virtual_memory_free(CyOSMemoryBlock *block)
{
    VirtualFree(block, 0, MEM_RELEASE);
}

cy_internal cy_inline void cy__os_virtual_memory_protect(void *mem, isize size)
{
    cy__os_virtual_memory_commit(mem, size);

    DWORD old_protect = 0;
    BOOL ok = VirtualProtect(mem, (usize)size, PAGE_NOACCESS, &old_protect);
    CY_ASSERT(ok);
}
#else

#endif

/* =============================== Allocators =============================== */
cy_inline void *cy_alloc_align(CyAllocator a, isize size, isize align)
{
    return a.proc(
        a.data, CY_ALLOCATION_ALLOC,
        size, align,
        NULL, 0,
        CY_DEFAULT_ALLOCATOR_FLAGS
    );
}

cy_inline void *cy_alloc(CyAllocator a, isize size)
{
    return cy_alloc_align(a, size, CY_DEFAULT_ALIGNMENT);
}

cy_inline void cy_free(CyAllocator a, void *ptr)
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

cy_inline void cy_free_all(CyAllocator a)
{
    a.proc(
        a.data, CY_ALLOCATION_FREE_ALL,
        0, 0,
        NULL, 0,
        CY_DEFAULT_ALLOCATOR_FLAGS
    );
}

cy_inline void *cy_resize_align(
    CyAllocator a, void *ptr, isize old_size, isize new_size, isize align
) {
    return a.proc(
        a.data, CY_ALLOCATION_RESIZE,
        new_size, align,
        ptr, old_size,
        CY_DEFAULT_ALLOCATOR_FLAGS
    );
}

cy_inline void *cy_resize(CyAllocator a, void *ptr, isize old_size, isize new_size)
{
    return cy_resize_align(a, ptr, old_size, new_size, CY_DEFAULT_ALIGNMENT);
}

cy_inline void *cy_default_resize_align(
    CyAllocator a, void *old_mem, isize old_size, isize new_size, isize align
) {
    if (old_mem == NULL) {
        return cy_alloc_align(a, new_size, align);
    }
    if (new_size == 0) {
        cy_free(a, old_mem);
        return NULL;
    }
    if (new_size <= old_size) {
        return old_mem;
    }

    void *new_mem = cy_alloc_align(a, new_size, align);
    if (new_mem == NULL) {
        return NULL;
    }

    cy_mem_move(new_mem, old_mem, old_size);
    cy_free(a, old_mem);
    return new_mem;
}

cy_inline void *cy_default_resize(
    CyAllocator a, void *old_mem, isize old_size, isize new_size
) {
    return cy_default_resize_align(
        a, old_mem,
        old_size, new_size,
        CY_DEFAULT_ALIGNMENT
    );
}

cy_inline void *cy_alloc_copy_align(
    CyAllocator a, const void *src, isize size, isize align
) {
    return cy_mem_copy(cy_alloc_align(a, size, align), src, size);
}

cy_inline void *cy_alloc_copy(CyAllocator a, const void *src, isize size)
{
    return cy_alloc_copy_align(a, src, size, CY_DEFAULT_ALIGNMENT);
}

cy_inline char *cy_alloc_string_len(CyAllocator a, const char *str, isize len)
{
    char *res = cy_alloc_copy(a, str, len);
    res[len] = '\0';
    return res;
}

cy_inline char *cy_alloc_string(CyAllocator a, const char *str)
{
    return cy_alloc_string_len(a, str, cy_str_len(str));
}

cy_inline CyAllocator cy_null_allocator(void)
{
    return (CyAllocator){
        .proc = cy_null_allocator_proc,
    };
}

CY_ALLOCATOR_PROC(cy_null_allocator_proc)
{
    CY_UNUSED(allocator_data);
    CY_UNUSED(size);
    CY_UNUSED(align);
    CY_UNUSED(old_size);
    CY_UNUSED(flags);

    void *ptr = NULL;
    switch(type) {
    case CY_ALLOCATION_FREE: {
        CY_ASSERT_MSG(old_mem == NULL, "null allocator: invalid pointer");
    } break;
    default: break;
    }

    return ptr;
}

cy_inline CyAllocator cy_heap_allocator(void)
{
    return (CyAllocator){
        .proc = cy_heap_allocator_proc,
    };
}

#if defined(CY_OS_WINDOWS)
    #include <malloc.h>

    #define malloc_align(s, a) _aligned_malloc((usize)s, (usize)a)
    #define realloc_align(alloc, mem, old_size, new_size, align) \
        _aligned_realloc(mem, (usize)new_size, (usize)align)
    #define free_align(p, a) _aligned_free(p)
#else
extern int posix_memalign(void**, size_t, size_t);

void *malloc_align(isize size, isize align)
{
    void *ptr = NULL;
    if (posix_memalign(&ptr, align, size) != 0) {
        return NULL;
    }

    return ptr;
}

    #define realloc_align cy_default_resize_align
    #define free_align(p, a) free(p)
#endif

CY_ALLOCATOR_PROC(cy_heap_allocator_proc)
{
#if defined(CY_OS_WINDOWS)
    CY_UNUSED(old_size);
#endif

    CY_UNUSED(allocator_data);
    CY_UNUSED(old_size);

    void *ptr = NULL;
    switch(type) {
    case CY_ALLOCATION_ALLOC: {
        ptr = malloc_align(size, align);
        if (flags & CY_ALLOCATOR_CLEAR_TO_ZERO) {
            cy_mem_zero(ptr, size);
        }
    } break;
    case CY_ALLOCATION_ALLOC_ALL: {
    } break;
    case CY_ALLOCATION_FREE: {
        free_align(old_mem, align);
    } break;
    case CY_ALLOCATION_FREE_ALL: {
    } break;
    case CY_ALLOCATION_RESIZE: {
        void *new_ptr = realloc_align(
            cy_heap_allocator(), old_mem,
            old_size, size, align
        );
        CY_VALIDATE_PTR(new_ptr);
        ptr = new_ptr;
    } break;
    }

    return ptr;
}

/* ---------------------------- Static Allocator ---------------------------- */
cy_inline CyAllocator cy_static_allocator(CyBuffer *buf)
{
    return (CyAllocator){
        .proc = cy_static_allocator_proc,
        .data = buf,
    };
}

cy_inline CyBuffer cy_static_buf_init(void *backing_buf, isize size)
{
    return (CyBuffer){
        .memory = backing_buf,
        .size = size,
    };
}

CY_ALLOCATOR_PROC(cy_static_allocator_proc)
{
    CY_UNUSED(align);
    CY_UNUSED(old_mem);
    CY_UNUSED(old_size);

    CyBuffer *buf = allocator_data;
    void *ptr = NULL;
    switch (type) {
    case CY_ALLOCATION_ALLOC: {
        CY_ASSERT_MSG(size == buf->size, "static_allocator: invalid size");
    } // NOTE(cya): fallthrough
    case CY_ALLOCATION_ALLOC_ALL: {
        ptr = buf->memory;
        if (flags & CY_ALLOCATOR_CLEAR_TO_ZERO) {
            cy_mem_zero(ptr, size);
        }
    } break;
    case CY_ALLOCATION_FREE:
    case CY_ALLOCATION_FREE_ALL:
    case CY_ALLOCATION_RESIZE: {
        CY_PANIC("static allocator: unsupported operation");
    } break;
    }

    return ptr;
}

/* ---------------------- Virtual Memory (VM) Allocator --------------------- */
CyAllocator cy_virtual_memory_allocator(void)
{
    return (CyAllocator){
        .proc = cy_virtual_memory_allocator_proc,
    };
}

cy_internal uintptr *cy__vm_header_from_alloc_start(void *ptr)
{
    return (uintptr*)((u8*)ptr - cy_sizeof(uintptr));
}

CY_ALLOCATOR_PROC(cy_virtual_memory_allocator_proc)
{
    CY_UNUSED(allocator_data);
    CY_UNUSED(flags);
    CY_UNUSED(old_size);

    void *ptr = NULL;
    switch (type) {
    case CY_ALLOCATION_ALLOC: {
        CY_ASSERT_MSG(size > 0, "VM allocator: invalid allocation size");

        isize alignment = CY_MAX(align, cy_sizeof(uintptr));
        isize total_size = size + alignment - 1;
        CyMemoryBlock *block = cy_virtual_memory_alloc(total_size);
        if (block == NULL) {
            CY_PANIC("VM allocator: out of virtual memory");
            break;
        }

        ptr = cy_align_forward_ptr(block->start, alignment);
        uintptr *header = cy__vm_header_from_alloc_start(ptr);
        *header = (uintptr)block;
    } break;
    case CY_ALLOCATION_FREE: {
        uintptr *header = cy__vm_header_from_alloc_start(old_mem);
        CyMemoryBlock *block = (CyMemoryBlock*)(*header);
        cy_virtual_memory_free(block);
    } break;
    case CY_ALLOCATION_RESIZE: {
        uintptr *header = cy__vm_header_from_alloc_start(old_mem);
        CyMemoryBlock *block = (CyMemoryBlock*)(*header);

        CyMemoryBlock *new_block = cy_virtual_memory_resize(block, size);
        if (new_block == NULL) {
            break;
        }

        ptr = new_block->start;
        header = cy__vm_header_from_alloc_start(ptr);
        *header = (uintptr)new_block;
    } break;
    case CY_ALLOCATION_ALLOC_ALL:
    case CY_ALLOCATION_FREE_ALL: {
        CY_PANIC("VM allocator: unsupported operation");
    } break;
    }

    return ptr;
}

/* ---------------------------- Arena Allocator ----------------------------- */
cy_inline CyAllocator cy_arena_allocator(CyArena *arena)
{
    return (CyAllocator){
        .proc = cy_arena_allocator_proc,
        .data = arena,
    };
}

#ifndef CY_ARENA_DEFAULT_SIZE
    #define CY_ARENA_DEFAULT_SIZE CY_KB(16)
#endif

#ifndef CY_ARENA_GROWTH_FACTOR
    #define CY_ARENA_GROWTH_FACTOR 2.0
#endif

cy_inline CyMemoryBlock *cy_arena_insert_block(CyArena *arena, isize size)
{
    CY_VALIDATE_PTR(arena);

    isize align = CY_DEFAULT_ALIGNMENT;
    isize block_padding = cy_sizeof(CyMemoryBlock) + align - 1;
    isize new_block_size = block_padding + size;
    CyMemoryBlock *new_block = cy_alloc(arena->backing, new_block_size);
    CY_VALIDATE_PTR(new_block);

    new_block->start = cy_align_forward_ptr(new_block + 1, align);
    new_block->size = size;
    new_block->prev = arena->cur_block;
    arena->cur_block = new_block;

    return new_block;
}

cy_inline CyArena cy_arena_init(CyAllocator backing, isize initial_size)
{
    CY_ASSERT_NOT_NULL(backing.proc);

    isize default_size = CY_ARENA_DEFAULT_SIZE;
    if (initial_size == 0) {
        initial_size = default_size;
    }

    CyArena arena = {.backing = backing};
    cy_arena_insert_block(&arena, initial_size);
    return arena;
}

cy_inline void cy_arena_deinit(CyArena *arena)
{
    if (arena == NULL) {
        return;
    }

    CyMemoryBlock *cur_block = arena->cur_block;
    while (cur_block != NULL) {
        CyMemoryBlock *prev = cur_block->prev;
        cy_free(arena->backing, cur_block);
        cur_block = prev;
    }

    cy_mem_set(arena, 0, cy_sizeof(*arena));
}

CY_ALLOCATOR_PROC(cy_arena_allocator_proc)
{
    CyArena *arena = (CyArena*)allocator_data;
    CyAllocator a = cy_arena_allocator(arena);
    void *ptr = NULL;
    switch(type) {
    case CY_ALLOCATION_ALLOC: {
        CyMemoryBlock *cur_block = arena->cur_block;
        u8 *start = cur_block->start;
        u8 *end = start + cur_block->offset;
        u8 *aligned_end = cy_align_forward_ptr(end, align);
        while (aligned_end + size > start + cur_block->size) {
            if (cur_block->prev == NULL) {
                // NOTE(cya): out of memory in this block!
                f64 cur_size = (f64)cur_block->size;
                isize new_size = (isize)(cur_size * CY_ARENA_GROWTH_FACTOR);

                cur_block = cy_arena_insert_block(arena, new_size);
                CY_VALIDATE_PTR(cur_block);

                end = (u8*)cur_block->start + cur_block->offset;
                aligned_end = cy_align_forward_ptr(end, align);
                break;
            }

            cur_block = cur_block->prev;
            end = (u8*)cur_block->start + cur_block->offset;
            aligned_end = cy_align_forward_ptr(end, align);
        }

        isize aligned_offset = aligned_end - (u8*)cur_block->start;

        cur_block->prev_offset = aligned_offset;
        cur_block->offset = aligned_offset + size;

        ptr = aligned_end;
        if (flags & CY_ALLOCATOR_CLEAR_TO_ZERO) {
            cy_mem_zero(ptr, size);
        }
    } break;
    case CY_ALLOCATION_ALLOC_ALL: {
        CyAllocator backing = arena->backing;
        CyMemoryBlock *cur_block = arena->cur_block->prev, *prev;
        while (cur_block != NULL) {
            prev = cur_block->prev;
            cy_free(backing, cur_block);
            cur_block = prev;
        }

        cur_block = arena->cur_block;
        isize remaining = cur_block->size - cur_block->offset;
        ptr = cy_alloc_align(backing, remaining, align);
        CY_VALIDATE_PTR(ptr);

        ptr = cy_align_forward_ptr(ptr, align);
    } break;
    case CY_ALLOCATION_FREE: {
    } break;
    case CY_ALLOCATION_FREE_ALL: {
        CyMemoryBlock *cur_block = arena->cur_block, *prev = cur_block->prev;
        while (prev != NULL) {
            cur_block = prev;
            prev = prev->prev;
            cy_free(arena->backing, cur_block);
        }

        cur_block = arena->cur_block;
        cy_mem_set(cur_block, 0, cy_sizeof(*cur_block) + cur_block->size);
    } break;
    case CY_ALLOCATION_RESIZE: {
        CY_ASSERT(cy_is_power_of_two(align));

        u8 *old_memory = old_mem;
        if (old_memory == NULL || old_size == 0) {
            return cy_alloc_align(a, size, align);
        }

        CyMemoryBlock *cur_block = arena->cur_block;
        u8 *start = cur_block->start;
        isize prev_offset = 0;
        b32 is_in_range = false, found_node = false;
        while (cur_block != NULL) {
            is_in_range = old_memory >= start &&
                old_memory < (start + cur_block->size);
            if (is_in_range) {
                prev_offset = cur_block->prev_offset;
                found_node = start + prev_offset == old_memory &&
                    (cur_block->offset - cur_block->prev_offset == old_size);
                break;
            }

            cur_block = cur_block->prev;
            start = cur_block->start;
        }

        if (!is_in_range) {
            CY_PANIC("arena allocator: out-of-bounds reallocation");
            break;
        } else if (!found_node) {
            // NOTE(cya): is in valid space but is not the latest allocation
            // (this might result in some odd behavior in some edge cases)
            void *new_mem = cy_alloc_align(a, size, align);
            CY_VALIDATE_PTR(new_mem);

            cy_mem_copy(new_mem, old_mem, CY_MIN(old_size, size));
            ptr = new_mem;
            break;
        }

        intptr aligned_offset = cy_align_forward_size(prev_offset, align);
        u8 *new_memory = start + aligned_offset;
        if (new_memory + size < start + cur_block->size) {
            cur_block->offset = aligned_offset + size;
            if (cur_block->offset < cur_block->prev_offset + old_size) {
                cy_mem_set(start + cur_block->offset, 0, old_size - size);
            }

            cur_block->prev_offset = aligned_offset;
        } else {
            void *new_ptr = cy_alloc_align(a, size, align);
            CY_VALIDATE_PTR(new_ptr);

            cy_mem_move(new_ptr, old_memory, CY_MIN(old_size, size));
            new_memory = new_ptr;
        }

        ptr = new_memory;
    } break;
    }

    return ptr;
}

/* -----------------------------Stack Allocator ----------------------------- */
cy_inline CyAllocator cy_stack_allocator(CyStack *stack)
{
    return (CyAllocator){
        .data = stack,
        .proc = cy_stack_allocator_proc,
    };
}

#ifndef CY_STACK_DEFAULT_SIZE
    #define CY_STACK_DEFAULT_SIZE CY_KB(16)
#endif

#ifndef CY_STACK_GROWTH_FACTOR
    #define CY_STACK_GROWTH_FACTOR 2.0
#endif

cy_inline CyStackNode *cy_stack_insert_node(CyStack *stack, isize size)
{
    CY_VALIDATE_PTR(stack);

    isize node_padding = cy_sizeof(CyStackNode) + CY_DEFAULT_ALIGNMENT;
    isize new_node_size = node_padding + size;
    CyStackNode *new_node = cy_alloc(stack->backing, new_node_size);
    CY_VALIDATE_PTR(new_node);

    new_node->buf = cy_align_forward_ptr(new_node + 1, CY_DEFAULT_ALIGNMENT);
    new_node->size = size;
    new_node->next = stack->state.first_node;
    stack->state.first_node = new_node;

    return new_node;
}

cy_inline CyStack cy_stack_init(CyAllocator backing, isize initial_size)
{
    isize default_size = CY_STACK_DEFAULT_SIZE;
    if (initial_size == 0) {
        initial_size = default_size;
    }

    CyStack stack = {
        .backing = backing,
    };
    stack.state.first_node = cy_stack_insert_node(&stack, initial_size);
    return stack;
}

cy_inline void cy_stack_deinit(CyStack *stack)
{
    if (stack == NULL) {
        return;
    }

    CyStackNode *cur_node = stack->state.first_node;
    while (cur_node != NULL) {
        CyStackNode *next = cur_node->next;
        cy_free(stack->backing, cur_node);
        cur_node = next;
    }

    cy_mem_set(stack, 0, cy_sizeof(*stack));
}

CY_ALLOCATOR_PROC(cy_stack_allocator_proc)
{
    CY_UNUSED(flags);
    CY_ASSERT(cy_is_power_of_two(align));

    CyStack *stack = (CyStack*)allocator_data;
    CyAllocator a = cy_stack_allocator(stack);
    void *ptr = NULL;
    switch(type) {
    case CY_ALLOCATION_ALLOC: {
        CyStackNode *cur_node = stack->state.first_node;
        u8 *cur_addr = cur_node->buf + cur_node->offset;
        CyStackHeader *header;
        isize padding = cy_calc_header_padding(
            (uintptr)cur_addr, align, cy_sizeof(*header)
        );
        u8 *alloc_end = cur_addr + padding + size;
        while (alloc_end > cur_node->buf + cur_node->size) {
            if (cur_node->next == NULL) {
                f64 largest_node_size = (f64)stack->state.first_node->size;
                isize new_size = (isize)
                    (largest_node_size * CY_STACK_GROWTH_FACTOR);
                isize max_alloc_size = align + cy_sizeof(*header) + size;

                cur_node = cy_stack_insert_node(
                    stack, CY_MAX(new_size, max_alloc_size)
                );
                CY_VALIDATE_PTR(cur_node);
            } else {
                cur_node = cur_node->next;
            }

            cur_addr = cur_node->buf + cur_node->offset;
            padding = cy_calc_header_padding(
                (uintptr)cur_addr, align, cy_sizeof(*header)
            );
            alloc_end = cur_addr + padding + size;
        }

        ptr = cur_addr + padding;
        header = (CyStackHeader*)ptr - 1;
        header->padding = padding;
        header->prev_offset = cur_node->offset;

        cur_node->prev_offset = header->prev_offset;
        cur_node->offset += size + padding;
    } break;
    case CY_ALLOCATION_ALLOC_ALL: {
        CyAllocator backing = stack->backing;
        CyStackNode *cur_node = stack->state.first_node->next, *next;
        while (cur_node != NULL) {
            next = cur_node->next;
            cy_free(backing, cur_node);
            cur_node = next;
        }

        cur_node = stack->state.first_node;
        isize remaining = cur_node->size - cur_node->offset;
        ptr = cy_alloc_align(backing, remaining, align);
        CY_VALIDATE_PTR(ptr);

        ptr = cy_align_forward_ptr(ptr, align);
    } break;
    case CY_ALLOCATION_FREE: {
        CY_VALIDATE_PTR(old_mem);

        CyStackNode *cur_node = stack->state.first_node;
        u8 *start = cur_node->buf;
        u8 *end = start + cur_node->size;
        u8 *cur_addr = old_mem;
        if (!(start <= cur_addr && cur_addr < end)) {
            CY_PANIC("stack allocator: out-of-bounds pointer");
            break;
        }
        if (cur_addr >= start + cur_node->offset) {
            break; // NOTE(cya): allowing double-frees
        }

        CyStackHeader *header = (CyStackHeader*)(uintptr)cur_addr - 1;
        isize prev_offset = cur_addr - header->padding - start;
        if (prev_offset != header->prev_offset) {
            CY_PANIC("stack allocator: out-of-order deallocation");
            break;
        }

        cur_node->offset = cur_node->prev_offset;
        cur_node->prev_offset = header->prev_offset;
    } break;
    case CY_ALLOCATION_FREE_ALL: {
        CyStackNode *cur_node = stack->state.first_node, *next = cur_node->next;
        while (next != NULL) {
            cur_node = next;
            next = next->next;
            cy_free(stack->backing, cur_node);
        }

        cur_node = stack->state.first_node;
        cy_mem_set(cur_node->buf, 0, cur_node->size);
        cur_node->prev_offset = cur_node->offset = 0;
        cur_node->next = NULL;
    } break;
    case CY_ALLOCATION_RESIZE: {
        if (old_mem == NULL || old_size == 0) {
            return cy_alloc_align(a, size, align);
        } else if (size == 0) {
            cy_free(a, old_mem);
            break;
        }

        CyStackNode *cur_node = stack->state.first_node;
        u8 *start = cur_node->buf;
        u8 *end = start + cur_node->size;
        u8 *cur_addr = old_mem;
        if (!(start <= cur_addr && cur_addr < end)) {
            CY_PANIC("stack allocator: out-of-bounds reallocation");
            break;
        }
        if (cur_addr >= start + cur_node->offset) {
            CY_PANIC("stack allocator: out-of-order reallocation");
            break;
        }

        b32 unchanged = old_size == size &&
            old_mem == cy_align_forward_ptr(old_mem, align);
        if (unchanged) {
            return old_mem;
        }

        CyStackHeader *header = (CyStackHeader*)(uintptr)cur_addr - 1;
        isize cur_padding = header->padding;
        u8 *alloc_start = cur_addr - cur_padding;
        isize new_padding = cy_calc_header_padding(
            (uintptr)alloc_start, align, cy_sizeof(*header)
        );
        if (size < old_size && new_padding == cur_padding) {
            cur_node->offset -= (old_size - size);
            return cy_align_forward_ptr(old_mem, align);
        }

        isize prev_offset = (isize)(cur_addr - cur_padding - start);
        isize new_offset = (isize)(alloc_start + new_padding + size - start);
        if (new_offset <= cur_node->size) {
            u8 *new_addr = alloc_start + new_padding;
            header = (CyStackHeader*)(uintptr)new_addr - 1;
            header->padding = new_padding;
            header->prev_offset = prev_offset;

            cur_node->offset = new_offset;
            ptr = alloc_start;
            break;
        }

        // NOTE(cya): not enough memory on current node
        f64 largest_node_size = (f64)stack->state.first_node->size;
        isize new_size = (isize)(largest_node_size * CY_STACK_GROWTH_FACTOR);
        isize max_alloc_size = align + cy_sizeof(*header) + size;

        CyStackNode *prev_node = cur_node;
        cur_node = cy_stack_insert_node(
            stack, CY_MAX(new_size, max_alloc_size)
        );
        CY_VALIDATE_PTR(cur_node);

        cur_addr = cur_node->buf;
        new_padding = cy_calc_header_padding(
            (uintptr)cur_addr, align, cy_sizeof(*header)
        );

        ptr = cur_addr + new_padding;
        cy_mem_copy(ptr, old_mem, old_size);

        // NOTE(cya): manually freeing memory from old node to avoid bounds
        // checking from free procedure
        CyStackHeader *old_header = (CyStackHeader*)old_mem - 1;
        prev_node->offset = prev_node->prev_offset;
        prev_node->prev_offset = old_header->prev_offset;

        header = (CyStackHeader*)ptr - 1;
        header->padding = new_padding;
        header->prev_offset = cur_node->offset;

        cur_node->offset += new_padding + size;
    } break;
    }

    return ptr;
}

/* ----------------------------- Pool Allocator ----------------------------- */
cy_inline CyAllocator cy_pool_allocator(CyPool *pool)
{
    return (CyAllocator){
        .proc = cy_pool_allocator_proc,
        .data = pool,
    };
}


cy_inline CyPool cy_pool_init(CyAllocator backing, isize chunks, isize chunk_size)
{
    return cy_pool_init_align(
        backing, chunks, chunk_size, CY_DEFAULT_ALIGNMENT
    );
}

cy_inline CyPool cy_pool_init_align(
    CyAllocator backing, isize chunks, isize chunk_size, isize chunk_align
) {
    CyPool pool = {0};
    if (chunk_size < cy_sizeof(uintptr)) {
        CY_PANIC("pool allocator: chunk size is too small");
        return pool;
    }

    isize chunk_size_total = chunk_size + chunk_align;
    isize buf_size = chunks * chunk_size_total;
    void *buf = cy_alloc_align(backing, buf_size, chunk_align);
    if (buf == NULL) {
        buf_size = 0;
        chunk_size = 0;
    }

    pool = (CyPool){
        .backing = backing,
        .memory = buf,
        .free_list_head = buf,
        .chunks = chunks,
        .chunk_size = chunk_size,
        .chunk_align = chunk_align,
    };
    CyAllocator a = cy_pool_allocator(&pool);
    cy_free_all(a);

    return pool;
}

cy_inline void cy_pool_deinit(CyPool *pool)
{
    if (pool == NULL) {
        return;
    }

    cy_free(pool->backing, pool->memory);
    cy_mem_set(pool, 0, cy_sizeof(*pool));
}

CY_ALLOCATOR_PROC(cy_pool_allocator_proc)
{
    CY_UNUSED(old_size);

    CyPool *pool = allocator_data;
    void *ptr = NULL;
    switch (type) {
    case CY_ALLOCATION_ALLOC: {
        CY_ASSERT(size == pool->chunk_size);
        CY_ASSERT(align == pool->chunk_align);
        if (pool->free_list_head == NULL) {
            CY_PANIC("pool allocator: out of memory");
            break;
        }

        ptr = pool->free_list_head;

        uintptr *next_free = (uintptr*)pool->free_list_head;
        pool->free_list_head = (void*)*next_free;
        if (flags & CY_ALLOCATOR_CLEAR_TO_ZERO) {
            cy_mem_zero(ptr, size);
        }
    } break;
    case CY_ALLOCATION_FREE: {
        if (old_mem == NULL) {
            break;
        }

        isize chunk_size_total = pool->chunk_size + pool->chunk_align;
        isize pool_size = pool->chunks * chunk_size_total;
        void *pool_end = (void*)((uintptr)pool->memory + (uintptr)pool_size);
        if (old_mem < pool->memory || old_mem > pool_end) {
            CY_PANIC("pool allocator: out-of-bounds free");
            break;
        }

        uintptr *next_free = (uintptr*)old_mem;
        *next_free = (uintptr)pool->free_list_head;
        pool->free_list_head = (void*)next_free;
    } break;
    case CY_ALLOCATION_FREE_ALL: {
        isize chunk_size_total = pool->chunk_size + pool->chunk_align;
        void *cur_chunk = pool->memory;
        for (isize i = 0; i < pool->chunks - 1; i++) {
            uintptr *next = (uintptr*)cur_chunk;
            *next = (uintptr)cur_chunk + (uintptr)chunk_size_total;
            cur_chunk = (void*)(*next);
        }

        uintptr *end = (uintptr*)cur_chunk;
        *end = (uintptr)NULL; // NOTE(cya): free list tail
    } break;
    case CY_ALLOCATION_ALLOC_ALL:
    case CY_ALLOCATION_RESIZE: {
        CY_PANIC("pool allocator: unsupported operation");
    } break;
    }

    return ptr;
}

/* ============================== Char procs =============================== */
const char *cy_char_first_occurence(const char *str, char c)
{
    for (; *str != c; str++) {
        if (*str == '\0')  {
            return NULL;
        }
    }

    return str;
}

const char *cy_char_last_occurence(const char *str, char c)
{
    const char *res = NULL;
    do {
        if (*str == c) {
            res = str;
        }
    } while (*str++ != '\0');

    return res;
}

cy_inline b32 cy_char_is_digit(char c)
{
    return (u8)(c - '0') < 10;
}

cy_inline b32 cy_char_is_hex_digit(char c)
{
    return cy_char_is_digit(c) ||
        CY_IS_IN_RANGE_INCL(c, 'a', 'f') ||
        CY_IS_IN_RANGE_INCL(c, 'A', 'F');
}

cy_inline b32 cy_char_is_lower(char c)
{
    return CY_IS_IN_RANGE_INCL(c, 'a', 'z');
}

cy_inline b32 cy_char_is_upper(char c)
{
    return c - 0x41 < 26;
}

cy_inline b32 cy_char_is_in_set(char c, const char *cut_set)
{
    while (*cut_set != '\0') {
        if (c == *cut_set++) {
            return true;
        }
    }

    return false;
}

cy_inline char cy_char_to_lower(char c) {
    return cy_char_is_upper(c) ? c + ('a' - 'A') : c;
}

cy_inline char cy_char_to_upper(char c) {
    return cy_char_is_lower(c) ? c - ('a' - 'A') : c;
}

cy_inline i64 cy_digit_to_i64(char digit)
{
    return cy_char_is_digit(digit) ? digit - '0' : digit - 'W';
}

cy_inline i64 cy_hex_digit_to_i64(char digit)
{
    if (cy_char_is_digit(digit)) {
        return digit - '0';
    } else if (CY_IS_IN_RANGE_INCL(digit, 'a', 'f')) {
        return 10 + digit - 'a';
    } else if (CY_IS_IN_RANGE_INCL(digit, 'A', 'F')) {
        return 10 + digit - 'A';
    }

    return -1;
}

/* ============================ C-String procs ============================== */
cy_inline isize cy_str_len(const char *str)
{
    if (str == NULL) {
        return 0;
    }

    const char *begin = str;
    while (!CY__IS_WORD_ALIGNED(str)) {
        if (*str == '\0') {
            return str - begin;
        }

        str += 1;
    }

    const isize *w = (const isize*)str;
    while (!CY__HAS_ZERO_BYTE(*w)) {
        w += 1;
    }

    str = (const char*)w;
    while (*str != '\0') {
        str += 1;
    }

    return str - begin;
}

cy_inline isize cy_wcs_len(const wchar_t *str)
{
    if (str == NULL) {
        return 0;
    }

    const wchar_t *begin = str;
    while (!CY__IS_WORD_ALIGNED(str)) {
        if (*str == '\0') {
            return str - begin;
        }

        str += 1;
    }

    const isize *w = (const isize*)str;
    while (!CY__HAS_ZERO_BYTE(*w)) {
        w += 1;
    }

    str = (const wchar_t*)w;
    while (*str != '\0') {
        str += 1;
    }

    return str - begin;
}

cy_inline char *cy_str_copy(char *dst, const char *src)
{
    while (!CY__IS_WORD_ALIGNED(src)) {
        if (*src == '\0') {
            *dst = '\0';
            return dst;
        }

        *dst++ = *src++;
    }

    isize *wd = (isize*)dst;
    const isize *ws = (const isize*)src;
    while (!CY__HAS_ZERO_BYTE(*ws)) {
        *wd++ = *ws++;
    }

    dst = (char*)wd;
    src = (const char*)ws;
    while (*src != '\0') {
        *dst++ = *src++;
    }

    *dst = '\0';
    return dst;
}

cy_inline isize cy_str_compare(const char *a, const char *b) {
    while (*a != '\0' && *a == *b) {
        a += 1, b += 1;
    }

    return *(const u8*)a - *(const u8*)b;
}

cy_inline isize cy_str_compare_n(const char *a, const char *b, isize len)
{
    while (len > 0) {
        if (*a != *b) {
            return *(const u8*)a - *(const u8*)b;
        }

        a += 1, b += 1;
        len -= 1;
    }

    return 0;
}

cy_inline b32 cy_str_has_prefix(const char *str, const char *prefix)
{
    while (*prefix != '\0') {
        if (*str++ != *prefix++) {
            return false;
        }
    }

    return true;
}

cy_inline char *cy_str_reverse(char *str)
{
    char *s = str, *e = str + cy_str_len(str) - 1;
    while (s < e) {
        CY_SWAP(char, *s, *e);
        s += 1, e -= 1;
    }

    return str;
}

cy_inline char *cy_str_reverse_n(char *str, isize len)
{
    char *s = str, *e = str + len - 1;
    while (s < e) {
        CY_SWAP(char, *s, *e);
        s += 1, e -= 1;
    }

    return str;
}

cy_inline void cy_str_to_lower(char *str)
{
    if (str == NULL) {
        return;
    }

    for (; *str != '\0'; str++) {
        *str = cy_char_to_lower(*str);
    }
}

cy_inline void cy_str_to_upper(char *str)
{
    if (str == NULL) {
        return;
    }

    for (; *str != '\0'; str++) {
        *str = cy_char_to_upper(*str);
    }
}

u64 cy_str_to_u64(const char *str, i32 base, isize *len_out)
{
    if (base == 0) {
        base = (cy_str_has_prefix(str, "0x")) ?  16 : 10;
    }

    u64 res;
    isize len = cy__scan_u64(str, base, &res);
    if (len_out != NULL) {
        *len_out = len;
    }

    return res;
}

i64 cy_str_to_i64(const char *str, i32 base, isize *len_out)
{
    if (base == 0) {
        const char *s = str;
        if (*s == '-') {
            s += 1;
        }

        base = (cy_str_has_prefix(s, "0x")) ? 16 : 10;
    }

    i64 res;
    isize len = cy__scan_i64(str, base, &res);
    if (len_out != NULL) {
        *len_out = len;
    }

    return res;
}

// TODO(cya): rework the parsing procedures

isize cy_str_parse_u64(u64 n, i32 base, char *dst)
{
    char *c = dst;
    if (n == 0) {
        *c++ = '0';
    } else {
        while (n > 0) {
            *c++ = cy__num_to_char_table_upper[n % (u64)base];
            n /= (u64)base;
        }
    }

    *c = '\0';
    cy_str_reverse(dst);

    return c - dst;
}

isize cy_str_parse_i64(i64 n, i32 base, char *dst)
{
    b32 negative = false;
    if (n < 0) {
        negative = true;
        n = -n;
    }

    u64 v = (u64)n;
    char *c = dst;
    if (v == 0) {
        *c++ = '0';
    } else {
        while (v > 0) {
            *c++ = cy__num_to_char_table_upper[v % (u64)base];
            v /= (u64)base;
        }
    }

    if (negative) {
        *c++ = '-';
    }

    *c = '\0';
    cy_str_reverse(dst);

    return c - dst;
}

cy_inline f32 cy_str_to_f32(const char *str, isize *len_out)
{
    return (f32)cy_str_to_f64(str, len_out);
}

f64 cy_str_to_f64(const char *str, isize *len_out)
{
    const char *s = str;
    i32 sign = 1;
    if (*s == '-') {
        sign = -sign;
        s += 1;
    } else if (*s == '+') {
        s += 1;
    }

    f64 val;
    for (val = 0.0; cy_char_is_digit(*s); s++) {
        val = val * 10.0 + (*s - '0');
    }

    if (*s == '.') {
        s += 1;
        f64 place = 0.1;
        while (cy_char_is_digit(*s)) {
            val += (*s - '0') * place;
            place *= 0.1;
            s += 1;
        }
    }

    i32 frac = 0;
    f64 scale = 1.0;
    if (*s == 'e' || *s == 'E') {
        s += 1;
        if (*s == '-') {
            frac = 1;
            s += 1;
        } else if (*s == '+') {
            s += 1;
        }

        u32 exp;
        for (exp = 0; cy_char_is_digit(*s); s++) {
            exp = exp * 10 + (u32)(*s - '0');
        }

        // NOTE(cya): faster exponent calc
        while (exp >= 50) {
            scale *= 1e50;
            exp -= 50;
        }
        while (exp >= 8) {
            scale *= 1e8;
            exp -= 8;
        }
        while (exp >= 1) {
            scale *= 10.0;
            exp -= 1;
        }
    }

    f64 res = sign * frac == 0 ? (val * scale) : (val / scale);
    if (len_out != NULL) {
        *len_out = s - str;
    }

    return res;
}

isize cy_str_parse_f32(f32 n, char *dst)
{
    return cy_str_parse_f64((f64)n, dst);
}

typedef struct {
    u32 integral;
    u32 decimal;
    i32 exponent;
} CyPrivFloatParts;

cy_internal cy_inline i16 cy__f64_normalize_decimal(f64 *val)
{
    const f64 exp_threshold_neg = 1e-5;
    const f64 exp_threshold_pos = 1e7;

    i16 exp = 0;
    f64 value = *val;
    if (value > exp_threshold_pos) {
        CY__CALC_BIN_EXPS(INC, 10, 1e, value, exp);
    } else if (value > 0.0 && value <= exp_threshold_neg) {
        CY__CALC_BIN_EXPS(DEC, 10, 1e, value, exp);
    }

    *val = value;
    return exp;
}

cy_internal cy_inline CyPrivFloatParts cy__f64_split(f64 val)
{
    i16 exponent = cy__f64_normalize_decimal(&val);
    u32 integral = (u32)val;
    f64 remainder = (val - (f64)integral) * 1e9;
    u32 decimal = (u32)remainder;

    remainder -= decimal;
    if (remainder > 0.5) {
        decimal += 1;
        if (decimal > 1e9) {
            decimal = 0;
            integral += 1;
            if (exponent != 0 && integral > 9) {
                integral = 1;
                exponent += 1;
            }
        }
    }

    return (CyPrivFloatParts){
        .integral = integral,
        .decimal = decimal,
        .exponent = exponent,
    };
}

cy_internal cy_inline isize cy__f64_parse_decimal(u32 value, char *dst)
{
    isize width = 9;
    while (value % 10 == 0 && width > 0) {
        value /= 10;
        width -= 1;
    }

    char *c = dst;
    while (width-- > 0) {
        *c++ = cy__num_to_char_table_upper[value % 10];
        value /= 10;
    }

    *c = '\0';
    cy_str_reverse(dst);

    return c - dst;
}

// NOTE(cya):
// * this doesn't allow for customizing the formatting of the value
// (parses up to 9 significant decimal digits and normalizes the number)
// * make sure you write this to a buffer with at least 23 bytes of capacity
// (including the null terminator)
isize cy_str_parse_f64(f64 n, char *dst)
{
    char *cur = dst;
    if (CY_IS_NAN(n)) {
        return cy_str_copy(cur, "NaN") - dst;
    }

    if (n < 0.0) {
        *cur++ = '-';
        n = -n;
    }

    if (CY_IS_INF(n)) {
        return cy_str_copy(cur, "Inf") - dst;
    }

    CyPrivFloatParts parts = cy__f64_split(n);
    cur += cy_str_parse_u64(parts.integral, 10, cur);
    if (parts.decimal > 0) {
        *cur++ = '.';
        cur += cy__f64_parse_decimal(parts.decimal, cur);
    }

    if (parts.exponent != 0) {
        *cur++ = 'e';
        cur += cy_str_parse_i64(parts.exponent, 10, cur);
    }

    *cur = '\0';
    return cur - dst;
}

/* ================================ Strings ================================= */
cy_inline isize cy_string_len(CyString str)
{
    return (str == NULL) ? 0 : CY_STRING_HEADER(str)->len;
}

cy_inline isize cy_string_cap(CyString str)
{
    return (str == NULL) ? 0 : CY_STRING_HEADER(str)->cap;
}

cy_inline isize cy_string_alloc_size(CyString str)
{
    return cy_sizeof(CyStringHeader) + cy_string_cap(str) + 1;
}

cy_inline isize cy_string_available_space(CyString str)
{
    CyStringHeader *h = CY_STRING_HEADER(str);
    if (h->cap > h->len) {
        return h->cap - h->len;
    }

    return 0;
}

cy_inline void cy__string_set_len(CyString str, isize len)
{
    if (str == NULL) {
        return;
    }

    CY_STRING_HEADER(str)->len = len;
}

cy_inline void cy__string_set_cap(CyString str, isize cap)
{
    if (str == NULL) {
        return;
    }

    CY_STRING_HEADER(str)->cap = cap;
}

CyString cy_string_create_reserve(CyAllocator a, isize cap)
{
    isize header_size = cy_sizeof(CyStringHeader);
    isize total_size = header_size + cap + 1;
    void *ptr = cy_alloc(a, total_size);
    CY_VALIDATE_PTR(ptr);

    cy_mem_zero(ptr, total_size);

    CyStringHeader *header = ptr;
    *header = (CyStringHeader){
        .alloc = a,
        .len = 0,
        .cap = cap,
    };

    return (CyString)ptr + header_size;
}

CyString cy_string_create_len(CyAllocator a, const char *str, isize len)
{
    isize header_size = cy_sizeof(CyStringHeader);
    isize total_size = header_size + len + 1;
    void *ptr = cy_alloc(a, total_size);
    CY_VALIDATE_PTR(ptr);

    if (str == NULL) {
        cy_mem_zero(ptr, total_size);
    }

    CyStringHeader *header = ptr;
    *header = (CyStringHeader){
        .alloc = a,
        .len = len,
        .cap = len,
    };

    CyString string = (CyString)ptr + header_size;
    if (len > 0 && str != NULL) {
        cy_mem_copy(string, str, len);
    }

    string[len] = '\0';
    return string;
}

cy_inline CyString cy_string_create(CyAllocator a, const char *str)
{
    return cy_string_create_len(a, str, cy_str_len(str));
}

cy_inline CyString cy_string_create_view(CyAllocator a, CyStringView str)
{
    return cy_string_create_len(a, (const char*)str.text, str.len);
}

CyString cy_string_reserve_space_for(CyString str, isize extra_len)
{
    isize available = cy_string_available_space(str);
    if (available >= extra_len) {
        return str;
    }

    void *mem = CY_STRING_HEADER(str);
    CyStringHeader *header = mem;
    CyAllocator a = header->alloc;

    isize new_cap = cy_string_len(str) + extra_len;
    isize old_size = cy_sizeof(*header) + cy_string_cap(str) + 1;
    isize new_size = cy_sizeof(*header) + new_cap + 1;

    void *new_mem = cy_resize(a, mem, old_size, new_size);
    CY_VALIDATE_PTR(new_mem);

    header = new_mem;
    header->alloc = a;

    str = (CyString)(header + 1);
    cy__string_set_cap(str, new_cap);
    return str;
}

CyString cy_string_shrink(CyString str)
{
    CY_VALIDATE_PTR(str);

    isize len = cy_string_len(str), cap = cy_string_cap(str);
    isize header_size = cy_sizeof(CyStringHeader);
    isize size = header_size + cap + 1;
    isize new_size = header_size + len + 1;
    void *ptr = str - header_size;
    if (new_size < size) {
        ptr = cy_resize(CY_STRING_HEADER(str)->alloc, ptr, size, new_size);
        str = (CyString)ptr + header_size;
        cy__string_set_cap(str, len);
    }

    return str;
}

cy_inline void cy_string_free(CyString str)
{
    if (str != NULL) {
        cy_free(CY_STRING_HEADER(str)->alloc, CY_STRING_HEADER(str));
    }
}

CyString cy_string_append_len(CyString str, const char *other, isize len)
{
    if (len > 0) {
        isize cur_len = cy_string_len(str);

        str = cy_string_reserve_space_for(str, len);
        CY_VALIDATE_PTR(str);

        cy_mem_copy(str + cur_len, other, len);

        isize new_len = cur_len + len;
        str[new_len] = '\0';

        cy__string_set_len(str, new_len);
    }

    return str;
}

cy_inline CyString cy_string_append(CyString str, const CyString other)
{
    return cy_string_append_len(str, other, cy_string_len(other));
}

cy_inline CyString cy_string_append_c(CyString str, const char *other)
{
    return cy_string_append_len(str, other, cy_str_len(other));
}

cy_inline CyString cy_string_append_rune(CyString str, Rune r)
{
    // TODO(cya): utf-8 encode rune
    return cy_string_append_len(str, (const char*)&r, 1);
}

CyString cy_string_append_fmt(CyString str, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);

    CyAllocator a = CY_STRING_HEADER(str)->alloc;
    char *buf = cy_aprintf_va(a, fmt, va);

    va_end(va);
    str = cy_string_append_c(str, buf);
    cy_free(a, buf);

    return str;
}

cy_inline CyString cy_string_append_view(CyString str, CyStringView view)
{
    return cy_string_append_len(str, (const char*)view.text, view.len);
}

CyString cy_string_prepend_len(CyString str, const char *other, isize len)
{
    if (len > 0) {
        isize cur_len = cy_string_len(str);

        str = cy_string_reserve_space_for(str, len);
        CY_VALIDATE_PTR(str);

        isize new_len = cur_len + len;
        cy_mem_move(str + len, str, cur_len);
        cy_mem_copy(str, other, len);

        str[new_len] = '\0';
        cy__string_set_len(str, new_len);
    }

    return str;
}

cy_inline CyString cy_string_prepend(CyString str, const CyString other)
{
    return cy_string_prepend_len(str, other, cy_string_len(other));
}

cy_inline CyString cy_string_prepend_c(CyString str, const char *other)
{
    return cy_string_prepend_len(str, other, cy_str_len(other));
}

cy_inline CyString cy_string_prepend_rune(CyString str, Rune r)
{
    // TODO(cya): utf-8 encode rune
    isize width = 1;
    return cy_string_prepend_len(str, (const char*)&r, width);
}

cy_inline CyString cy_string_prepend_fmt(CyString str, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);

    CyAllocator a = CY_STRING_HEADER(str)->alloc;
    char *buf = cy_aprintf_va(a, fmt, va);

    va_end(va);
    str = cy_string_prepend_c(str, buf);
    cy_free(a, buf);

    return str;
}

cy_inline CyString cy_string_prepend_view(CyString str, CyStringView view)
{
    return cy_string_prepend_len(str, (const char*)view.text, view.len);
}

CyString cy_string_pad_right(CyString str, isize width, Rune r)
{
    // TODO(cya): calculate width with utf8_width proc (to be implemented)
    isize str_width = cy_utf8_codepoints(str);
    isize rune_width = 1;
    while (str_width < width) {
        str = cy_string_append_rune(str, r);
        str_width += rune_width;
    }

    return str;
}

CyString cy_string_set(CyString str, const char *c_str)
{
    isize new_len = cy_str_len(c_str);
    if (cy_string_cap(str) < new_len) {
        str = cy_string_reserve_space_for(str, new_len - cy_string_len(str));
        CY_VALIDATE_PTR(str);
    }

    cy_mem_copy(str, c_str, new_len + 1);
    cy__string_set_len(str, new_len);

    return str;
}

CyString cy_string_dup(CyAllocator a, const CyString src)
{
    return cy_string_create_len(a, src, cy_string_len(src));
}

b32 cy_string_are_equal(const CyString a, const CyString b)
{
    isize a_len = cy_string_len(a);
    isize b_len = cy_string_len(b);
    if (a_len != b_len) {
        return false;
    }

    return cy_mem_compare(a, b, a_len) == 0;
}

CyString cy__string_trim_internal(CyString str, const char *cut_set, i32 flags)
{
    char *start, *new_start;
    new_start = start = str;
    char *end, *new_end;
    new_end = end = str + cy_string_len(str) - 1;
    if (flags & CY__STRING_TRIM_LEADING) {
        while (
            new_start <= end &&
                cy_char_first_occurence(cut_set, *new_start) != NULL
        ) {
            new_start += 1;
        }
    }
    if (flags & CY__STRING_TRIM_TRAILING) {
        while (
            new_end > new_start &&
                cy_char_first_occurence(cut_set, *new_end) != NULL
        ) {
            new_end -= 1;
        }
    }

    isize len = (isize)(new_start > new_end ? 0 : new_end - new_start + 1);
    if (str != new_start) {
        cy_mem_move(str, new_start, len);
    }

    str[len] = '\0';
    cy__string_set_len(str, len);
    return str;
}

cy_inline CyString cy_string_trim(CyString str, const char *char_set)
{
    return cy__string_trim_internal(
        str, char_set, CY__STRING_TRIM_LEADING | CY__STRING_TRIM_TRAILING
    );
}

cy_inline CyString cy_string_trim_leading(CyString str, const char *char_set)
{
    return cy__string_trim_internal(str, char_set, CY__STRING_TRIM_LEADING);
}

cy_inline CyString cy_string_trim_trailing(CyString str, const char *char_set)
{
    return cy__string_trim_internal(str, char_set, CY__STRING_TRIM_TRAILING);
}

#define CY__WHITESPACE " \t\r\n\v\f"

cy_inline CyString cy_string_trim_whitespace(CyString str)
{
    return cy_string_trim(str, CY__WHITESPACE);
}

cy_inline CyString cy_string_trim_leading_whitespace(CyString str)
{
    return cy_string_trim_leading(str, CY__WHITESPACE);
}

cy_inline CyString cy_string_trim_trailing_whitespace(CyString str)
{
    return cy_string_trim_trailing(str, CY__WHITESPACE);
}

/* ----------------------------- String views ------------------------------- */
CyStringView cy_string_view_create_len(const char *str, isize len)
{
    if (len < 0) {
        len = cy_str_len(str);
    }

    return (CyStringView){
        .text = (const u8*)str,
        .len = len,
    };
}

CyStringView cy_string_view_create(CyString str)
{
    return cy_string_view_create_len(str, cy_string_len(str));
}

CyStringView cy_string_view_create_c(const char *str)
{
    return cy_string_view_create_len(str, cy_str_len(str));
}

CyStringView cy_string_view_substring(
    CyStringView str, isize begin_idx, isize end_idx
) {
    isize max = str.len, lo = begin_idx, hi = end_idx;
    CY_ASSERT_MSG(lo <= hi && hi <= max, "%td..%td..%td", lo, hi, max);

    return cy_string_view_create_len((const char*)str.text + lo, hi - lo);
}

cy_inline b32 cy_string_view_are_equal(CyStringView a, CyStringView b)
{
    return (a.len == b.len) && (cy_mem_compare(a.text, b.text, a.len) == 0);
}

cy_inline b32 cy_string_view_has_prefix(CyStringView str, const char *prefix)
{
    CyStringView other = cy_string_view_create_c(prefix);
    return cy_string_view_are_equal(
        cy_string_view_create_len((const char*)str.text, other.len), other
    );
}

b32 cy_string_view_contains(CyStringView str, const char *char_set)
{
    isize len = cy_str_len(char_set);
    // TODO(cya): unicode rune support
    for (isize i = 0; i < str.len; i++) {
        for (isize j = 0; j < len; j++) {
            if (str.text[i] == char_set[j]) {
                return true;
            }
        }
    }

    return false;
}

/* =========================== Strings (UTF-16) ============================= */
#if defined(CY_OS_WINDOWS)
#define CY__U16S_TO_BYTES(c) (isize)((c) * cy_sizeof(u16))

cy_inline isize cy_string_16_len(CyString16 str)
{
    return (str == NULL) ? 0 : CY_STRING_HEADER(str)->len;
}

cy_inline isize cy_string_16_cap(CyString16 str)
{
    return (str == NULL) ? 0 : CY_STRING_HEADER(str)->cap;
}

cy_inline isize cy_string_16_alloc_size(CyString16 str)
{
    isize bytes = CY__U16S_TO_BYTES(cy_string_16_cap(str) + 1);
    return cy_sizeof(CyStringHeader) + bytes;
}

cy_inline isize cy_string_16_available_space(CyString16 str)
{
    CyStringHeader *h = CY_STRING_HEADER(str);
    if (h->cap > h->len) {
        return h->cap - h->len;
    }

    return 0;
}

cy_inline void cy__string_16_set_len(CyString16 str, isize len)
{
    if (str == NULL) {
        return;
    }

    CY_STRING_HEADER(str)->len = len;
}

cy_inline void cy__string_16_set_cap(CyString16 str, isize cap)
{
    if (str == NULL) {
        return;
    }

    CY_STRING_HEADER(str)->cap = cap;
}

CyString16 cy_string_16_create_reserve(CyAllocator a, isize cap)
{
    isize header_size = cy_sizeof(CyStringHeader);
    isize total_size = CY__U16S_TO_BYTES(header_size + cap + 1);
    void *ptr = cy_alloc(a, total_size);
    CY_VALIDATE_PTR(ptr);

    cy_mem_zero(ptr, total_size);

    CyStringHeader *header = ptr;
    *header = (CyStringHeader){
        .alloc = a,
        .len = 0,
        .cap = cap,
    };

    return (CyString16)(header + 1);
}

CyString16 cy_string_16_create_len(
    CyAllocator a, const wchar_t *str, isize len
) {
    isize header_size = cy_sizeof(CyStringHeader);
    isize total_size = CY__U16S_TO_BYTES(header_size + len + 1);
    void *ptr = cy_alloc(a, total_size);
    CY_VALIDATE_PTR(ptr);

    if (str == NULL) {
        cy_mem_zero(ptr, total_size);
    }

    CyStringHeader *header = ptr;
    *header = (CyStringHeader){
        .alloc = a,
        .len = len,
        .cap = len,
    };

    CyString16 string = (CyString16)((uintptr)ptr + (uintptr)header_size);
    if (len > 0 && str != NULL) {
        cy_mem_copy(string, str, CY__U16S_TO_BYTES(len));
    }

    string[len] = '\0';
    return string;
}

cy_inline CyString16 cy_string_16_create(CyAllocator a, const wchar_t *str)
{
    return cy_string_16_create_len(a, str, cy_wcs_len(str));
}

cy_inline CyString16 cy_string_16_create_view(CyAllocator a, CyString16View str)
{
    return cy_string_16_create_len(a, (const wchar_t*)str.text, str.len);
}

CyString16 cy_string_16_reserve_space_for(CyString16 str, isize extra_len)
{
    isize available = cy_string_16_available_space(str);
    if (available >= extra_len) {
        return str;
    }

    void *mem = CY_STRING_HEADER(str);
    CyStringHeader *header = mem;
    CyAllocator a = header->alloc;

    isize header_size = cy_sizeof(*header);
    isize old_cap = cy_string_16_cap(str);
    isize new_cap = cy_string_16_len(str) + extra_len;
    isize old_size = header_size + CY__U16S_TO_BYTES(old_cap + 1);
    isize new_size = header_size + CY__U16S_TO_BYTES(new_cap + 1);

    void *new_mem = cy_resize(a, mem, old_size, new_size);
    CY_VALIDATE_PTR(new_mem);

    header = new_mem;
    header->alloc = a;

    str = (CyString16)(header + 1);
    cy__string_16_set_cap(str, new_cap);
    return str;
}

CyString16 cy_string_16_shrink(CyString16 str)
{
    CY_VALIDATE_PTR(str);

    isize len = cy_string_16_len(str), cap = cy_string_16_cap(str);
    isize header_size = cy_sizeof(CyStringHeader);
    isize old_size = header_size + CY__U16S_TO_BYTES(cap + 1);
    isize new_size = header_size + CY__U16S_TO_BYTES(len + 1);
    void *ptr = (u8*)str - header_size;
    if (new_size < old_size) {
        ptr = cy_resize(CY_STRING_HEADER(str)->alloc, ptr, old_size, new_size);
        str = (CyString16)((uintptr)ptr + (uintptr)header_size);
        cy__string_16_set_cap(str, len);
    }

    return str;
}

cy_inline CyString16 cy_string_16_resize(CyString16 str, isize new_cap)
{
    void *mem = CY_STRING_HEADER(str);
    CyStringHeader *header = mem;
    CyAllocator a = header->alloc;

    isize header_size = cy_sizeof(*header);
    isize old_cap = cy_string_16_cap(str);
    isize old_size = header_size + CY__U16S_TO_BYTES(old_cap + 1);
    isize new_size = header_size + CY__U16S_TO_BYTES(new_cap + 1);
    void *new_mem = cy_resize(a, mem, old_size, new_size);
    CY_VALIDATE_PTR(new_mem);

    header = new_mem;
    header->alloc = a;

    str = (CyString16)(header + 1);
    cy__string_16_set_cap(str, new_cap);
    return str;
}

cy_inline void cy_string_16_free(CyString16 str)
{
    if (str != NULL) {
        cy_free(CY_STRING_HEADER(str)->alloc, CY_STRING_HEADER(str));
    }
}

cy_inline void cy_string_16_clear(CyString16 str)
{
    if (str == NULL) {
        return;
    }

    cy_mem_zero(str, CY__U16S_TO_BYTES(cy_string_16_cap(str)));
    cy__string_16_set_len(str, 0);
}

CyString16 cy_string_16_append_len(
    CyString16 str, const wchar_t *other, isize len
) {
    if (len > 0) {
        isize cur_len = cy_string_16_len(str);

        str = cy_string_16_reserve_space_for(str, len);
        CY_VALIDATE_PTR(str);

        cy_mem_copy(str + cur_len, other, CY__U16S_TO_BYTES(len));

        isize new_len = cur_len + len;
        str[new_len] = '\0';

        cy__string_16_set_len(str, new_len);
    }

    return str;
}

cy_inline CyString16 cy_string_16_append(CyString16 str, const CyString16 other)
{
    return cy_string_16_append_len(str, other, cy_string_16_len(other));
}

cy_inline CyString16 cy_string_16_append_c(CyString16 str, const wchar_t *other)
{
    return cy_string_16_append_len(str, other, cy_wcs_len(other));
}

cy_inline CyString16 cy_string_16_append_view(CyString16 str, CyString16View view)
{
    return cy_string_16_append_len(str, (const wchar_t*)view.text, view.len);
}

/* ---------------------------- String16 views ------------------------------ */
cy_inline CyString16View cy_string_16_view_create_len(
    const wchar_t *str, isize len
) {
    if (len < 0) {
        len = cy_wcs_len(str);
    }

    return (CyString16View){
        .text = (const u16*)str,
        .len = len,
    };
}

cy_inline CyString16View cy_string_16_view_create(CyString16 str)
{
    return cy_string_16_view_create_len(str, cy_string_16_len(str));
}

cy_inline CyString16View cy_string_16_view_create_c(const wchar_t *str)
{
    return cy_string_16_view_create_len(str, cy_wcs_len(str));
}

CyString16View cy_string_16_view_substring(
    CyString16View str, isize begin_idx, isize end_idx
) {
    isize max = str.len, lo = begin_idx, hi = end_idx;
    CY_ASSERT_MSG(lo <= hi && hi <= max, "%td..%td..%td", lo, hi, max);

    return cy_string_16_view_create_len((const wchar_t*)str.text + lo, hi - lo);
}

cy_inline b32 cy_string_16_view_are_equal(CyString16View a, CyString16View b)
{
    return (a.len == b.len) && (
        cy_mem_compare(a.text, b.text, CY__U16S_TO_BYTES(a.len)) == 0
    );
}

cy_inline b32 cy_string_16_view_has_prefix(
    CyString16View str, const wchar_t *prefix
) {
    CyString16View other = cy_string_16_view_create_c(prefix);
    return cy_string_16_view_are_equal(
        cy_string_16_view_create_len((const wchar_t*)str.text, other.len), other
    );
}

b32 cy_string_16_view_contains(CyString16View str, const wchar_t *char_set)
{
    isize len = cy_wcs_len(char_set);
    for (isize i = 0; i < str.len; i++) {
        for (isize j = 0; j < len; j++) {
            if (str.text[i] == char_set[j]) {
                return true;
            }
        }
    }

    return false;
}
#endif // CY_OS_WINDOWS

/* ============================ Unicode helpers ============================= */
// TODO(cya): implement
#if 0
cy_global u8 cy__utf8_class_table[32] = {
    0, 0, 0, 0, // 0 = 0xxxx
    0, 0, 0, 0, // 1 = 10xxx
    0, 0, 0, 0, // 2 = 110xx
    0, 0, 0, 0, // 3 = 1110x
    1, 1, 1, 1, // 4 = 11110
    1, 1, 1, 1, // 5 = 11111 (invalid)
    2, 2, 2, 2,
    3, 3, 4, 5,
};

isize cy_utf8_encode_rune(u8 buf[4], Rune r)
{
    return 0;
}

isize cy_utf8_decode(const u8 *str, Rune *codepoint_out)
{
    return 0;
}
#endif

cy_inline isize cy_utf8_codepoints(const char *str)
{
    isize count = 0;
    while (*str != '\0') {
        u8 c = (u8)*str;
        u8 inc = 0;
        if (c < 0x80) {
            inc = 1;
        } else if ((c & 0xE0) == 0xC0) {
            inc = 2;
        } else if ((c & 0xF0) == 0xE0) {
            inc = 3;
        } else if ((c & 0xF8) == 0xF0) {
            inc = 4;
        } else {
            return -1;
        }

        str += inc;
        count += 1;
    }

    return count;
}

#if defined(CY_COMPILER_MSVC)
    #pragma warning(pop)
#elif defined(CY_COMPILER_GCC)
    #pragma GCC diagnostic pop
#endif

#endif // CY_IMPLEMENTATION
