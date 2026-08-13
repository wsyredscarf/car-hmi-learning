#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void crash_fun(void)
{
    int *p =NULL;
    *p = 100;
    printf("没走到此处！\n");
}

int main(void)
{
    crash_fun();
    return 0;
}