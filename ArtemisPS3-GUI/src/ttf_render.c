#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <tiny3d.h>
#include "ttf_render.h"
#include "utf8_utils.h"
#include "ttf_fonts.h"

#define MAX_CHARS 1600
#define TTF_UX 30
#define TTF_UY 24

typedef struct ttf_dyn {
    u32 ttf;
    u16 *text;
    u32 r_use;
    u16 y_start;
    u16 width;
    u16 height;
    u16 flags;
} ttf_dyn;

static ttf_dyn ttf_font_datas[MAX_CHARS];
static u32 r_use = 0;

float Y_ttf = 0.0f;
float Z_ttf = 0.0f;

static int Win_X_ttf = 0;
static int Win_Y_ttf = 0;
static int Win_W_ttf = 848;
static int Win_H_ttf = 512;
static u32 Win_flag = 0;

void set_ttf_window(int x, int y, int width, int height, u32 mode)
{
    Win_X_ttf = x;
    Win_Y_ttf = y;
    Win_W_ttf = width;
    Win_H_ttf = height;
    Win_flag = mode;
    Y_ttf = 0.0f;
    Z_ttf = 0.0f;
}

static void DrawBox_ttf(float x, float y, float z, float w, float h, u32 rgba)
{
    tiny3d_SetPolygon(TINY3D_QUADS);
    
    tiny3d_VertexPos(x    , y    , z);
    tiny3d_VertexColor(rgba);
    
    tiny3d_VertexPos(x + w, y    , z);
    
    tiny3d_VertexPos(x + w, y + h, z);
    
    tiny3d_VertexPos(x    , y + h, z);
    
    tiny3d_End();
}

static void DrawTextBox_ttf(float x, float y, float z, float w, float h, u32 rgba, float tx, float ty)
{
    tiny3d_SetPolygon(TINY3D_QUADS);
    
    tiny3d_VertexPos(x    , y    , z);
    tiny3d_VertexColor(rgba);
    tiny3d_VertexTexture(0.0f , 0.0f);
    
    tiny3d_VertexPos(x + w, y    , z);
    tiny3d_VertexTexture(tx, 0.0f);
    
    tiny3d_VertexPos(x + w, y + h, z);
    tiny3d_VertexTexture(tx, ty);
    
    tiny3d_VertexPos(x    , y + h, z);
    tiny3d_VertexTexture(0.0f , ty);
    
    tiny3d_End();
}

vu16 * init_ttf_table(u16 *texture)
{
    int n;
    
    r_use = 0;
    for(n = 0; n < MAX_CHARS; n++) {
        memset(&ttf_font_datas[n], 0, sizeof(ttf_dyn));
        ttf_font_datas[n].text = texture;
        
        texture += 32 * 32;
    }
    
    return texture;
}

void reset_ttf_frame(void)
{
    int n;
    
    for(n = 0; n < MAX_CHARS; n++) {
        ttf_font_datas[n].flags &= 1;
    }
    
    r_use++;
}

int display_ttf_string(int posx, int posy, const char *string, u32 color, u32 bkcolor, int sw, int sh, int (*DrawIcon_cb)(int, int, char))
{
    int l, n, m, ww, ww2;
    u8 colorc;
    u32 ttf_char;
    u8 *ustring = (u8 *) string;
    
    int lenx = 0;
    
    while(*ustring) {
        
        if(posy >= Win_H_ttf) break;
        
        if(*ustring == ' ') {
            posx += sw >> 1;
            ustring++;
            continue;
        }
        
        // UTF-8解码
        if(*ustring & 128) {
            m = 1;
            
            if((*ustring & 0xf8) == 0xf0) { // 4字节
                ttf_char = (u32) (*(ustring++) & 3);
                m = 3;
            } else if((*ustring & 0xE0) == 0xE0) { // 3字节
                ttf_char = (u32) (*(ustring++) & 0xf);
                m = 2;
            } else if((*ustring & 0xE0) == 0xC0) { // 2字节
                ttf_char = (u32) (*(ustring++) & 0x1f);
                m = 1;
            } else {
                ustring++;
                continue;
            } // 错误!
            
            for(n = 0; n < m; n++) {
                if(!*ustring) break; // 错误!
                if((*ustring & 0xc0) != 0x80) break; // 错误!
                ttf_char = (ttf_char << 6) | ((u32) (*(ustring++) & 63));
            }
            
            if((n != m) && !*ustring) break;
            
        } else {
            ttf_char = (u32) *(ustring++);
        }
        
        // 窗口模式处理
        if(Win_flag & WIN_SKIP_LF) {
            if(ttf_char == '\r' || ttf_char == '\n')
                ttf_char = ' ';
        } else {
            if(Win_flag & WIN_DOUBLE_LF) {
                if(ttf_char == '\r') {
                    if(posx > lenx) lenx = posx;
                    posx = 0;
                    continue;
                }
                if(ttf_char == '\n') {
                    posy += sh;
                    continue;
                }
            } else {
                if(ttf_char == '\n') {
                    if(posx > lenx) lenx = posx;
                    posx = 0;
                    posy += sh;
                    continue;
                }
            }
        }
        
        // 图标处理
        if((ttf_char < 32) && DrawIcon_cb) {
            int resx = DrawIcon_cb(posx, posy, (char) ttf_char);
            if(resx > 0) {
                posx += resx;
                continue;
            } else {
                ttf_char = '?';
            }
        }
        
        // 查找或分配字符槽
        if(ttf_char < 128) {
            n = ttf_char;
        } else {
            m = 0;
            int rel = 0;
            
            for(n = 128; n < MAX_CHARS; n++) {
                if(!(ttf_font_datas[n].flags & 1))
                    m = n;
                
                if((ttf_font_datas[n].flags & 3) == 1) {
                    int trel = r_use - ttf_font_datas[n].r_use;
                    if(m == 0) {
                        m = n;
                        rel = trel;
                    } else if(rel > trel) {
                        m = n;
                        rel = trel;
                    }
                }
                if(ttf_font_datas[n].ttf == ttf_char)
                    break;
            }
            
            if(m == 0)
                m = 128;
        }
        
        if(n >= MAX_CHARS) {
            ttf_font_datas[m].flags = 0;
            l = m;
        } else {
            l = n;
        }
        
        u16 *bitmap = ttf_font_datas[l].text;
        
        // 构建字符位图
        if(!(ttf_font_datas[l].flags & 1)) {
            short bw = TTF_UX;
            short bh = TTF_UY;
            short by_correction;
            u8 temp_bitmap[TTF_UX * TTF_UY];
            
            // 使用TTF_to_Bitmap生成字符位图
            TTF_to_Bitmap(ttf_char, temp_bitmap, &bw, &bh, &by_correction);
            
            memset(bitmap, 0, 32 * 32 * 2);
            
            if(bw > 0 && bh > 0 && bw <= 32 && bh <= 32) { // 确保尺寸在有效范围内
                ww = 0;
                ww2 = 0;
                
                int y_correction = TTF_UY - 1 - by_correction;
                if(y_correction < 0)
                    y_correction = 0;
                
                ttf_font_datas[l].flags = 1;
                ttf_font_datas[l].y_start = y_correction;
                ttf_font_datas[l].height = bh;
                ttf_font_datas[l].width = bw;
                ttf_font_datas[l].ttf = ttf_char;
                
                for(n = 0; n < bh; n++) {
                    if(n >= 32)
                        break;
                    for(m = 0; m < bw; m++) {
                        if(m >= 32 || ww + m >= bw * bh) // 防止缓冲区溢出
                            continue;
                        
                        colorc = (u8) temp_bitmap[ww + m];
                        
                        if(colorc)
                            bitmap[m + ww2] = (colorc << 8) | 0xfff;
                    }
                    
                    ww2 += 32;
                    ww += bw;
                }
            } else {
                // 字符无法渲染或尺寸过大
                ttf_font_datas[l].flags = 0;
                ttf_font_datas[l].width = 0;
                ttf_font_datas[l].height = 0;
            }
        }
        
        // 显示字符
        ttf_font_datas[l].flags |= 2; // 正在使用
        ttf_font_datas[l].r_use = r_use;
        
        if((Win_flag & WIN_AUTO_LF) && (posx + (ttf_font_datas[l].width * sw / 32) + 1) > Win_W_ttf) {
            posx = 0;
            posy += sh;
        }
        
        u32 ccolor = color;
        u32 cx = (ttf_font_datas[l].width * sw / 32) + 1;
        
        // 超出窗口则跳过
        if((posx + cx) > Win_W_ttf || (posy + sh) > Win_H_ttf)
            ccolor = 0;
        
        // 只有当字符有效且尺寸合理时才进行渲染
        if(ccolor && bitmap && ttf_font_datas[l].width > 0 && ttf_font_datas[l].height > 0) {
            // 计算纹理坐标，确保在有效范围内
            float tex_width = (float)ttf_font_datas[l].width / 32.0f;
            float tex_height = (float)ttf_font_datas[l].height / 32.0f;
            
            // 限制纹理坐标范围
            if(tex_width > 1.0f) tex_width = 1.0f;
            if(tex_height > 1.0f) tex_height = 1.0f;
            if(tex_width < 0.0f) tex_width = 0.0f;
            if(tex_height < 0.0f) tex_height = 0.0f;
            
            // 设置纹理
            tiny3d_SetTextureWrap(0, tiny3d_TextureOffset(bitmap), 32, 32, 32 * 2,
                TINY3D_TEX_FORMAT_A4R4G4B4, TEXTWRAP_CLAMP, TEXTWRAP_CLAMP, TEXTURE_LINEAR);
            
            // 绘制背景
            if(bkcolor != 0)
                DrawBox_ttf((float)(Win_X_ttf + posx), (float)(Win_Y_ttf + posy) + ((float)ttf_font_datas[l].y_start * sh) * 0.03125f,
                    Z_ttf, (float)sw, (float)sh, bkcolor);
            
            // 绘制文字
            DrawTextBox_ttf((float)(Win_X_ttf + posx), (float)(Win_Y_ttf + posy) + ((float)ttf_font_datas[l].y_start * sh) * 0.03125f,
                Z_ttf, (float)sw, (float)sh, color,
                tex_width, tex_height);
        }
        
        posx += cx;
    }
    
    Y_ttf = (float)posy + sh;
    
    if(posx < lenx)
        posx = lenx;
    return posx;
}

int width_ttf_string(const char *string, int sw, int sh)
{
    return display_ttf_string(0, 0, string, 0, 0, sw, sh, NULL);
}