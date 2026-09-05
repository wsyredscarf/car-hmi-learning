#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int main(int vage ,char* vagc[])
{

    srand(time(NULL));//随机数种子
    while(1){
        int speed = rand() % 120;//车速模拟
        int temp = 25 + rand() % 10;//温度模拟
        printf("ID 1: SPEED:%dkm/h, TEMP:%d°\n",speed,temp);
        sleep(1);//一秒一次发送数据

    }

    return 0; 
}