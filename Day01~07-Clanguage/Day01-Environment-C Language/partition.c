#include <stdio.h>
#include <stdlib.h>

// 1.全局变量 → 全局初始化区(.data)
int global_val = 100;
// 2.未初始化全局变量 → 全局未初始化区(.bss)
int global_uninit;

void func_test(int param) // param：函数形参，栈
{
    // 局部变量（栈）
    int stack_local = 10;
    static int static_local = 1; //静态局部变量：全局静态区

    printf("【栈】func_test内局部变量 stack_local 地址:      %p\n", &stack_local);
    printf("【栈】函数形参 param 地址:                     %p\n", &param);
    printf("【全局区】静态局部变量 static_local 地址:      %p\n", &static_local);
}

int main(void)
{
    // 字符串字面量 → 只读常量区 .rodata
    char *str_const = "hello_memory";

    // 局部变量（栈）
    int main_local = 20;

    // 堆内存：手动malloc申请
    char *heap_buf = (char *)malloc(64);//申请64字节

    printf("==================== 内存五大分区地址展示 ====================\n");
    printf("【代码段text】main函数代码地址:                 %p\n", main);
    printf("【常量区rodata】字符串常量 str_const地址:       %p\n", str_const);
    printf("\n");

    printf("【全局区.data】初始化全局变量 global_val地址:   %p\n", &global_val);
    printf("【全局区.bss】未初始化全局变量 global_uninit地址:%p\n", &global_uninit);
    printf("\n");

    printf("【栈stack】main函数内局部变量 main_local地址:   %p\n", &main_local);
    func_test(88);
    printf("\n");

    printf("【堆heap】malloc申请内存地址：%p\n", heap_buf);

    free(heap_buf); //堆内存释放
    heap_buf = NULL;

    return 0;
}