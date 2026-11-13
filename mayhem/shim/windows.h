/* Minimal stand-in for the Windows headers so mintty's terminal core builds on Linux. */
#ifndef MAYHEM_SHIM_WINDOWS_H
#define MAYHEM_SHIM_WINDOWS_H
#include <wchar.h>
#include <stdint.h>
#include <uchar.h>
#include <sys/ioctl.h>
#include <sys/time.h>
typedef char16_t WCHAR;
#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
typedef void *HANDLE, *HWND, *HDC, *HBITMAP, *HFONT, *HINSTANCE, *HICON, *HMENU, *HMONITOR, *HBRUSH, *HKEY;
typedef uint32_t DWORD, COLORREF, UINT;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef long LONG;
typedef int BOOL;
typedef uintptr_t WPARAM, UINT_PTR;
typedef intptr_t LPARAM, LRESULT;
#define RGB(r,g,b) ((COLORREF)(((BYTE)(r)) | (((WORD)(BYTE)(g)) << 8) | (((DWORD)(BYTE)(b)) << 16)))
#define GetRValue(c) ((BYTE)(c))
#define GetGValue(c) ((BYTE)((c) >> 8))
#define GetBValue(c) ((BYTE)((c) >> 16))
typedef struct { LONG left, top, right, bottom; } RECT;
typedef struct { LONG x, y; } POINT;
typedef struct { LONG cx, cy; } SIZE;
typedef void *HIMC, *HCURSOR;
typedef unsigned short ATOM;
typedef struct { char lfFaceName[32]; LONG lfHeight, lfWeight; BYTE lfItalic, lfCharSet; } LOGFONT;
typedef struct { DWORD cbSize; RECT rcMonitor, rcWork; DWORD dwFlags; } MONITORINFO;
#endif
