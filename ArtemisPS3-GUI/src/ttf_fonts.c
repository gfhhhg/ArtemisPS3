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
    
    if(!ttf_inited) {
        if(FT_Init_FreeType(&freetype) != 0) {
            // FreeType初始化失败
            return -1;
        }
        ttf_inited = 1;
    }

    f_face[set] = 0;

    if(path) {
        if(FT_New_Face(freetype, path, 0, &face[set]) < 0) {
            // 尝试从系统字体目录加载备选字体
            char system_font_path[256];
            
            // 根据set索引选择不同的系统字体
            switch(set) {
                case 0: // 拉丁字体
                    snprintf(system_font_path, sizeof(system_font_path), "/dev_flash/data/font/ltn0.pgf");
                    break;
                case 1: // 中文字体
                    snprintf(system_font_path, sizeof(system_font_path), "/dev_flash/data/font/cht0.pgf");
                    break;
                case 2: // 日文字体
                    snprintf(system_font_path, sizeof(system_font_path), "/dev_flash/data/font/jpn0.pgf");
                    break;
                case 3: // 韩文字体
                    snprintf(system_font_path, sizeof(system_font_path), "/dev_flash/data/font/krn0.pgf");
                    break;
                default:
                    return -1;
            }
            
            // 尝试加载系统字体
            if(FT_New_Face(freetype, system_font_path, 0, &face[set]) < 0) {
                return -1; // 仍然失败
            }
        }
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
   // 将face数组的每个元素设置为NULL，避免访问已释放的内存
   for(int i = 0; i < 4; i++) {
      face[i] = NULL;
      f_face[i] = 0;
   }
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

// 初始化PS3系统字体
int InitPS3SystemFonts() {
    // 尝试加载PS3系统字体
    int result = 0;
    
    // 加载拉丁字体(ltn0.pgf)
    result |= TTFLoadFont(0, (char*)"/dev_flash/data/font/ltn0.pgf", NULL, 0);
    
    // 加载中文字体(cht0.pgf)
    result |= TTFLoadFont(1, (char*)"/dev_flash/data/font/cht0.pgf", NULL, 0);
    
    // 加载日文字体(jpn0.pgf)
    result |= TTFLoadFont(2, (char*)"/dev_flash/data/font/jpn0.pgf", NULL, 0);
    
    // 加载韩文字体(krn0.pgf)
    result |= TTFLoadFont(3, (char*)"/dev_flash/data/font/krn0.pgf", NULL, 0);
    
    return result;
}

void TTF_to_Bitmap(uint32_t code, uint8_t * bitmap, short *w, short *h, short *y_correction)
{
    // 确保FreeType已初始化
    if(!ttf_inited) {
        if(FT_Init_FreeType(&freetype) != 0) {
            (*w) = 0;
            return;
        }
        ttf_inited = 1;
    }
    
    // 自动检测是否需要加载系统字体
    int need_font_load = 0;
    for(int i = 0; i < 4; i++) {
        if(!f_face[i]) {
            need_font_load = 1;
            break;
        }
    }
    
    if(need_font_load) {
        InitPS3SystemFonts();
    }
    
    // 设置字体大小
    if(f_face[0]) FT_Set_Pixel_Sizes(face[0], (*w), (*h));
    if(f_face[1]) FT_Set_Pixel_Sizes(face[1], (*w), (*h));
    if(f_face[2]) FT_Set_Pixel_Sizes(face[2], (*w), (*h));
    if(f_face[3]) FT_Set_Pixel_Sizes(face[3], (*w), (*h));
    
    FT_GlyphSlot slot = NULL;

    memset(bitmap, 0, (*w) * (*h));

    FT_UInt index;

    // 针对中文字符的特殊处理：优先使用中文字体槽位
    if(code >= 0x4E00 && code <= 0x9FFF) { // 基本汉字范围
        if(f_face[1] && (index = FT_Get_Char_Index(face[1], code)) != 0 
            && !FT_Load_Glyph(face[1], index, FT_LOAD_RENDER)) {
            slot = face[1]->glyph;
        }
    }
    
    // 如果中文字体没找到，尝试从其他字体槽位寻找
    if(!slot) {
        // 尝试从已加载的字体中找到能显示该字符的字体
        if(f_face[0] && (index = FT_Get_Char_Index(face[0], code)) != 0 
            && !FT_Load_Glyph(face[0], index, FT_LOAD_RENDER)) slot = face[0]->glyph;
        else if(f_face[1] && (index = FT_Get_Char_Index(face[1], code)) != 0 
            && !FT_Load_Glyph(face[1], index, FT_LOAD_RENDER)) slot = face[1]->glyph;
        else if(f_face[2] && (index = FT_Get_Char_Index(face[2], code)) != 0 
            && !FT_Load_Glyph(face[2], index, FT_LOAD_RENDER)) slot = face[2]->glyph;
        else if(f_face[3] && (index = FT_Get_Char_Index(face[3], code)) != 0 
            && !FT_Load_Glyph(face[3], index, FT_LOAD_RENDER)) slot = face[3]->glyph;
        else {(*w) = 0; return;}
    }

    int n, m, ww;

    *y_correction = (*h) - 1 - slot->bitmap_top;
    
    ww = 0;

    for(n = 0; n < slot->bitmap.rows; n++) {
        for (m = 0; m < slot->bitmap.width; m++) {

            if(m >= (*w) || n >= (*h)) continue;
            
            bitmap[(n * (*w)) + m] = (uint8_t) slot->bitmap.buffer[ww + m];
        }
    
    ww += slot->bitmap.width;
    }

    *w = ((slot->advance.x + 31) >> 6) + ((slot->bitmap_left < 0) ? -slot->bitmap_left : 0);
    *h = slot->bitmap.rows;
}
