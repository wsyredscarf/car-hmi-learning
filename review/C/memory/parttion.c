/*
五大区：
    |——栈区（stack）
        |——特点：系统自动开辟，自动回收，开辟空间较小，但是灵活，内存向下增长（高地址到低地址）。主要存放：局部变量、形参、const修饰的局部变量
    |——堆区（heap）
        |——特点：手动开辟，手动释放。不释放容易造成内存泄露和内存碎片化，内存地址向上增长（低地址往高地址）
    |——全局区（静态区）
        |——分为：.data和.bss。 .data存初始化为非0的全局变量，.bss主要存未初始化和初始化为0的全局变量。static修饰的局部变量也在此区域
            分.data和.bss，主要是为了优化内存存储和提高程序执行效率，确保资源得到有效利用和数据得到安全保障
    |——常量区（.rodata）
        |——存放：字符串字面量、全局const常量
    |——代码区（.text）
        |——编译后的程序机器指令（所有函数代码），只读
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// 1.初始化为非零的全局变量 放 全局初始化区(.data)
int global_val = 100;

// 2.未初始化和初始化为0的全局变量 放 全局未初始化区(.bss)
int global_uninit;
int global_uninit_g = 0;

//3.全局const常量,当const修饰全局变量时，该变量在整个程序的生命周期内都不能被修改。
const int a = 10;

void func_test(int param) // param：函数形参，栈
{
  
    int stack_local = 10;  // 局部变量（栈）

    static int static_local = 1; //静态局部变量：全局静态区

    const int b = 10;//const 局部变量,在这种情况下，b的值在函数test的执行期间不能被改变。

    printf("【栈】函数形参 param 地址:                     %p\n", &param);
    printf("【栈】func_test内局部变量 stack_local 地址:      %p\n", &stack_local);
    printf("【全局区】静态局部变量 static_local 地址:      %p\n", &static_local);
    printf("【栈】const 局部变量 b 地址:                     %p\n", &b);
}


int main (int vrgc, char * vrgv[])
{
   // 局部变量（栈）
    int main_local = 20;

     // 堆内存：手动malloc申请
    char *heap_buf = (char *)malloc(64);

    // 字符串字面量 → 只读常量区 .rodata
    char *str_const = "hello_memory";


    printf("==================== 内存五大分区地址展示 ====================\n");
    printf("【代码段text】main函数代码地址:                 %p\n", main);
    printf("【栈stack】main函数内局部变量 main_local地址:   %p\n", &main_local);
    printf("【堆heap】malloc申请内存地址 :                 %p\n", heap_buf);
    printf("【常量区rodata】字符串常量 str_const地址:       %p\n", str_const);
    printf("\n");

    printf("【全局区.data】初始化非0全局变量 global_val地址:   %p\n", &global_val);
    printf("【全局区.bss】未初始化全局变量 global_uninit地址:%p\n", &global_uninit);
    printf("【全局区.bss】初始化全局变量为0 global_uninit地址:%p\n", &global_uninit_g);
    printf("【全局区.bss】const修饰的全局变量 a:%p\n", &a);
    printf("\n");
   
    func_test(88);
    printf("\n");

    free(heap_buf); //堆内存释放
    heap_buf = NULL;

    return 0;
} 



