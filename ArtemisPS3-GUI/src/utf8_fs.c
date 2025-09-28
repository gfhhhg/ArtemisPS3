/*
 * ArtemisPS3-GUI - UTF-8文件系统操作支持实现
 *
 * 该模块提供了支持UTF-8编码的文件系统操作函数实现。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "utf8_fs.h"

/*
 * 函数: utf8_opendir
 * 描述: 打开一个UTF-8编码的目录
 */
DIR *utf8_opendir(const char *path)
{
    if (!path)
        return NULL;
    
    // PS3平台上，我们假设文件系统API已经能够处理UTF-8路径
    // 在实际应用中，可能需要根据PS3的具体API进行路径转换
    return opendir(path);
}

/*
 * 函数: utf8_closedir
 * 描述: 关闭由utf8_opendir打开的目录
 */
int utf8_closedir(DIR *dir)
{
    if (!dir)
        return -1;
    
    return closedir(dir);
}

/*
 * 函数: utf8_readdir
 * 描述: 读取目录中的下一个条目，并将文件名转换为UTF-8编码
 */
int utf8_readdir(DIR *dir, char *name, size_t name_len)
{
    if (!dir || !name)
        return -1;
    
    struct dirent *entry = readdir(dir);
    if (!entry)
        return 1; // 没有更多条目
    
    // 确保文件名以null结尾
    strncpy(name, entry->d_name, name_len - 1);
    name[name_len - 1] = '\0';
    
    return 0; // 成功
}

/*
 * 函数: utf8_dir_exists
 * 描述: 检查UTF-8编码的目录是否存在
 */
int utf8_dir_exists(const char *path)
{
    if (!path)
        return ERROR;
    
    struct stat st;
    if (stat(path, &st) != 0)
        return ERROR;
    
    return S_ISDIR(st.st_mode) ? SUCCESS : ERROR;
}

/*
 * 函数: utf8_file_exists
 * 描述: 检查UTF-8编码的文件是否存在
 */
int utf8_file_exists(const char *path)
{
    if (!path)
        return ERROR;
    
    struct stat st;
    if (stat(path, &st) != 0)
        return ERROR;
    
    return S_ISREG(st.st_mode) ? SUCCESS : ERROR;
}

/*
 * 函数: utf8_get_dir_list_size
 * 描述: 获取目录中文件和子目录的数量
 */
int utf8_get_dir_list_size(const char *path)
{
    if (!path)
        return -1;
    
    DIR *dir = utf8_opendir(path);
    if (!dir)
        return -1;
    
    int count = 0;
    struct dirent *entry;
    
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            count++;
    }
    
    utf8_closedir(dir);
    return count;
}

/*
 * 函数: utf8_ends_with
 * 描述: 检查UTF-8字符串是否以指定的后缀结尾
 */
int utf8_ends_with(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    
    if (suffix_len > str_len)
        return 0;
    
    // 从字符串末尾开始比较后缀
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}