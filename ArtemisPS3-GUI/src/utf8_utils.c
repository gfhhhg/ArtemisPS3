/*
 * UTF-8编码处理工具实现
 * 用于支持中文等多字节字符的显示
 */

#include "utf8_utils.h"
#include <string.h>

// UTF-8字符解码函数
uint32_t utf8_decode(const uint8_t **str) {
    uint32_t code = 0;
    uint8_t c = **str;
    
    if (c < 0x80) {
        // 单字节字符
        code = c;
        (*str)++;
    } else if ((c & 0xE0) == 0xC0) {
        // 双字节字符
        code = (c & 0x1F) << 6;
        (*str)++;
        c = **str;
        code |= (c & 0x3F);
        (*str)++;
    } else if ((c & 0xF0) == 0xE0) {
        // 三字节字符
        code = (c & 0x0F) << 12;
        (*str)++;
        c = **str;
        code |= (c & 0x3F) << 6;
        (*str)++;
        c = **str;
        code |= (c & 0x3F);
        (*str)++;
    } else if ((c & 0xF8) == 0xF0) {
        // 四字节字符
        code = (c & 0x07) << 18;
        (*str)++;
        c = **str;
        code |= (c & 0x3F) << 12;
        (*str)++;
        c = **str;
        code |= (c & 0x3F) << 6;
        (*str)++;
        c = **str;
        code |= (c & 0x3F);
        (*str)++;
    } else {
        // 无效的UTF-8字符
        (*str)++;
        code = 0xFFFD; // UTF-8替换字符
    }
    
    return code;
}

// 获取UTF-8字符串中字符的数量
size_t utf8_strlen(const uint8_t *str) {
    size_t len = 0;
    const uint8_t *p = str;
    
    while (*p) {
        utf8_decode(&p);
        len++;
    }
    
    return len;
}

// 将UTF-8字符串转换为宽字符
int utf8_to_wchar(const char *utf8_str, uint16_t *wchar_buf, size_t buf_size) {
    const uint8_t *str = (const uint8_t *)utf8_str;
    size_t count = 0;
    
    while (*str && count < buf_size) {
        uint32_t code = utf8_decode(&str);
        
        // UCS-2只能表示0x0000-0xFFFF范围内的字符
        if (code <= 0xFFFF) {
            wchar_buf[count++] = (uint16_t)code;
        } else if (code <= 0x10FFFF) {
            // 对于Unicode补充平面的字符，使用UTF-16代理对
            if (count + 1 < buf_size) {
                code -= 0x10000;
                wchar_buf[count++] = 0xD800 | (code >> 10);
                wchar_buf[count++] = 0xDC00 | (code & 0x3FF);
            } else {
                break;
            }
        } else {
            // 无效的Unicode码点
            wchar_buf[count++] = 0xFFFD;
        }
    }
    
    return (int)count;
}

// 将宽字符转换为UTF-8字符串
int wchar_to_utf8(const uint16_t *wchar_buf, char *utf8_str, size_t buf_size) {
    size_t count = 0;
    size_t i = 0;
    
    while (wchar_buf[i] && count < buf_size) {
        uint16_t c = wchar_buf[i];
        
        if (c < 0x80) {
            // 单字节
            if (count + 1 <= buf_size) {
                utf8_str[count++] = (char)c;
                i++;
            } else {
                break;
            }
        } else if (c < 0x800) {
            // 双字节
            if (count + 2 <= buf_size) {
                utf8_str[count++] = 0xC0 | (c >> 6);
                utf8_str[count++] = 0x80 | (c & 0x3F);
                i++;
            } else {
                break;
            }
        } else if (c >= 0xD800 && c <= 0xDBFF && i + 1 < buf_size) {
            // UTF-16高代理项
            uint16_t low = wchar_buf[i + 1];
            if (low >= 0xDC00 && low <= 0xDFFF) {
                // 有效的UTF-16代理对
                if (count + 4 <= buf_size) {
                    uint32_t code = 0x10000 + ((c - 0xD800) << 10) + (low - 0xDC00);
                    utf8_str[count++] = 0xF0 | (code >> 18);
                    utf8_str[count++] = 0x80 | ((code >> 12) & 0x3F);
                    utf8_str[count++] = 0x80 | ((code >> 6) & 0x3F);
                    utf8_str[count++] = 0x80 | (code & 0x3F);
                    i += 2;
                } else {
                    break;
                }
            } else {
                // 无效的代理对
                if (count + 3 <= buf_size) {
                    utf8_str[count++] = 0xE0 | (c >> 12);
                    utf8_str[count++] = 0x80 | ((c >> 6) & 0x3F);
                    utf8_str[count++] = 0x80 | (c & 0x3F);
                    i++;
                } else {
                    break;
                }
            }
        } else if (c >= 0xDC00 && c <= 0xDFFF) {
            // 无效的低代理项单独出现
            if (count + 3 <= buf_size) {
                utf8_str[count++] = 0xE0 | (c >> 12);
                utf8_str[count++] = 0x80 | ((c >> 6) & 0x3F);
                utf8_str[count++] = 0x80 | (c & 0x3F);
                i++;
            } else {
                break;
            }
        } else {
            // 三字节
            if (count + 3 <= buf_size) {
                utf8_str[count++] = 0xE0 | (c >> 12);
                utf8_str[count++] = 0x80 | ((c >> 6) & 0x3F);
                utf8_str[count++] = 0x80 | (c & 0x3F);
                i++;
            } else {
                break;
            }
        }
    }
    
    // 确保字符串以null结尾
    if (count < buf_size) {
        utf8_str[count] = '\0';
    } else if (buf_size > 0) {
        utf8_str[buf_size - 1] = '\0';
        count = buf_size - 1;
    }
    
    return (int)count;
}

// 获取UTF-8字符的字节数
int utf8_char_len(uint8_t c) {
    if (c < 0x80) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 0; // 无效的UTF-8首字节
}

// 检查UTF-8字符串是否有效
int utf8_is_valid(const char *str) {
    const uint8_t *p = (const uint8_t *)str;
    
    while (*p) {
        int len = utf8_char_len(*p);
        if (len == 0) return 0; // 无效的首字节
        
        // 检查后续字节
        for (int i = 1; i < len; i++) {
            if (!p[i] || (p[i] & 0xC0) != 0x80) {
                return 0; // 无效的后续字节
            }
        }
        
        p += len;
    }
    
    return 1;
}