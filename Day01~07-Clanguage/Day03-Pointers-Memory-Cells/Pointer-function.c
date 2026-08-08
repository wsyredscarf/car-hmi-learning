#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
指针函数
函数指针
指针数组
数组指针
*/


//********************************指针函数*********************************//

//易错点：不能返回局部栈变量地址（会产生悬空指针），只能返回全局 /static修饰/ 堆内存地址。不然就函数周期结束，地址回收，指针悬空
// 函数返回 int* 指针 → 指针函数
// int* func(int a, int b)
// {
//     static int sum;
//     sum = a + b;
//     return &sum; // 返回整型地址
// }

//方案 1：改用 static 静态局部

// int* get_num()
// {
//     static int val = 100; // 静态区，函数销毁不消失，程序运行区间全程有效
//     return &val; //  安全
// }

//方案 2：全局变量
// int g_val = 100;
// int* get_num()
// {
//     return &g_val; // ✅ 安全
// }

//方案 3：堆 malloc
// int* get_num()
// {
//     int *p = malloc(sizeof(int));
//     *p = 100;
//     return p; // ✅ 堆内存，外部记得free
// }
//特殊点字符串字面量。  字符串数组也属于栈区，所以加上static变成静态数组
// char* get_msg()
// {
//     static char buf[] = "test msg"; // static静态数组
//     //补充一点：char *buf = "abc"; 字符串字面量在只读段，返回这个指针没问题，不属于栈局部变量。
//     return buf; // 没问题
// }


//*********************************************函数指针*********************************//

//示例 1：无参无返回函数指针
// void func(void)
// {
//     printf("hello func\n");
// }

// // 定义函数指针p，匹配void(void)类型函数
// void (*p)(void);

// int main(void)
// {
//     p = func;   // 取函数地址，&func 等价 func，两种写法都行
//     p();        // 通过指针调用函数，等价 func();
//     return 0;
// }

//示例 2：带参数、带返回值
// 目标函数：两个int入参，返回int
// int add(int a, int b)
// {
//     return a + b;
// }

// // 定义匹配类型的函数指针
// int (*calc)(int, int);

// int main(void)
// {
//     calc = add;
//     int res = calc(10, 20);
//     printf("sum=%d\n", res); // 输出30
//     return 0;
// }


//**************************************回调函数********* */
// 1. 普通函数定义 (给函数指针和回调用)
int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }

// ==========================================
// 2. 函数指针 & 回调函数 演示
// 这里的参数 int (*op)(int, int) 就是一个【函数指针】。
// 它像一个"插槽"，可以接收 add 或 sub 这样的函数地址进来。

int calc(int (*op)(int, int), int x, int y) {
    printf("执行回调函数... ");
    return op(x, y); // 这里触发了【回调】
}

void func_pointer_demo() {
    printf("\n--- 函数指针与回调演示 ---\n");

    // 定义函数指针变量
    int (*p_func)(int, int) = NULL;
    
    // 让函数指针指向 add 函数
    p_func = add;
    printf("通过函数指针调用 add(5, 3) 的结果: %d\n", p_func(5, 3));

    // 让函数指针指向 sub 函数
    p_func = sub;
    printf("通过函数指针调用 sub(5, 3) 的结果: %d\n", p_func(5, 3));

    // 演示：把函数指针当做参数传入，实现回调
    // calc 函数并不管传进来的是 add 还是 sub，它只负责调。这就是回调的精髓！
    int result_add = calc(add, 10, 5);
    int result_sub = calc(sub, 10, 5);
    
    printf("回调计算结果 (add): %d\n", result_add);
    printf("回调计算结果 (sub): %d\n", result_sub);
}


//**************************************函数指针数组（工程常用）********* */
// typedef int (*Op)(int,int);
// int add(int a,int b){return a+b;}
// int mul(int a,int b){return a*b;}
// int sub(int a,int b){return a-b;}

// 函数指针数组，映射序号和函数
// Op op_table[] = {add, sub, mul};

// int calc(int x,int y,int select)
// {
//     return op_table[select](x,y); // 直接查表调用，无分支判断
// }

// ==========================================
// 主函数入口
// ==========================================
int main(void)
{
    func_pointer_demo();
    
    return 0;
}