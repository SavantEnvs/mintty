/* Force-included before std.h: mintty's `wchar` is UTF-16 (Cygwin), but glibc's wchar_t is 32-bit,
   so route the wide-string libc calls mintty uses to 16-bit implementations (mayhem/shim/mt16.c). */
#ifndef MT16_H
#define MT16_H
#include <stddef.h>
#include <stdlib.h>
#include <wctype.h>
#include <string.h>
#include <wchar.h>
#include <uchar.h>
size_t mt_wcslen(const char16_t *);
size_t mt_wcsnlen(const char16_t *, size_t);
char16_t *mt_wcschr(const char16_t *, char16_t);
char16_t *mt_wcsrchr(const char16_t *, char16_t);
char16_t *mt_wcsncpy(char16_t *, const char16_t *, size_t);
char16_t *mt_wcscpy(char16_t *, const char16_t *);
char16_t *mt_wcsncat(char16_t *, const char16_t *, size_t);
int mt_wcsncmp(const char16_t *, const char16_t *, size_t);
int mt_wcscmp(const char16_t *, const char16_t *);
char16_t *mt_wcsdup(const char16_t *);
int mt_wcswidth(const char16_t *, size_t);
int mt_wcwidth(unsigned);
size_t mt_mbstowcs(char16_t *, const char *, size_t);
size_t mt_mbrtowc(char16_t *, const char *, size_t, mbstate_t *);
#define wcslen mt_wcslen
#define wcsnlen mt_wcsnlen
#define wcschr mt_wcschr
#define wcsrchr mt_wcsrchr
#define wcsncpy mt_wcsncpy
#define wcscpy mt_wcscpy
#define wcsncat mt_wcsncat
#define wcsncmp mt_wcsncmp
#define wcscmp mt_wcscmp
#define wcsdup mt_wcsdup
#define wcswidth mt_wcswidth
#define wcwidth mt_wcwidth
#define mbstowcs mt_mbstowcs
#define mbrtowc mt_mbrtowc
#endif
