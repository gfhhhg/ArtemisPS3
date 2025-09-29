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
            
            // 对于ASCII字符，使用TTF_to_Bitmap函数，但优化渲染方式
            w = sw;
            h = sh;
            TTF_to_Bitmap(code, bitmap, &w, &h, &y_correction);
            
            if(w > 0 && h > 0) {
                if(color) {
                    // 计算缩放因子
                    float scale_factor = 1.0f;
                    if (h > 0 && w > 0) {
                        float scale_y = (float)sh / (float)h;
                        float scale_x = (float)sw / (float)w;
                        scale_factor = (scale_x < scale_y) ? scale_x : scale_y;
                    }
                    
                    // 创建临时纹理
                    u32 *temp_texture = (u32*)malloc(w * h * 4);
                    if (temp_texture) {
                        for (int row = 0; row < h; row++) {
                            for (int col = 0; col < w; col++) {
                                u8 alpha = bitmap[row * w + col];
                                if (alpha > 0) {
                                    // 计算正确的颜色值
                                    u32 c = color;
                                    c &= 0xFFFFFF00;
                                    c |= alpha;
                                    temp_texture[row * w + col] = c;
                                } else {
                                    temp_texture[row * w + col] = 0;
                                }
                            }
                        }
                        
                        // 批量渲染字符
                        tiny3d_SetPolygon(TINY3D_QUADS);
                        
                        if(bkcolor) {
                            // 绘制背景
                            tiny3d_VertexPos(x, y, 0);
                            tiny3d_VertexColor(bkcolor);
                            tiny3d_VertexPos(x + sw, y, 0);
                            tiny3d_VertexPos(x + sw, y + sh, 0);
                            tiny3d_VertexPos(x, y + sh, 0);
                            tiny3d_End();
                            tiny3d_SetPolygon(TINY3D_QUADS);
                        }
                        
                        // 计算纹理偏移量
                        u32 tex_offset = tiny3d_TextureOffset(temp_texture);
                        
                        // 设置纹理
                        tiny3d_SetTexture(0, tex_offset, w, h, w * 4, TINY3D_TEX_FORMAT_A8R8G8B8, 1);
                        
                        // 居中显示字符
                        int char_x = x + (sw - (int)(w * scale_factor)) / 2;
                        int char_y = y + (sh - (int)(h * scale_factor)) / 2;
                        
                        // 绘制缩放后的字符
                        tiny3d_VertexPos(char_x, char_y, 0);
                        tiny3d_VertexColor(0xFFFFFFFF);
                        tiny3d_VertexTexture(0.0f, 0.0f);
                        
                        tiny3d_VertexPos(char_x + (int)(w * scale_factor), char_y, 0);
                        tiny3d_VertexTexture(1.0f, 0.0f);
                        
                        tiny3d_VertexPos(char_x + (int)(w * scale_factor), char_y + (int)(h * scale_factor), 0);
                        tiny3d_VertexTexture(1.0f, 1.0f);
                        
                        tiny3d_VertexPos(char_x, char_y + (int)(h * scale_factor), 0);
                        tiny3d_VertexTexture(0.0f, 1.0f);
                        
                        tiny3d_End();
                        
                        free(temp_texture);
                    }
                }
                
                x += sw;
            } else {
                x += sw;
            }
        } else {
            // 对于非ASCII字符（中文等），使用TTF_to_Bitmap方法渲染
            w = sw;
            h = sh;
            TTF_to_Bitmap(code, bitmap, &w, &h, &y_correction);
            
            if(w > 0 && h > 0) {
                if(color) {
                    // 计算缩放因子
                    float scale_factor = 1.0f;
                    if (h > 0 && w > 0) {
                        float scale_y = (float)sh / (float)h;
                        float scale_x = (float)(sw * 2) / (float)w; // 中文等宽字符使用双倍宽度
                        scale_factor = (scale_x < scale_y) ? scale_x : scale_y;
                    }
                    
                    // 创建临时纹理
                    u32 *temp_texture = (u32*)malloc(w * h * 4);
                    if (temp_texture) {
                        for (int row = 0; row < h; row++) {
                            for (int col = 0; col < w; col++) {
                                u8 alpha = bitmap[row * w + col];
                                if (alpha > 0) {
                                    // 计算正确的颜色值
                                    u32 c = color;
                                    c &= 0xFFFFFF00;
                                    c |= alpha;
                                    temp_texture[row * w + col] = c;
                                } else {
                                    temp_texture[row * w + col] = 0;
                                }
                            }
                        }
                        
                        // 批量渲染字符
                        tiny3d_SetPolygon(TINY3D_QUADS);
                        
                        if(bkcolor) {
                            // 绘制背景
                            tiny3d_VertexPos(x, y, 0);
                            tiny3d_VertexColor(bkcolor);
                            tiny3d_VertexPos(x + sw * 2, y, 0);
                            tiny3d_VertexPos(x + sw * 2, y + sh, 0);
                            tiny3d_VertexPos(x, y + sh, 0);
                            tiny3d_End();
                            tiny3d_SetPolygon(TINY3D_QUADS);
                        }
                        
                        // 计算纹理偏移量
                        u32 tex_offset = tiny3d_TextureOffset(temp_texture);
                        
                        // 设置纹理
                        tiny3d_SetTexture(0, tex_offset, w, h, w * 4, TINY3D_TEX_FORMAT_A8R8G8B8, 1);
                        
                        // 居中显示字符
                        int char_x = x + (sw * 2 - (int)(w * scale_factor)) / 2;
                        int char_y = y + (sh - (int)(h * scale_factor)) / 2;
                        
                        // 绘制缩放后的字符
                        tiny3d_VertexPos(char_x, char_y, 0);
                        tiny3d_VertexColor(0xFFFFFFFF);
                        tiny3d_VertexTexture(0.0f, 0.0f);
                        
                        tiny3d_VertexPos(char_x + (int)(w * scale_factor), char_y, 0);
                        tiny3d_VertexTexture(1.0f, 0.0f);
                        
                        tiny3d_VertexPos(char_x + (int)(w * scale_factor), char_y + (int)(h * scale_factor), 0);
                        tiny3d_VertexTexture(1.0f, 1.0f);
                        
                        tiny3d_VertexPos(char_x, char_y + (int)(h * scale_factor), 0);
                        tiny3d_VertexTexture(0.0f, 1.0f);
                        
                        tiny3d_End();
                        
                        free(temp_texture);
                    }
                }
                
                x += sw * 2; // 中文等宽字符使用双倍宽度
            } else {
                x += sw * 2; // 中文等宽字符使用双倍宽度
            }
        }
    }
    
    Y_ttf = y;
    
    return x;
}

int width_ttf_string(const char *string, int sw, int sh)
{
    return display_ttf_string(0, 0, string, 0, 0, sw, sh, NULL);
}