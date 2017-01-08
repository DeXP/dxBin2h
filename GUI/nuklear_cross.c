/*
 Nuklear Cross - v1.20 - public domain
 no warrenty implied; use at your own risk.
 authored from 2016-2017 by Dmitry Hrabrov and Micha Mettke
*/

#include "nuklear_cross.h"

#define NOC_FILE_DIALOG_IMPLEMENTATION
#include "noc_file_dialog.h"

#define NK_IMPLEMENTATION
#include "nuklear.h"

#if defined(WIN32)
    #define NK_GDIP_IMPLEMENTATION
    #include "nuklear_drivers/nuklear_gdip.h"
#else
    #define STB_IMAGE_IMPLEMENTATION
    #include "stb_image.h"

    #if defined(DX_USE_OPENGL) && (DX_USE_OPENGL > 2)
        #define NK_SDL_GL3_IMPLEMENTATION
        #include "nuklear_drivers/nuklear_sdl_gl3.h"
    #else
        #define NK_SDL_GL2_IMPLEMENTATION
        #include "nuklear_drivers/nuklear_sdl_gl2.h"
    #endif
#endif



#if defined(WIN32)
/* https://msdn.microsoft.com/en-us/library/windows/desktop/ms534038(v=vs.85).aspx */
GpStatus WINGDIPAPI GdipGetDpiX(GpGraphics *graphics, REAL* dpi);

LRESULT CALLBACK WindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam){
    switch (msg){
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    if (nk_gdip_handle_event(wnd, msg, wparam, lparam)) return 0;
    return DefWindowProcW(wnd, msg, wparam, lparam);
}
#endif



struct nk_image dxNkLoadImageFromMem(const void* buf, int bufSize){
    #if defined(WIN32)
        return nk_gdip_load_image_from_memory(buf, bufSize);
    #else
        int x,y,n;
        GLuint tex;
        unsigned char *data = stbi_load_from_memory(buf, bufSize, &x, &y, &n, 0);

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        return nk_image_id((int)tex);
    #endif
}


int dxNkWinInit(dxNkWinInfo* wi, const char* title, int width, int height){
#if defined(WIN32)
    WNDCLASSW wc;
    RECT rect = { 0, 0, width, height };
    DWORD style = WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX;
    DWORD exstyle = WS_EX_APPWINDOW;
    MultiByteToWideChar(CP_ACP, 0, title, strlen(title)+1, wi->winTitle, DX_WINTITSIZE);

    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandleW(0);
    wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(101));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = wi->winTitle;
    RegisterClassW(&wc);
        
    AdjustWindowRectEx(&rect, style, FALSE, exstyle);

    wi->wnd = CreateWindowExW(exstyle, wc.lpszClassName, wi->winTitle,
            style | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
            rect.right - rect.left, rect.bottom - rect.top,
            NULL, NULL, wc.hInstance, NULL);

    wi->needs_refresh = 1;
#else
    /* SDL setup */
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    #if defined(DX_USE_OPENGL)
        #define DX_OPENGL_VERSION DX_USE_OPENGL
    #else
        #define DX_OPENGL_VERSION 2
    #endif
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, DX_OPENGL_VERSION);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, DX_OPENGL_VERSION);
    #if defined(DX_USE_OPENGL) && (DX_USE_OPENGL > 2)
        SDL_GL_SetAttribute (SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
        SDL_GL_SetAttribute (SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    #endif
    wi->win = SDL_CreateWindow(title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height, SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI);
    wi->glContext = SDL_GL_CreateContext(wi->win);
    SDL_GetWindowSize(wi->win, &(wi->win_width), &(wi->win_height));

    #if DX_USE_OPENGL > 2
    glViewport(0, 0, width, height);
    glewExperimental = 1;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to setup GLEW\n");
        exit(1);
    }
    #endif /* OpenGL version */
#endif /* SDL */
    return 1;
}

int dxNkLoadDefaultFont(dxNkWinInfo* wi){
#if defined(WIN32)
    wi->font = nk_gdipfont_create("Arial", 12);
    nk_gdip_set_font(wi->font);
#else
    /* Load Fonts: if none of these are loaded a default font will be used  */
    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 16, 0);*/
    nk_sdl_font_stash_end();
    /*nk_style_set_font(ctx, &roboto->handle);*/
#endif
    return 1;
}

int dxNkLoadFontMem(dxNkWinInfo* wi, struct nk_context *ctx, unsigned char *membuf, int membufSize, int size){
#if defined(WIN32)
    (void)ctx;
    REAL dpi;
    GdipGetDpiX(gdip.memory, &dpi);
    int height = size * 72 / dpi + 1; /* pixels to points convert */
    wi->font = nk_gdipfont_create_mem(membuf, membufSize, height);
    nk_gdip_set_font(wi->font);
#else
    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    struct nk_font *memfont = nk_font_atlas_add_from_memory(atlas, membuf, membufSize, size, 0);
    nk_sdl_font_stash_end();
    nk_style_set_font(ctx, &memfont->handle);
#endif
    return 1;
}

NK_API struct nk_context *dxNkCtxInit(dxNkWinInfo* wi, int width, int height){
#if defined(WIN32)
    /*return nk_gdi_init(wi->font, wi->dc, width, height);*/
    return nk_gdip_init(wi->wnd, width, height);
#else
    (void)width; (void)height;
    return nk_sdl_init(wi->win);
#endif
}

int dxNkMessagePeek(dxNkWinInfo* wi, struct nk_context *ctx){
    int running = 1;
#if defined(WIN32)
    MSG msg;
    /* Input */
    nk_input_begin(ctx);
    if (wi->needs_refresh == 0){
        if (GetMessageW(&msg, NULL, 0, 0) <= 0){
            running = 0;
        } else {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        wi->needs_refresh = 1;
    } else {
        wi->needs_refresh = 0;
    }
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)){
        if (msg.message == WM_QUIT)
            running = 0;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
        wi->needs_refresh = 1;
    }
    nk_input_end(ctx);
#else
    SDL_Event evt;
    nk_input_begin(ctx);
    while (SDL_PollEvent(&evt)) {
        if (evt.type == SDL_QUIT) running = 0;/*goto cleanup*/
        nk_sdl_handle_event(&evt);
    }
    nk_input_end(ctx);
#endif
    return running;
}

int dxNkRender(dxNkWinInfo* wi){
    #if defined(WIN32)
        /*nk_gdi_render(nk_rgb(30,30,30));*/
        nk_gdip_render(NK_ANTI_ALIASING_ON, nk_rgb(30,30,30));
    #else
        float bg[4];
        nk_color_fv(bg, nk_rgb(30,30,30));
        SDL_GetWindowSize(wi->win, &(wi->win_width), &(wi->win_height) );
        glViewport(0, 0, wi->win_width, wi->win_height);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(bg[0], bg[1], bg[2], bg[3]);
        nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
        SDL_GL_SwapWindow(wi->win);
    #endif
    return 1;
}

int dxNkShutdown(dxNkWinInfo* wi){
    #if defined(WIN32)
        nk_gdipfont_del(wi->font);
        nk_gdip_shutdown();
        UnregisterClassW( wi->winTitle, GetModuleHandleW(0) );
    #else
        nk_sdl_shutdown();
        SDL_GL_DeleteContext(wi->glContext);
        SDL_DestroyWindow(wi->win);
        SDL_Quit();
    #endif
    return 1;
}