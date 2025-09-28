/*
 * ArtemisPS3-GUI - UTF-8文件系统操作支持
 *
 * 该模块提供了支持UTF-8编码的文件系统操作函数，允许正确处理包含中文等非ASCII字符的文件名和路径。
 */

#ifndef UTF8_FS_H
#define UTF8_FS_H

#include <sys/types.h>
#include <dirent.h>
#include "utf8_utils.h"

/*
 * 函数: utf8_opendir
 * 描述: 打开一个UTF-8编码的目录
 * 参数:
 *   path - UTF-8编码的目录路径
 * 返回: 
 *   成功返回DIR*指针，失败返回NULL
 */
DIR *utf8_opendir(const char *path);

/*
 * 函数: utf8_closedir
 * 描述: 关闭由utf8_opendir打开的目录
 * 参数:
 *   dir - DIR*指针
 * 返回: 
 *   成功返回0，失败返回-1
 */
int utf8_closedir(DIR *dir);

/*
 * 函数: utf8_readdir
 * 描述: 读取目录中的下一个条目，并将文件名转换为UTF-8编码
 * 参数:
 *   dir - DIR*指针
 *   name - 用于存储UTF-8编码文件名的缓冲区
 *   name_len - 缓冲区大小
 * 返回: 
 *   成功返回0，没有更多条目返回1，失败返回-1
 */
int utf8_readdir(DIR *dir, char *name, size_t name_len);

/*
 * 函数: utf8_dir_exists
 * 描述: 检查UTF-8编码的目录是否存在
 * 参数:
 *   path - UTF-8编码的目录路径
 * 返回: 
 *   存在返回SUCCESS，不存在返回ERROR
 */
int utf8_dir_exists(const char *path);

/*
 * 函数: utf8_file_exists
 * 描述: 检查UTF-8编码的文件是否存在
 * 参数:
 *   path - UTF-8编码的文件路径
 * 返回: 
 *   存在返回SUCCESS，不存在返回ERROR
 */
int utf8_file_exists(const char *path);

/*
 * 函数: utf8_get_dir_list_size
 * 描述: 获取目录中文件和子目录的数量
 * 参数:
 *   path - UTF-8编码的目录路径
 * 返回: 
 *   成功返回文件和子目录数量，失败返回-1
 */
int utf8_get_dir_list_size(const char *path);

/*
 * 函数: utf8_ends_with
 * 描述: 检查UTF-8字符串是否以指定的后缀结尾
 * 参数:
 *   str - UTF-8字符串
 *   suffix - 要检查的后缀
 * 返回: 
 *   以指定后缀结尾返回1，否则返回0
 */
int utf8_ends_with(const char *str, const char *suffix);

#endif // UTF8_FS_H