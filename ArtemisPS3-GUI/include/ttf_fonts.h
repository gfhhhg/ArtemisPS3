#ifndef TTF_FONTS_H
#define TTF_FONTS_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

// 全局变量声明
extern int ttf_inited;
extern FT_Library freetype;
extern FT_Face face;
extern int doShrinkChar;

// 函数声明
int TTFLoadFont(char * path, void * from_memory, int size_from_memory);
void TTFUnloadFont();
void TTF_to_Bitmap(uint8_t chr, uint8_t * bitmap, short *w, short *h, short *y_correction);

#endif // TTF_FONTS_H