/*
 Nuklear Cross - v1.20 - public domain
 no warrenty implied; use at your own risk.
 authored from 2016-2017 by Dmitry Hrabrov and Micha Mettke
*/

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT

#if defined(WIN32)
    #define _WIN32_WINNT 0x0500
    #define WINVER 0x0500
    #include <windows.h>
    #include <commdlg.h>
    #define NOC_FILE_DIALOG_WIN32
    #include "nuklear.h"
    #include "nuklear_drivers/nuklear_gdip.h"
    #define DX_WINTITSIZE 128
#else
    /* OpenGL version. 2 or 3 */
    /*#define DX_USE_OPENGL 3*/

    #if defined(DX_USE_OPENGL) && (DX_USE_OPENGL > 2)
        #include <GL/glew.h>
    #endif
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_opengl.h>
    #if defined(__APPLE__)
        #define NOC_FILE_DIALOG_OSX
    #else
        #define NOC_FILE_DIALOG_GTK
    #endif
    #define NK_INCLUDE_FONT_BAKING
    #define NK_INCLUDE_STANDARD_IO
    /*#define NK_INCLUDE_DEFAULT_FONT*/
    #include "nuklear.h"
    #include "stb_image.h"

    #if defined(DX_USE_OPENGL) && (DX_USE_OPENGL > 2)
        #include "nuklear_drivers/nuklear_sdl_gl3.h"
    #else
        #include "nuklear_drivers/nuklear_sdl_gl2.h"
    #endif
#endif


typedef struct dxNkWinInfo {
#if defined(WIN32)
    GdipFont* font;
    HWND wnd;
    wchar_t winTitle[DX_WINTITSIZE];
    int needs_refresh;
#else
    SDL_Window *win;
    SDL_GLContext glContext;
    int win_width;
    int win_height;
#endif
} dxNkWinInfo;

#if defined(WIN32)
LRESULT CALLBACK WindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);
#endif


int dxNkWinInit(dxNkWinInfo* wi, const char* title, int width, int height);
int dxNkLoadDefaultFont(dxNkWinInfo* wi);
int dxNkLoadFontMem(dxNkWinInfo* wi, struct nk_context *ctx, unsigned char *membuf, int membufSize, int size);
NK_API struct nk_context *dxNkCtxInit(dxNkWinInfo* wi, int width, int height);
struct nk_image dxNkLoadImageFromMem(const void* buf, int bufSize);
int dxNkMessagePeek(dxNkWinInfo* wi, struct nk_context *ctx);
int dxNkRender(dxNkWinInfo* wi);
int dxNkShutdown(dxNkWinInfo* wi);