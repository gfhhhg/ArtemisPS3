/*
 * UTF-8编码处理工具
 * 用于支持中文等多字节字符的显示
 */

#ifndef UTF8_UTILS_H
#define UTF8_UTILS_H

#include <stdint.h>
#include <stddef.h>

// UTF-8字符解码函数
// 返回解码后的Unicode码点，并更新字符串指针到下一个字符
uint32_t utf8_decode(const uint8_t **str);

// 获取UTF-8字符串中字符的数量（不是字节数）
size_t utf8_strlen(const uint8_t *str);

// 将UTF-8字符串转换为宽字符（UCS-2/UTF-16）
// 返回转换后的宽字符数量
int utf8_to_wchar(const char *utf8_str, uint16_t *wchar_buf, size_t buf_size);

// 将宽字符（UCS-2/UTF-16）转换为UTF-8字符串
// 返回转换后的UTF-8字符串字节数
int wchar_to_utf8(const uint16_t *wchar_buf, char *utf8_str, size_t buf_size);

// 获取UTF-8字符的字节数
int utf8_char_len(uint8_t c);

// 检查UTF-8字符串是否有效
int utf8_is_valid(const char *str);

#endif // UTF8_UTILS_H