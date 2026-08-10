#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#define VALUE 100

void show_value(int value)
{
    printf("打印出值位：%d\n",value);
}

int main(void)
{
    show_value(VALUE);
    return 0;
}