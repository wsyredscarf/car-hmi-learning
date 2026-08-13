#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int *g_p = NULL;//不手动置空，系统会自动置空。手动可读性更好

void test(void)
{
    g_p = malloc(100);//堆内存
    free(g_p);//全局变量和局部变量，只要用堆空间就要手动释放，不释放就会造成内存泄漏
    g_p = NULL;// 不置空就会变成悬空指针
}


int main(void)
{

/*
栈区（stack）的使用，系统静态开辟和回收，编译器进行内存分配,局部函数结束时即回收，栈的地址位置很大，响应快。
*/
    int stack_num[] = {1,2,3,4,5,6,7,8};
    for(int i=0; i<8;i++)
    {
        printf("stack_num = %d\n",stack_num[i]);
        printf("stack_num地址= %p\n",&stack_num[i]);
    }
    int S = 0;
    printf("S的地址= %p\n",&S);

    int *p =NULL;
    printf("p1地址 =%p\n",p);

/*
堆区（heap）的使用，手动动态开辟和回收，寻找较大内存块，周期为free之前，堆的地址基本在0x5左右比较小，响应较慢
*/
    int *p1=NULL;
    p1 = (int*)malloc(sizeof(int));
    if(!p1)
    {
        printf("p1 = NULL!!!!!!\n");
    }else{
        printf("p1地址 =%p\n",p1);
    }

    free(p1);//释放，防止内存泄漏（如果不释放，下面直接置空，就会丢失这块空间的起始地址，无法使用，最终造成内存泄漏，如果较多的这样的错误，就会越来越卡，甚至卡死机）
    p1=NULL;//置空防止悬空指针（野指针的一种：空间被回收了，但是地址还在）
    test();

    return 0;
}