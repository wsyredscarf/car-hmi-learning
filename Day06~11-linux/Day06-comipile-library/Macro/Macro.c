#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define A 100
#define B 20
#define CHAR "hello word!"

//定义一个求最大值的宏，必须多层小括号，确保展开无问题，否则展开时替换优先级出问题
#define MAX(a,b) ((a)>(b)?(a):(b))

int main(void)
{
        int x = 10, y = 20;
        int max_val = MAX(x, y);
        printf("最大值是: %d\n", max_val); // 输出 20
        printf("A= %d\n",A);
        printf("CHAR= %s\n",CHAR);

    return 0;
}