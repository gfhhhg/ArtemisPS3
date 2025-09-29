#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <tiny3d.h>
#include "ttf_render.h"
#include "utf8_utils.h"
#include "ttf_fonts.h"

float Y_ttf = 0.0f;
float Z_ttf = 0.0f;

#define MAX_TTF_TEXTURES 256
#define MAX_CHARS_PER_TTF_TEXTURE 16

u16 *ttf_table[MAX_TTF_TEXTURES];
short ttf_width[MAX_TTF_TEXTURES];
short ttf_height[MAX_TTF_TEXTURES];
short ttf_char_in_texture[MAX_TTF_TEXTURES][MAX_CHARS_PER_TTF_TEXTURE];
short ttf_char_in_texture_size[MAX_TTF_TEXTURES];

int ttf_defined_textures;
int ttf_current_frame;

int window_x, window_y, window_w, window_h, window_mode;

void reset_ttf_frame(void)
{
    ttf_current_frame ^= 1;
    if(!ttf_current_frame) {
        int i;
        for(i = 0; i < ttf_defined_textures; i++)
            if(ttf_table[i])
                memset(ttf_table[i], 0, ttf_width[i] * ttf_height[i]);
    }
}

vu16 * init_ttf_table(u16 *texture)
{
    int i;
    
    for(i = 0; i < MAX_TTF_TEXTURES; i++) {
        ttf_table[i] = NULL;
        ttf_char_in_texture_size[i] = 0;
    }
    
    ttf_defined_textures = 0;
    ttf_current_frame = 0;
    
    return (vu16*)texture;
}

void set_ttf_window(int x, int y, int width, int height, u32 mode)
{
    window_x = x;
    window_y = y;
    window_w = width;
    window_h = height;
    window_mode = mode;
}

int display_ttf_string(int posx, int posy, const char *string, u32 color, u32 bkcolor, int sw, int sh, int (*DrawIcon)(int, int, char))
{
    int x = posx;
    int y = posy;
    const uint8_t *p = (const uint8_t *)string;
    
    u8 bitmap[256 * 256];
    short w, h, y_correction;
    
    while (*p)
    {
        uint32_t code = utf8_decode(&p);
        
        if(code == 0x0A) { // 换行符
            if(window_mode & WIN_DOUBLE_LF) y += (sh * 2);
            else y += sh;
            x = posx;
            continue;
        }
        
        if(code == 0x0D) continue; // 回车符，跳过
        
        if(code < 128) { // ASCII字符
            if(code == 0x20) { // 空格
                x += sw / 2;
                continue;
            }
            
            if(DrawIcon && DrawIcon(x, y, code)) {
                x += sw;
                continue;
            }
        }
        
        w = sw;
        h = sh;
        TTF_to_Bitmap(code, bitmap, &w, &h, &y_correction);
        
        if(w > 0 && h > 0) {
            if(color) {
                tiny3d_SetPolygon(TINY3D_QUADS);
                
                if(bkcolor) {
                    tiny3d_VertexPos(x, y, 0);
                    tiny3d_VertexColor(bkcolor);
                    tiny3d_VertexPos(x + w, y, 0);
                    tiny3d_VertexPos(x + w, y + h, 0);
                    tiny3d_VertexPos(x, y + h, 0);
                    tiny3d_End();
                    tiny3d_SetPolygon(TINY3D_QUADS);
                }
                
                y_correction = (sh / 2) - (h / 2);
                
                int row, col;
                for(row = 0; row < h; row++) {
                    for(col = 0; col < w; col++) {
                        u8 alpha = bitmap[row * w + col];
                        if(alpha > 0) {
                            u32 c = color;
                            c &= 0xFFFFFF00;
                            c |= alpha;
                            
                            tiny3d_VertexPos(x + col, y + row + y_correction, 0);
                            tiny3d_VertexColor(c);
                            tiny3d_VertexPos(x + col + 1, y + row + y_correction, 0);
                            tiny3d_VertexPos(x + col + 1, y + row + y_correction + 1, 0);
                            tiny3d_VertexPos(x + col, y + row + y_correction + 1, 0);
                        }
                    }
                }
                
                tiny3d_End();
            }
            
            x += w;
        } else {
            x += sw;
        }
    }
    
    Y_ttf = y;
    
    return x;
}

int width_ttf_string(const char *string, int sw, int sh)
{
    return display_ttf_string(0, 0, string, 0, 0, sw, sh, NULL);
}