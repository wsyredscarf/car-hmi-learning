#include "common.h"
//主函数入口和.h文件，不能进行打包
int main(void)
{
    int a = 100;
    int b = 50;

    printf("add_fun(a+b)= %d\n",add_fun(a,b));
    printf("sun_fun(a-b)= %d\n",sub_fun(a,b));

    return 0;
}