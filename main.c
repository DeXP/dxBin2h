#include <stdio.h>
#include <string.h>


#include "nuklear_cross/nuklear_cross.h"
#include "style.h"

#define NOC_FILE_DIALOG_IMPLEMENTATION
#include "noc_file_dialog.h"



#include "resources/ttf-font.c"
#include "resources/images/gear.png.h"

#include "resources/images/error.png.h"
#include "resources/images/done.png.h"

#include "resources/images/checked.png.h"
#include "resources/images/unchecked.png.h"


#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 190

#define ICON_WIDTH 48

enum dxMessageboxIcon {
    DX_ICON_ERROR,
    DX_ICON_DONE,

    DX_ICON_COUNT
};

const void* icons_data[DX_ICON_COUNT] = {
    error_image,
    done_image
};

int icons_size[DX_ICON_COUNT] = {
    sizeof(error_image),
    sizeof(done_image)
};

typedef struct {
    int active;
    enum dxMessageboxIcon icon;
    char title[256];
    char text[4096];
} dxPopup;

int dxNkMessageBox(dxPopup* p, enum dxMessageboxIcon icon, const char* title, const char* text){
    p->icon = icon;
    strcpy(p->text, text);
    strcpy(p->title, title);
    p->active = nk_true;
    return 1;
}


int getObjNameByFilename(const char fn[], char* objName){
    int st, en, len;
    len = strlen(fn);
    st = len;
    while( (st>0) && (fn[st]!='\\') && (fn[st] != '/')  ) st--;
    if( (fn[st]!='\\') || (fn[st] != '/') ) st++;
    en = st;
    while( (en<=len) && (fn[en]!='.') ) en++;
    strncpy(objName, fn+st, en-st);
    objName[en-st + 1] = 0;
    return en-st;
}

int main(void){
    struct nkc nkcx;
    dxPopup p;
    struct nk_context *ctx;
    struct nk_image nkgear;
    struct nk_image checked;
    struct nk_image unchecked;

    static char fileName[256];
    static int fileNameLen;
    static char objName[256];
    static int objNameLen;
    static char outName[256];
    static int outNameLen;
    float ratio_one[] = {0.2f, 0.8f};
    float ratio_two[] = {0.8f, 0.2f};

    static int remove_r = nk_true;
    static int terminate_a1 = nk_true;
    static int write_size = nk_false;
    static int is_static = nk_false;
    static int is_unsigned = nk_true;
    
    int ii;
    struct nk_image icons_images[DX_ICON_COUNT];
    
    int running = nk_true;
    
    p.active = nk_false;

    strcpy(fileName, "Choose file...");
    fileNameLen = strlen(fileName);


    ctx = nkc_init( &nkcx, "dxBin2h", WINDOW_WIDTH, WINDOW_HEIGHT, NKC_WIN_NORMAL);
    if(!ctx) nkc_rdie("Can't init Nuklear+ library\n");
    nkc_load_font_memory(&nkcx, TTFfont, sizeof(TTFfont), 16, 0);
    nkgear = nkc_load_image_memory(&nkcx, (void*)gear_png, sizeof(gear_png) );
    checked = nkc_load_image_memory(&nkcx, (void*)checked_image, sizeof(checked_image) );
    unchecked = nkc_load_image_memory(&nkcx, (void*)unchecked_image, sizeof(unchecked_image) );

    for(ii=0; ii<DX_ICON_COUNT; ii++)
        icons_images[ii] = nkc_load_image_memory(&nkcx, icons_data[ii], icons_size[ii]);
        
    set_style(ctx, THEME_RED);
    {struct nk_style_toggle *toggle;
        toggle = &ctx->style.checkbox;
        toggle->border = -2; /* cursor must overlap original image */
        toggle->normal          = nk_style_item_image(unchecked);
        toggle->hover           = nk_style_item_image(unchecked);
        toggle->active          = nk_style_item_image(unchecked);
        toggle->cursor_normal   = nk_style_item_image(checked);
        toggle->cursor_hover    = nk_style_item_image(checked);
    }
    ctx->style.window.padding = nk_vec2(5,12);

    while (running)
    {
        union nkc_event e = nkc_poll_events(&nkcx);
        if( (e.type == NKC_EWINDOW) && (e.window.param == NKC_EQUIT) ) running = nk_false;

        /* GUI */
        {
        if (nk_begin(ctx, "MainPanel", nk_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT),
            NK_WINDOW_NO_SCROLLBAR))
        {
            nk_layout_row(ctx, NK_DYNAMIC, 30, 2, ratio_two);
            nk_edit_string(ctx, NK_EDIT_READ_ONLY, fileName, &fileNameLen, sizeof(fileName), nk_filter_default);
            if (nk_button_label(ctx, "Browse")){
                const char* s = noc_file_dialog_open(NOC_FILE_DIALOG_OPEN, NULL, NULL, NULL);
                if( s != NULL ){
                    strcpy(fileName, s);
                    fileNameLen = strlen( fileName );

                    objNameLen = getObjNameByFilename(fileName, objName);
                    strcpy(outName, fileName);
                    strcat(outName, ".h");
                    outNameLen = strlen(outName);
                }
            }
            
            nk_layout_row_dynamic(ctx, 30, 5);
            nk_checkbox_label(ctx, "Remove \\r", &remove_r);
            nk_checkbox_label(ctx, "0x1A Terminate", &terminate_a1);
            nk_checkbox_label(ctx, "Write size", &write_size);
            nk_checkbox_label(ctx, "Unsigned", &is_unsigned);
            nk_checkbox_label(ctx, "Static", &is_static);

            nk_layout_row(ctx, NK_DYNAMIC, 30, 2, ratio_one);
            nk_label(ctx, "Object name:", NK_TEXT_RIGHT);
            nk_edit_string(ctx, NK_EDIT_SIMPLE, objName, &objNameLen, sizeof(objName), nk_filter_default);

            nk_layout_row(ctx, NK_DYNAMIC, 30, 2, ratio_one);
            nk_label(ctx, "Output file:", NK_TEXT_RIGHT);
            nk_edit_string(ctx, NK_EDIT_SIMPLE, outName, &outNameLen, sizeof(outName), nk_filter_default);

            nk_layout_row_dynamic(ctx, 30, 3);
            nk_spacing(ctx, 2);
            /*if (nk_button_label(ctx, "Convert")){*/
            if (nk_button_image_label(ctx, nkgear, "Convert", NK_TEXT_CENTERED)){
                FILE* f;
                long size = 0;
                unsigned char c;
                f = fopen(fileName, "rb");
                if(f){
                    FILE* o;
                    while( !feof(f) ){
                        fscanf(f, "%c", &c);
                        if( terminate_a1 && (c == 0x1A) ) break;
                        if( remove_r && (c == '\r') ) size--;
                        size++;
                    }
                    fseek(f, 0, SEEK_SET);
                    o = fopen(outName, "w+");
                    if(o){
                        if(write_size) fprintf(o, "unsigned long %sSize = %ld;\n", objName, size+1);

                        if( is_static ) fprintf(o, "static ");
                        fprintf(o, "const ");
                        if( is_unsigned ) fprintf(o, "unsigned ");
                        fprintf(o, "char %s[%ld] = {\n", objName, size+1);
                        while( !feof(f) ){
                            fscanf(f, "%c", &c);
                            if( terminate_a1 && (c == 0x1A) ) break;
                            if( !(remove_r && (c == '\r')) ) fprintf(o, "0x%.2X,", c);
                        }
                        fprintf(o, "\n0 };");
                        if(o) fclose(o);
                        dxNkMessageBox(&p, DX_ICON_DONE, "Success", "File written successfully");
                    } else dxNkMessageBox(&p, DX_ICON_ERROR, "Writing error", 
                                                             "Can't open file for writing");
                } else dxNkMessageBox(&p, DX_ICON_ERROR, "Reading error", 
                                                         "Can't open input file. Does file exists?");
                if(f) fclose(f);
            }
            
            if (p.active)
            {
                static struct nk_rect s = {0, 10, WINDOW_WIDTH-10, WINDOW_HEIGHT-20};
                if (nk_popup_begin(ctx, NK_POPUP_STATIC, p.title,  NK_WINDOW_TITLE, s))
                {
                    nk_layout_row_begin(ctx, NK_STATIC, ICON_WIDTH, 3);
                    {
                        nk_layout_row_push(ctx, ICON_WIDTH);
                        nk_image(ctx, icons_images[p.icon] );
                        nk_layout_row_push(ctx, 5);
                        nk_spacing(ctx, 1);
                        nk_layout_row_push(ctx, WINDOW_WIDTH - ICON_WIDTH - 50);
                        nk_label_wrap(ctx, p.text);
                    }
                    nk_layout_row_end(ctx);
                    
                    nk_layout_row_dynamic(ctx, 25, 3);
                    nk_spacing(ctx, 1);
                    if (nk_button_label(ctx, "OK")) {
                        p.active = nk_false;
                        nk_popup_close(ctx);
                    }
                    nk_popup_end(ctx);
                } else p.active = nk_false;
            }
        };
        nk_end(ctx);}
        if( nk_window_is_closed(ctx, "MainPanel") ) break;

        nkc_render(&nkcx, nk_rgb(30,30,30) );
    }
    nkc_shutdown(&nkcx);
    return 0;
}
