#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//结构体保存车辆信息
typedef struct{
    char plate[20];//车牌信息
    char owner[20];//车主名字
    int speed;     //车速
    int age;       //车主年龄
}Vehicle;

//写
void fun_fprintf_write(const char* file,Vehicle *cars ,int count)
{
    FILE *fp = fopen(file,"w");
    if(fp==NULL)
    {
        perror("file open fail");
        return;
    }
    fprintf(fp,"%d\n",count);//第一行存车辆个数
    for(int i=0;i<count;i++)
    {
        fprintf(fp,"%s %s %d %d\n",cars[i].plate,cars[i].owner,cars[i].speed,cars[i].age);
    }
    fclose(fp);
    printf("成功保存 %d 辆车的信息到 %s\n", count, file);

}

//读
void fun_fscanf_read(const char*file)
{
    FILE *fp = fopen(file,"r");
    if(fp==NULL)
    {
        perror("file open fail");
        return;
    }

    int count = 0;
    if(fscanf(fp,"%d",&count) !=1)//第一行为%d，返回值为应为1
    {
        printf("文件格式错误或为空！");
        fclose(fp);
        fp = NULL;
        return;
    }
    Vehicle *cars = (Vehicle*)malloc(count * sizeof(Vehicle));
    if(cars==NULL)
    {
        perror("内存开辟失败！");
        fclose(fp);
        fp = NULL;
        return;
    }

    //读取车辆信息
    printf("count %d\n",count);
    if(count!=3)//读取第一行后
    {
        perror("文件里的车辆数量与程序预设的 3 不符！");
        fclose(fp);
        free(cars);
        fp=NULL;
        return;
    }
    for(int i=0;i<3;i++)
    {
        fscanf(fp,"%s %s %d %d\n",cars[i].plate,cars[i].owner,&cars[i].speed,&cars[i].age);
        printf("第%d台车辆 车牌为：%s 驾驶员名字:%s, 速度为：%d 驾驶人年龄：%d\n",i+1,cars[i].plate,cars[i].owner,cars[i].speed,cars[i].age);
    }
    
    fclose(fp);
    free(cars);
    fp=NULL;

}

int main(void)
{
     // 测试数据：用结构体数组初始化（结构体数组）
    Vehicle garage[3] = {
        {"沪A88888", "张三" ,120 ,30},
        {"苏B66666", "李四" ,130 ,25},
        {"京C12345", "王五" ,150 ,18}
    };

    //写文件
    fun_fprintf_write("garage_data.txt",garage,3);
    //读文件
    fun_fscanf_read("garage_data.txt");

    return 0;
}