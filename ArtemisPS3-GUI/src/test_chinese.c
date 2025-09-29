/*
 * 中文显示测试文件
 * 用于验证Artemis PS3界面是否正确支持中文显示
 */

#include <stdio.h>
#include "libfont.h"
#include <tiny3d.h>
#include "common.h"

// 中文测试函数
void TestChineseDisplay() {
    // 初始化Tiny3D
    tiny3d_Init(1024*1024); // 正确的参数：只需要顶点缓冲区大小
    
    // 设置字体
    SetCurrentFont(0); // 使用默认字体索引
    SetFontSize(24, 24);
    SetFontColor(0x000000FF, 0xFFFFFFFF); // 黑色文字，白色背景
    SetFontAlign(1); // 居中对齐
    
    // 清除屏幕
    tiny3d_Clear(0xFFFFFFFF, TINY3D_CLEAR_ALL);
    
    // 启用alpha测试和混合
    tiny3d_AlphaTest(1, 0x10, TINY3D_ALPHA_FUNC_GEQUAL);
    tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
        TINY3D_BLEND_FUNC_SRC_ALPHA_ONE_MINUS_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_RGB_ONE_MINUS_SRC_ALPHA,
        TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);
    
    // 渲染2D内容
    tiny3d_Project2D();
    
    // 使用DrawUTF8String显示中文
    DrawUTF8String(424, 100, "Artemis PS3 中文显示测试");
    DrawUTF8String(424, 150, "这是一段中文文本，用于测试UTF-8显示功能");
    
    // 使用DrawFormatString显示中文
    DrawFormatString(424, 200, "格式化字符串测试：%s", "你好，世界！");
    
    // 测试不同字体大小
    SetFontSize(32, 32);
    DrawUTF8String(424, 250, "大号字体测试");
    
    SetFontSize(16, 16);
    DrawUTF8String(424, 300, "小号字体测试");
    
    // 测试不同颜色
    SetFontSize(24, 24);
    SetFontColor(0xFF0000FF, 0xFFFFFFFF); // 红色文字
    DrawUTF8String(424, 350, "红色文字测试");
    
    SetFontColor(0x00FF00FF, 0xFFFFFFFF); // 绿色文字
    DrawUTF8String(424, 400, "绿色文字测试");
    
    // 测试对齐方式
    SetFontColor(0x000000FF, 0xFFFFFFFF); // 恢复黑色文字
    SetFontAlign(0); // 左对齐
    DrawUTF8String(50, 450, "左对齐文本");
    
    SetFontAlign(2); // 右对齐
    DrawUTF8String(798, 450, "右对齐文本");
    
    // 显示版权信息
    SetFontSize(14, 14);
    SetFontAlign(1);
    DrawUTF8String(424, 480, "按任意键退出测试");
    
    // 刷新屏幕
    tiny3d_Flip();
    
    // 等待用户按键
    printf("中文显示测试已运行，请在PS3上查看结果\n");
    
    // 注意：tiny3d_Shutdown函数在当前环境中未定义，因此不需要调用
}

// 主函数入口（如果需要单独运行）
#ifdef STANDALONE_TEST
int main() {
    TestChineseDisplay();
    return 0;
}
#endif