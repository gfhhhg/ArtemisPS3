/* 
   TINY3D - font library / (c) 2010 Hermes  <www.elotrolado.net>

*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <pngdec/pngdec.h>

#include "libfont.h"
#include "menu.h"
#include "utf8_utils.h"
#include "ttf_fonts.h"
#include "ttf_render.h"

// 定义字体索引常量
#define font_source_han_sans 3  // 思源黑体是第四个加载的字体，索引为3

struct t_font_description
{
    int w, h, bh;
    
    u8 first_char;
    u8 last_char;
    
    u32 rsx_text_offset;
    u32 rsx_bytes_per_char; 
    u32 color_format;

    short fw[256]; // chr width
    short fy[256]; // chr y correction
};

static struct t_font_datas
{

    int number_of_fonts;

    int current_font;

    struct t_font_description fonts[8];

    int sx, sy;
	int mono;
	int extra;

    u32 color, bkcolor;

    int align; //0 = left, 1 = center, 2 = right
    int autonewline;

    float X,Y,Z;

} font_datas;

typedef struct t_special_char
{
	char value;

	short fw;
	short fy;
	float sx;
	float sy;

	png_texture image;
} special_char;

static special_char special_chars[MAX_SPECIAL_CHARS];
static int special_char_index = 0;


special_char* GetSpecialCharFromValue(const char value)
{
	int x;
	special_char* ret = NULL;

	for (x = 0; x < special_char_index; x++)
	{
		if (special_chars[x].value == value)
			ret = &(special_chars[x]);
	}
	return ret;
}

void ResetFont()
{
    font_datas.current_font = font_datas.number_of_fonts =0;

    font_datas.color = 0xffffffff;
    font_datas.bkcolor = 0;
    font_datas.align = 0;
    font_datas.X = font_datas.Y = font_datas.Z = 0.0f;
    font_datas.autonewline = 0;

    font_datas.sx = font_datas.sy = 8;
	font_datas.mono = 0;
}

u8 * AddFontFromBitmapArray(u8 *font, u8 *texture, u8 first_char, u8 last_char, int w, int h, int bits_per_pixel, int byte_order)
{
    int n, a, b;
    u8 i;
    
    if(font_datas.number_of_fonts >= 8) return texture;

    font_datas.fonts[font_datas.number_of_fonts].w = w;
    font_datas.fonts[font_datas.number_of_fonts].h = h;
    font_datas.fonts[font_datas.number_of_fonts].bh = h;
    font_datas.fonts[font_datas.number_of_fonts].color_format = TINY3D_TEX_FORMAT_A4R4G4B4; //TINY3D_TEX_FORMAT_A8R8G8B8;
    font_datas.fonts[font_datas.number_of_fonts].first_char = first_char;
    font_datas.fonts[font_datas.number_of_fonts].last_char  = last_char;
    font_datas.align =0;

    font_datas.color = 0xffffffff;
    font_datas.bkcolor = 0x0;

    font_datas.sx = w;
    font_datas.sy = h;

    font_datas.Z = 0.0f;

    for(n = 0; n < 256; n++) {
        font_datas.fonts[font_datas.number_of_fonts].fw[n] = 0; 
        font_datas.fonts[font_datas.number_of_fonts].fy[n] = 0;
    }

       
    for(n = first_char; n <= last_char; n++) {

        font_datas.fonts[font_datas.number_of_fonts].fw[n] = w;

        texture = (u8 *) ((((long) texture) + 15) & ~15);

        if(n == first_char) font_datas.fonts[font_datas.number_of_fonts].rsx_text_offset = tiny3d_TextureOffset(texture);

        if(n == first_char+1) font_datas.fonts[font_datas.number_of_fonts].rsx_bytes_per_char = tiny3d_TextureOffset(texture)
            - font_datas.fonts[font_datas.number_of_fonts].rsx_text_offset;

        for(a = 0; a < h; a++) {
            for(b = 0; b < w; b++) {
                
                i = font[(b * bits_per_pixel)/8];

                if(byte_order) 
                    i= (i << ((b & (7/bits_per_pixel)) * bits_per_pixel))>> (8-bits_per_pixel);
                else
                    i >>= (b & (7/bits_per_pixel)) * bits_per_pixel;
                
                i = (i & ((1 << bits_per_pixel)-1)) * 255 / ((1 << bits_per_pixel)-1);

                if(i) {//TINY3D_TEX_FORMAT_A1R5G5B5
                    //i>>=3;
                    //*((u16 *) texture) = (1<<15) | (i<<10) | (i<<5) | (i);
                    //TINY3D_TEX_FORMAT_A4R4G4B4
                    i>>=4;
                    *((u16 *) texture) = (i<<12) | 0xfff;

                } else {
              
                    texture[0] = texture[1] = 0x0; //texture[2] = 0x0;
                    //texture[3] = 0x0; // alpha
                } 
                texture+=2;
               
            }

            font += (w * bits_per_pixel) / 8;
                
        }
    
    }

    texture = (u8 *) ((((long) texture) + 15) & ~15);

    font_datas.number_of_fonts++;

    return texture;
}

u8 * AddFontFromTTF(u8 *texture, u8 first_char, u8 last_char, int w, int h, 
    void (* ttf_callback) (u8 chr, u8 * bitmap, short *w, short *h, short *y_correction))
{
    int n, a, b;
    u8 i;
    u8 *font;
    static u8 letter_bitmap[257 * 256];
    
    int bits_per_pixel = 8;
    
    if(font_datas.number_of_fonts >= 8) return texture;

    if(h < 8) h = 8;
    if(w < 8) w = 8;
    if(h > 256) h = 256;
    if(w > 256) w = 256;

    font_datas.fonts[font_datas.number_of_fonts].w = w;
    font_datas.fonts[font_datas.number_of_fonts].h = h;
    font_datas.fonts[font_datas.number_of_fonts].bh = h+4;
    font_datas.fonts[font_datas.number_of_fonts].color_format = TINY3D_TEX_FORMAT_A4R4G4B4;
    font_datas.fonts[font_datas.number_of_fonts].first_char = first_char;
    font_datas.fonts[font_datas.number_of_fonts].last_char  = last_char;
    font_datas.align =0;

    font_datas.color = 0xffffffff;
    font_datas.bkcolor = 0x0;

    font_datas.sx = w;
    font_datas.sy = h;

    font_datas.Z = 0.0f;

    for(n = 0; n < 256; n++) {
        font_datas.fonts[font_datas.number_of_fonts].fw[n] = 0; 
        font_datas.fonts[font_datas.number_of_fonts].fy[n] = 0;
    }

       
    for(n = first_char; n <= last_char; n++) {
        
        short hh = h;

        font = letter_bitmap;

        font_datas.fonts[font_datas.number_of_fonts].fw[n] = (short) w;
		
		ttf_callback((uint32_t)n, letter_bitmap, &font_datas.fonts[font_datas.number_of_fonts].fw[n], &hh,  &font_datas.fonts[font_datas.number_of_fonts].fy[n]);

        // letter background correction
        if((hh + font_datas.fonts[font_datas.number_of_fonts].fy[n]) > font_datas.fonts[font_datas.number_of_fonts].bh) 
            font_datas.fonts[font_datas.number_of_fonts].bh = hh + font_datas.fonts[font_datas.number_of_fonts].fy[n];

        texture = (u8 *) ((((long) texture) + 15) & ~15);

        if(n == first_char) font_datas.fonts[font_datas.number_of_fonts].rsx_text_offset = tiny3d_TextureOffset(texture);

        if(n == first_char+1) font_datas.fonts[font_datas.number_of_fonts].rsx_bytes_per_char = tiny3d_TextureOffset(texture)
            - font_datas.fonts[font_datas.number_of_fonts].rsx_text_offset;

        for(a = 0; a < h; a++) {
            for(b = 0; b < w; b++) {
                
                i = font[(b * bits_per_pixel)/8];

                i >>= (b & (7/bits_per_pixel)) * bits_per_pixel;
                
                i = (i & ((1 << bits_per_pixel)-1)) * 255 / ((1 << bits_per_pixel)-1);

                if(i) {//TINY3D_TEX_FORMAT_A4R4G4B4
                    i>>=4;
                    *((u16 *) texture) = (i<<12) | 0xfff;
                } else {
              
                    texture[0] = texture[1] = 0x0; //texture[2] = 0x0;
                    //texture[3] = 0x0; // alpha
                } 
                texture+=2;
               
            }

            font += (w * bits_per_pixel) / 8;
                
        }
    
    }

    texture = (u8 *) ((((long) texture) + 15) & ~15);

    font_datas.number_of_fonts++;

    return texture;
}

void SetCurrentFont(int nfont)
{
    if(nfont < 0 || nfont >= font_datas.number_of_fonts) nfont = 0;

    font_datas.current_font = nfont;
}

void SetFontSize(int sx, int sy)
{
    if(sx < 8) sx = 8;
    if(sy < 8) sy = 8;

    font_datas.sx = sx;
    font_datas.sy = sy;
}

void SetFontColor(u32 color, u32 bkcolor)
{
    font_datas.color   = color;
    font_datas.bkcolor = bkcolor;
}

void SetFontAlign(int mode)
{
    font_datas.align  = mode;
    font_datas.autonewline = 0;
}

void SetFontAutoNewLine(int width)
{
    font_datas.autonewline = width;
    font_datas.align  = 0;
}

void SetFontZ(float z)
{
    font_datas.Z  = z;
}

float GetFontX()
{
    return font_datas.X;
}

float GetFontY()
{
    return font_datas.Y;
}

void SetMonoSpace(int space)
{
	font_datas.mono = space;
}

void SetExtraSpace(int space)
{
	font_datas.extra = space;
}

void RegisterSpecialCharacter(char value, short fw, short fy, float sx, float sy, png_texture image)
{
	special_char chr;
	chr.value = value;
	chr.fw = fw;
	chr.fy = fy;
	chr.sx = sx;
	chr.sy = sy;
	chr.image = image;

	// Verify special character
	if (chr.value == 0)
		return;
	if (chr.image.texture_off == 0)
		return;
	if (chr.image.size == 0)
		return;
	if (chr.image.texture.width == 0 || chr.image.texture.height == 0)
		return;
	
	// Verify value is not in use
	if (GetSpecialCharFromValue(chr.value))
		return;

	// Verify room in array
	if ((special_char_index + 1) < MAX_SPECIAL_CHARS)
	{
		special_chars[special_char_index] = chr;
		special_char_index++;
	}

}

int DrawCharSpecial(float x, float y, float z, const special_char* schr, uint8_t draw)
{
    float dx = (float)font_datas.sx * schr->sx;
    float dy = (float)font_datas.sy * schr->sy;

    if (!draw)
        return (int)dx;

    if (schr->fy)
        y += (float)((schr->fy * font_datas.sy) / (float)font_datas.fonts[font_datas.current_font].h);
    else
        y += ((float)font_datas.sy - dy) / 2;

    // Load sprite texture
    tiny3d_SetTexture(0, schr->image.texture_off, schr->image.texture.width,
        schr->image.texture.height, schr->image.texture.pitch,
        TINY3D_TEX_FORMAT_A8R8G8B8, 1);

    tiny3d_SetPolygon(TINY3D_QUADS);

    tiny3d_VertexPos(x, y, z);
    tiny3d_VertexColor(font_datas.color);
    tiny3d_VertexTexture(0.0f, 0.0f);

    tiny3d_VertexPos(x + dx, y, z);
    tiny3d_VertexTexture(0.999f, 0.0f);

    tiny3d_VertexPos(x + dx, y + dy + 1, z);
    tiny3d_VertexTexture(0.999f, 0.999f);

    tiny3d_VertexPos(x, y + dy + 1, z);
    tiny3d_VertexTexture(0.0f, 0.999f);

    tiny3d_End();

    return (int)dx;
}

int WidthFromStr(u8 * str)
{
    int w = 0;

    while(*str) {
		special_char* schr = GetSpecialCharFromValue(*str);
		if (schr)
			w += ((font_datas.sx * schr->sx) + font_datas.extra) * schr->fw / (float)font_datas.fonts[font_datas.current_font].w;
		else
			w += (font_datas.sx + font_datas.extra) * font_datas.fonts[font_datas.current_font].fw[*str] / (float)font_datas.fonts[font_datas.current_font].w;
		str++;
    }

    return w;
}

int WidthFromStrMono(u8 * str)
{
    int w = 0;

    while(*str) {
		w += (font_datas.sx * font_datas.mono) / font_datas.fonts[font_datas.current_font].w;
		str++;
    }

    return w;
}

void DrawCharMono(float x, float y, float z, u8 chr)
{
	special_char* schr = GetSpecialCharFromValue(chr);
	if (schr)
	{
		DrawCharSpecial(x, y, z, schr, 1);
		return;
	}

	float dx = font_datas.sx, dy = font_datas.sy;
	float dx2 = (dx * font_datas.mono) / font_datas.fonts[font_datas.current_font].w;
	float dy2 = (float)(dy * font_datas.fonts[font_datas.current_font].bh) / (float)font_datas.fonts[font_datas.current_font].h;

	if (font_datas.number_of_fonts <= 0) return;

	if (chr < font_datas.fonts[font_datas.current_font].first_char) return;

	if (font_datas.bkcolor) {
		tiny3d_SetPolygon(TINY3D_QUADS);

		tiny3d_VertexPos(x, y, z);
		tiny3d_VertexColor(font_datas.bkcolor);

		tiny3d_VertexPos(x + dx2, y, z);

		tiny3d_VertexPos(x + dx2, y + dy2, z);

		tiny3d_VertexPos(x, y + dy2, z);

		tiny3d_End();
	}

	y += (float)(font_datas.fonts[font_datas.current_font].fy[chr] * font_datas.sy) / (float)(font_datas.fonts[font_datas.current_font].h);

	if (chr > font_datas.fonts[font_datas.current_font].last_char) return;

	// Load sprite texture
	tiny3d_SetTexture(0, font_datas.fonts[font_datas.current_font].rsx_text_offset + font_datas.fonts[font_datas.current_font].rsx_bytes_per_char
		* (chr - font_datas.fonts[font_datas.current_font].first_char), font_datas.fonts[font_datas.current_font].w,
		font_datas.fonts[font_datas.current_font].h, font_datas.fonts[font_datas.current_font].w *
		((font_datas.fonts[font_datas.current_font].color_format == TINY3D_TEX_FORMAT_A8R8G8B8) ? 4 : 2),
		font_datas.fonts[font_datas.current_font].color_format, 1);

	tiny3d_SetPolygon(TINY3D_QUADS);

	tiny3d_VertexPos(x, y, z);
	tiny3d_VertexColor(font_datas.color);
	tiny3d_VertexTexture(0.0f, 0.0f);

	tiny3d_VertexPos(x + dx, y, z);
	tiny3d_VertexTexture(0.95f, 0.0f);

	tiny3d_VertexPos(x + dx, y + dy, z);
	tiny3d_VertexTexture(0.95f, 0.95f);

	tiny3d_VertexPos(x, y + dy, z);
	tiny3d_VertexTexture(0.0f, 0.95f);

	tiny3d_End();
}

void DrawChar(float x, float y, float z, u8 chr)
{
	special_char* schr = GetSpecialCharFromValue(chr);
	if (schr)
	{
		DrawCharSpecial(x, y, z, schr);
		return;
	}

	float dx = font_datas.sx, dy = font_datas.sy;
	float dx2 = (dx * font_datas.fonts[font_datas.current_font].fw[chr]) / font_datas.fonts[font_datas.current_font].w;
	float dy2 = (float)(dy * font_datas.fonts[font_datas.current_font].bh) / (float)font_datas.fonts[font_datas.current_font].h;

	if (font_datas.number_of_fonts <= 0) return;

	if (chr < font_datas.fonts[font_datas.current_font].first_char) return;

	if (font_datas.bkcolor) {
		tiny3d_SetPolygon(TINY3D_QUADS);

		tiny3d_VertexPos(x, y, z);
		tiny3d_VertexColor(font_datas.bkcolor);

		tiny3d_VertexPos(x + dx2, y, z);

		tiny3d_VertexPos(x + dx2, y + dy2, z);

		tiny3d_VertexPos(x, y + dy2, z);

		tiny3d_End();
	}

	y += (float)(font_datas.fonts[font_datas.current_font].fy[chr] * font_datas.sy) / (float)(font_datas.fonts[font_datas.current_font].h);

	if (chr > font_datas.fonts[font_datas.current_font].last_char) return;

	// Load sprite texture
	tiny3d_SetTexture(0, font_datas.fonts[font_datas.current_font].rsx_text_offset + font_datas.fonts[font_datas.current_font].rsx_bytes_per_char
		* (chr - font_datas.fonts[font_datas.current_font].first_char), font_datas.fonts[font_datas.current_font].w,
		font_datas.fonts[font_datas.current_font].h, font_datas.fonts[font_datas.current_font].w *
		((font_datas.fonts[font_datas.current_font].color_format == TINY3D_TEX_FORMAT_A8R8G8B8) ? 4 : 2),
		font_datas.fonts[font_datas.current_font].color_format, 1);

	tiny3d_SetPolygon(TINY3D_QUADS);

	tiny3d_VertexPos(x, y, z);
	tiny3d_VertexColor(font_datas.color);
	tiny3d_VertexTexture(0.0f, 0.0f);

	tiny3d_VertexPos(x + dx, y, z);
	tiny3d_VertexTexture(0.95f, 0.0f);

	tiny3d_VertexPos(x + dx, y + dy, z);
	tiny3d_VertexTexture(0.95f, 0.95f);

	tiny3d_VertexPos(x, y + dy, z);
	tiny3d_VertexTexture(0.0f, 0.95f);

	tiny3d_End();
}

static int i_must_break_line(char *str, float x)
{
    int xx =0;
	int dx = (font_datas.sx+font_datas.extra);
	
    while(*str) {
        if(((u8)*str) <= 32) break;
        xx += dx * font_datas.fonts[font_datas.current_font].fw[((u8)*str)] / font_datas.fonts[font_datas.current_font].w;
        str++;
    }

    
    if(*str && (x+xx) >= font_datas.autonewline) return 1;

    return 0;
}

int skip_icon(int x, int y, char c)
{
	special_char* schr = GetSpecialCharFromValue(c);
	if (schr)
		return DrawCharSpecial(x, y, font_datas.Z, schr, 0);

	else return 0;
}

int draw_icon(int x, int y, char c)
{
	special_char* schr = GetSpecialCharFromValue(c);
	if (schr)
		return DrawCharSpecial(x, y, font_datas.Z, schr, 1);

	else return 0;
}

float DrawStringMono(float x, float y, char *str)
{
	if (!font_datas.mono)
		return DrawString(x, y, str);
	
	float initX = x;
	int dx = font_datas.sx;
	
    if(font_datas.align == 1) {
    
        x= (848 - WidthFromStrMono((u8 *) str)) / 2;

    }
	else if (font_datas.align == 2) {
		x -= WidthFromStrMono((u8 *) str);
	}
	else if (font_datas.align == 3) {
		x -= WidthFromStrMono((u8 *) str)/2;
	}

    while (*str) {
        
        if(*str == '\n') {
            x = initX; 
            y += font_datas.sy * font_datas.fonts[font_datas.current_font].bh / font_datas.fonts[font_datas.current_font].h;
            str++;
            continue;
        } else {
            if(font_datas.autonewline && i_must_break_line(str, x)) {
                x = initX; 
                y += font_datas.sy * font_datas.fonts[font_datas.current_font].bh / font_datas.fonts[font_datas.current_font].h;
            }
        }

        DrawChar(x, y, font_datas.Z, (u8) *str);
		x += (dx * font_datas.mono) / font_datas.fonts[font_datas.current_font].w;
        str++; 
    }

    font_datas.X = x; font_datas.Y = y;

    return x;
}

// UTF-8字符串宽度测量函数
int WidthFromUTF8(const char *str) {
    if (!str) return 0;
    
    int total_width = 0;
    const uint8_t *p = (const uint8_t *)str;
    
    while (*p) {
        uint32_t code = utf8_decode(&p);
        float dx = font_datas.sx + font_datas.extra;
        
        if (code < 128) {
            // ASCII字符
            special_char* schr = GetSpecialCharFromValue((u8)code);
            if (schr) {
                total_width += (font_datas.sx * schr->sx) * schr->fw / (float)font_datas.fonts[font_datas.current_font].w;
            } else {
                if (code >= font_datas.fonts[font_datas.current_font].first_char && 
                    code <= font_datas.fonts[font_datas.current_font].last_char) {
                    total_width += dx * font_datas.fonts[font_datas.current_font].fw[(u8)code] / 
                                font_datas.fonts[font_datas.current_font].w;
                } else {
                    // 未知字符，使用默认宽度
                    total_width += dx;
                }
            }
        } else {
            // 非ASCII字符（中文等），这里假设我们使用的字体支持这些字符
            // 对于中文字符，我们使用双倍宽度
            total_width += (font_datas.sx + font_datas.extra) * 2;
        }
    }
    
    return total_width;
}

// UTF-8字符串绘制函数
float DrawUTF8String(float x, float y, const char *str) {
    int dx = (font_datas.sx + font_datas.extra);
    float initX = x;
    
    if (font_datas.align == 1) {
        x = (848 - WidthFromUTF8(str)) / 2;
    } else if (font_datas.align == 2) {
        x -= WidthFromUTF8(str);
    } else if (font_datas.align == 3) {
        x -= WidthFromUTF8(str) / 2;
    }
    
    const uint8_t *p = (const uint8_t *)str;
    
    // 保存当前字体设置
    u32 current_color = font_datas.color;
    int current_font = font_datas.current_font;
    
    while (*p) {
        uint32_t code = utf8_decode(&p);
        
        if (code == '\n') {
            x = initX;
            y += font_datas.sy * font_datas.fonts[font_datas.current_font].bh / 
                 font_datas.fonts[font_datas.current_font].h;
            continue;
        }
        
        if (code < 128) {
            // ASCII字符，使用现有的DrawChar函数
            DrawChar(x, y, font_datas.Z, (u8)code);
            
            special_char* schr = GetSpecialCharFromValue((u8)code);
            if (schr) {
                x += (font_datas.sx * schr->sx) * schr->fw / (float)font_datas.fonts[font_datas.current_font].w;
            } else {
                float ddX = dx * font_datas.fonts[font_datas.current_font].fw[(u8)code] / 
                           font_datas.fonts[font_datas.current_font].w;
                if (p > (const uint8_t *)str && *(p-1) == 'j')
                    ddX *= 2.0 / 3.0;
                if (code == 'm' || code == 'M')
                    ddX *= 0.9;
                if (code == '.')
                    ddX *= 3.0 / 2.0;
                x += ddX;
            }
        } else {
            // 对于非ASCII字符（如中文），使用我们修改后的多字体渲染功能
            // 保存当前字体状态
            int temp_font = font_datas.current_font;
            
            unsigned char bitmap[1024]; // 假设最大32x32位图
            short w, h, y_correction;
            
            // 使用TTF_to_Bitmap函数处理多字体渲染，尝试从PS3系统字体中查找合适的字符
            TTF_to_Bitmap(code, bitmap, &w, &h, &y_correction);
            
            if(w > 0 && h > 0)
            {
                // 1. 先计算与ASCII字符相同的字符框大小
                float char_height = font_datas.sy;
                float char_width = (font_datas.sx + font_datas.extra) * 2;
                
                // 2. 计算缩放因子，使中文字符适合这个字符框
                float scale_factor = 1.0f;
                if (h > 0 && w > 0) {
                    // 计算垂直方向的缩放因子
                    float scale_y = char_height / (float)h;
                    // 计算水平方向的缩放因子
                    float scale_x = char_width / (float)w;
                    // 使用较小的缩放因子，确保字符完全适合字符框
                    scale_factor = (scale_x < scale_y) ? scale_x : scale_y;
                    // 确保缩放因子不会太小，导致字体过细
                    if (scale_factor < 0.8f) {
                        scale_factor = 0.8f;
                    }
                }
                
                // 3. 计算垂直位置，确保中文字符的基线与ASCII字符对齐
                float dy2 = (float)(font_datas.sy * font_datas.fonts[current_font].bh) / (float)font_datas.fonts[current_font].h;
                float char_y_float = y + dy2 - (h * scale_factor);
                
                // 4. 微调垂直位置，确保视觉一致性
                char_y_float += 2;
                
                // 5. 计算字符在字符框中的水平居中位置
                float horizontal_offset = (char_width - (w * scale_factor)) / 2;
                int char_x = (int)x + horizontal_offset;
                int char_y = (int)char_y_float;
                
                // 绘制字符位图
                // 设置多边形模式
                tiny3d_SetPolygon(TINY3D_QUADS);
                
                // 对于每个像素，绘制一个四边形，并应用缩放因子
                int row, col;
                for (row = 0; row < h; row++) {
                    for (col = 0; col < w; col++) {
                        u8 alpha = bitmap[row * w + col];
                        if (alpha > 0) {
                            // 计算正确的颜色值，确保alpha通道被正确使用
                            u32 color = (font_datas.color & 0xffffff00) | alpha;
                             
                            // 应用缩放因子绘制字符
                            float scaled_row = row * scale_factor;
                            float scaled_col = col * scale_factor;
                            float scaled_next_row = scaled_row + scale_factor;
                            float scaled_next_col = scaled_col + scale_factor;
                             
                            // 绘制缩放后的像素
                            tiny3d_VertexPos(char_x + scaled_col, char_y + scaled_row, font_datas.Z);
                            tiny3d_VertexColor(color);
                            tiny3d_VertexPos(char_x + scaled_next_col, char_y + scaled_row, font_datas.Z);
                            tiny3d_VertexPos(char_x + scaled_next_col, char_y + scaled_next_row, font_datas.Z);
                            tiny3d_VertexPos(char_x + scaled_col, char_y + scaled_next_row, font_datas.Z);
                        }
                    }
                }
                
                // 结束绘制
                tiny3d_End();
                
                // 更新X坐标，使用与ASCII字符相同的逻辑，确保字符间距一致
                x += char_width;
            } else {
                // 如果无法渲染字符，绘制一个方块作为占位符
                float char_width = (font_datas.sx + font_datas.extra) * 2;
                float char_height = font_datas.sy;
                
                if (font_datas.bkcolor) {
                    tiny3d_SetPolygon(TINY3D_QUADS);
                    tiny3d_VertexPos(x, y, font_datas.Z);
                    tiny3d_VertexColor(font_datas.bkcolor);
                    tiny3d_VertexPos(x + char_width, y, font_datas.Z);
                    tiny3d_VertexPos(x + char_width, y + char_height, font_datas.Z);
                    tiny3d_VertexPos(x, y + char_height, font_datas.Z);
                    tiny3d_End();
                }
                
                tiny3d_SetPolygon(TINY3D_QUADS);
                tiny3d_VertexPos(x, y, font_datas.Z);
                tiny3d_VertexColor(font_datas.color);
                tiny3d_VertexPos(x + char_width, y, font_datas.Z);
                tiny3d_VertexPos(x + char_width, y + char_height, font_datas.Z);
                tiny3d_VertexPos(x, y + char_height, font_datas.Z);
                tiny3d_End();
                
                x += char_width;
            }
            
            // 恢复字体状态
            SetCurrentFont(temp_font);
        }
    }
    
    // 恢复原始字体设置
    font_datas.color = current_color;
    SetCurrentFont(current_font);
    
    font_datas.X = x; font_datas.Y = y;
    
    return x;
}

// UTF-8格式化字符串绘制函数
float DrawUTF8FormatString(float x, float y, const char *format, ...) {
    static char buff[4096];
    va_list opt;
    
    va_start(opt, format);
    vsnprintf(buff, sizeof(buff), format, opt);
    va_end(opt);
    
    return DrawUTF8String(x, y, buff);
}

float DrawString(float x, float y, char *str)
{
    switch (font_datas.align)
    {
    case FONT_ALIGN_SCREEN_CENTER:
        x= (848 - WidthFromStr(str)) / 2;
        break;

    case FONT_ALIGN_RIGHT:
		x -= WidthFromStr(str);
        break;

    case FONT_ALIGN_CENTER:
		x -= WidthFromStr(str)/2;
        break;

    default:
        break;
    }

    display_ttf_string((int)x + 1, (int)y + 1, str, 0x00000000 | (font_datas.color & 0x000000ff), 0, font_datas.sx, font_datas.sy + 4, &skip_icon);

    return display_ttf_string((int)x, (int)y, str, font_datas.color, font_datas.bkcolor, font_datas.sx, font_datas.sy + 4, &draw_icon);
}

static char buff[4096];

float DrawFormatString(float x, float y, char *format, ...)
{
	int dx = font_datas.sx;
	float initX = x;
    char *str = (char *) buff;
    va_list	opt;
	
	va_start(opt, format);
	vsprintf( (void *) buff, format, opt);
	va_end(opt);

    if(font_datas.align == 1) {
    
        x = (848 - WidthFromStr((u8 *) str)) / 2;

    }
	else if (font_datas.align == 2) {
		x -= WidthFromStr((u8 *) str);
	}
	else if (font_datas.align == 3) {
		x -= WidthFromStr((u8 *) str)/2;
	}

    while (*str) {
        
        if(*str == '\n') {
            x = initX; 
            y += font_datas.sy * font_datas.fonts[font_datas.current_font].bh / font_datas.fonts[font_datas.current_font].h; 
            str++;
            continue;
        } else {
            if(font_datas.autonewline && i_must_break_line(str, x)) {
                x = initX; 
                y += font_datas.sy * font_datas.fonts[font_datas.current_font].bh / font_datas.fonts[font_datas.current_font].h;
            }
        }

        DrawChar(x, y, font_datas.Z, (u8) *str);
       
        x += dx * font_datas.fonts[font_datas.current_font].fw[((u8)*str)] / font_datas.fonts[font_datas.current_font].w;
        str++;
    }

    font_datas.X = x; font_datas.Y = y;

    return x;
}
