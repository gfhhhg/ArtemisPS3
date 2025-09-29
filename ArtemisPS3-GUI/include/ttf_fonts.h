#ifndef TTF_FONTS_H
#define TTF_FONTS_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

// 全局变量声明
extern int ttf_inited;
extern FT_Library freetype;
extern FT_Face face[4];
extern int f_face[4];
extern int doShrinkChar;

// 函数声明
int TTFLoadFont(int set, char * path, void * from_memory, int size_from_memory);
void TTFUnloadFont();
void TTF_to_Bitmap(uint32_t code, uint8_t * bitmap, short *w, short *h, short *y_correction);
int InitPS3SystemFonts();

#endif // TTF_FONTS_H