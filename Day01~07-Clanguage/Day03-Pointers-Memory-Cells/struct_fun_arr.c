#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
指针数组:数组里面存的是指针变量，只是地址，而字符串字面量在只读段，不能修改：//menu[0][0] = 'O'; // 段错误崩溃
*/
int poin_arrr(void)
{
    // 指针数组：4个char*，每个指针指向字符串常量
    char *menu[4] = {"open", "close", "read", "write"};
    for(int i=0; i<4; i++)
    {
        printf("%s\n", menu[i]);
    }
}

/*
数组指针:指针指向一个定长数组：
*/
// int arr[5] = {1,2,3,4,5};
// int (*p)[5] = &arr;//必须取数组地址 &arr，不能写arr
// p++;//p偏移 5*4=20字节（int占4字节），直接跳到下一个长度为5的数组


//补充赋值区别
// char buf[10] = "abc";

// // str1：数组里每个格子存字符串地址
// str1[0] = "hello";

// // str2：只能接收完整数组的地址 &buf
// str2 = &buf;
// // str2 = buf; 编译警告/报错，类型不匹配



//****************************************函数指针数组
//多个同类型函数，存到数组批量管理
// typedef void (*MenuFunc)(void);

// void func1(){printf("1. 开灯\n");}
// void func2(){printf("2. 关灯\n");}
// void func3(){printf("3. 查询状态\n");}

// int main(void)
// {
//     // 函数指针数组
//     MenuFunc menu[] = {func1, func2, func3};
//     int select = 1;
//     menu[select](); // 执行func2
//     return 0;
// }
/*
//****************************************结构体内函数指针
结构体内嵌函数指针（面向对象 C 语言、驱动框架）
给结构体绑定操作函数，模拟类的方法
*/
typedef struct
{
    int id;
    void(*print_info)(int);//结构体内部的函数指针变量
} Device; 

void show_id(int id)
{
    printf("设备id = %d\n",id);
}


int main(void)
{
    Device dev;
    dev.id = 101;
    dev.print_info = show_id;
    dev.print_info(dev.id);

    return 0;
}