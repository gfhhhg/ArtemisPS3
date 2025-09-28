#include "ttf_fonts.h"

/******************************************************************************************************************************************************/
/* TTF functions to load and convert fonts          
 * From fonts_from_ttf Tiny3D sample                                                                                                   */
 /*****************************************************************************************************************************************************/

int ttf_inited = 0;

FT_Library freetype;
FT_Face face[4];
int f_face[4] = {0, 0, 0, 0};

/* TTFLoadFont can load TTF fonts from device or from memory:
set = index of the font to load (0-3)
path = path to the font or NULL to work from memory
from_memory = pointer to the font in memory. It is ignored if path != NULL.
size_from_memory = size of the memory font. It is ignored if path != NULL.
*/
int TTFLoadFont(int set, char * path, void * from_memory, int size_from_memory)
{
   
    if(set < 0 || set > 3) return -1;
    
    if(!ttf_inited)
        FT_Init_FreeType(&freetype);
    ttf_inited = 1;

    f_face[set] = 0;

    if(path) {
        if(FT_New_Face(freetype, path, 0, &face[set]) < 0) return -1;
    } else {
        if(FT_New_Memory_Face(freetype, from_memory, size_from_memory, 0, &face[set])) return -1;
        }

    f_face[set] = 1;

    return 0;
}

/* release all */
void TTFUnloadFont()
{
   FT_Done_FreeType(freetype);
   face = NULL; // 确保face指针被设置为NULL，避免访问已释放的内存
   ttf_inited = 0;
}

/* function to render the character
chr : character from 0 to 255
bitmap: u8 bitmap passed to render the character character (max 256 x 256 x 1 (8 bits Alpha))
*w : w is the bitmap width as input and the width of the character (used to increase X) as output
*h : h is the bitmap height as input and the height of the character (used to Y correction combined with y_correction) as output
y_correction : the Y correction to display the character correctly in the screen
*/

int doShrinkChar = 0;
void TTF_to_Bitmap(uint8_t chr, uint8_t * bitmap, short *w, short *h, short *y_correction)
{
    if(f_face[0]) FT_Set_Pixel_Sizes(face[0], (*w), (*h));
    if(f_face[1]) FT_Set_Pixel_Sizes(face[1], (*w), (*h));
    if(f_face[2]) FT_Set_Pixel_Sizes(face[2], (*w), (*h));
    if(f_face[3]) FT_Set_Pixel_Sizes(face[3], (*w), (*h));
    
    FT_GlyphSlot slot = NULL;

    memset(bitmap, 0, (*w) * (*h));

    FT_UInt index;

    // 尝试从已加载的字体中找到能显示该字符的字体
    if(f_face[0] && (index = FT_Get_Char_Index(face[0], (char) chr)) != 0 
        && !FT_Load_Glyph(face[0], index, FT_LOAD_RENDER)) slot = face[0]->glyph;
    else if(f_face[1] && (index = FT_Get_Char_Index(face[1], (char) chr)) != 0 
        && !FT_Load_Glyph(face[1], index, FT_LOAD_RENDER)) slot = face[1]->glyph;
    else if(f_face[2] && (index = FT_Get_Char_Index(face[2], (char) chr)) != 0 
        && !FT_Load_Glyph(face[2], index, FT_LOAD_RENDER)) slot = face[2]->glyph;
    else if(f_face[3] && (index = FT_Get_Char_Index(face[3], (char) chr)) != 0 
        && !FT_Load_Glyph(face[3], index, FT_LOAD_RENDER)) slot = face[3]->glyph;
    else {(*w) = 0; return;}

    int n, m, ww;

    *y_correction = (*h) - 1 - slot->bitmap_top;
    
    ww = 0;

    for(n = 0; n < slot->bitmap.rows; n++) {
        for (m = 0; m < slot->bitmap.width; m++) {

            if(m >= (*w) || n >= (*h)) continue;
            
            bitmap[m] = (uint8_t) slot->bitmap.buffer[ww + m];
        }
    
    bitmap += *w;

    ww += slot->bitmap.width;
    }

    *w = ((slot->advance.x + 31) >> 6) + ((slot->bitmap_left < 0) ? -slot->bitmap_left : 0);
    *h = slot->bitmap.rows;
}
}
