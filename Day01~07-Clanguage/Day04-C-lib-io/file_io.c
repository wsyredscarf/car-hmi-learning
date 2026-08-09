#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
**********************************单个字符写和读操作
函数：fputc（），fgetc（）
*/
//只写（w）
void fun_fputc_write(void)
{

    FILE *fp = fopen("c.txt","w");
    if(fp==NULL)
    {
        perror("write fopen fail!");
        return;
    }

    fputc('H',fp);
    fputc('E',fp);
    fputc('L',fp);
    fputc('L',fp);
    fputc('O',fp);
    fputc('\n',fp);//手动换行,光标在下一行，不换行光标在末尾

    fclose(fp);
    fp = NULL;
}
//只读（r）
void fun_fgetc_read(void)
{
    FILE *fp = fopen("c.txt","r");
    if(fp==NULL)
    {
        perror("read fopen fail!");
        return;
    }

    int ch = 0;
    while((ch=fgetc(fp)) != EOF)
    {
        putchar(ch);//打印到终端控制台
    }
    fclose(fp);
    fp=NULL;
}


/*
**********************************字符串的写和读操作
函数：fputs（"字符串"，"文件指针"），fgets（缓冲区，缓冲区大小，文件指针）
*/
//只写（w）
void fun_fputs_write(void)
{
    FILE *fp = fopen("s.txt","w");
    if(fp==NULL)
    {
        perror("write fopen fail");
        return;
    }
    fputs("aaaaaaaaaa",fp);
    fputc('\n',fp);
    fputs("bbbbbbbbbb",fp);
    fputc('\n',fp);
    fputs("cccccccccc",fp);
    fputc('\n',fp);
    fclose(fp);
    fp=NULL;
}
//只读（r）
void fun_pgets_read(void)
{
    FILE *fp = fopen("s.txt","r");
    if(fp==NULL)
    {
        perror("read fopen fail");
        return;
    }
    char buf[128]={0};
    while(fgets(buf,sizeof(buf) ,fp) != NULL)
    {
        printf("读取一次为 %s",buf);
    }
    fclose(fp);
    fp=NULL;
}

/*
**********************************fscanf /fprintf 格式化读写操作
函数：fscanf /fprintf 格式化读写
*/
//fprintf 格式化写入（自动拼接换行，替代 fputs）
void fun_fprintf_write(void)
{
    FILE *fp = fopen("f.txt","w");
    if(fp==NULL) return;
    int speed = 100;
    float volt = 12.2f;
    fprintf(fp,"num %d f %.1f\n",speed,volt);
    fclose(fp);
    fp=NULL;
}
// fscanf 读取一行内指定变量（精准截取数字）
void fun_fscanf_read(void)
{
    FILE *fp = fopen("f.txt","r");
    if(fp==NULL) return;
    int speed =0;
    float volt =0;
    fscanf(fp,"num %d f %f\n",&speed,&volt);
    printf("提取速度：%d 电压：%.1f\n",speed,volt);
    fclose(fp);
    fp=NULL;
}
//补充：只读取一行全部字符（%[^\n]）
void read_full_line_by_scanf(void)
{
    FILE *fp = fopen("f.txt","r");
    if(fp==NULL) return;
    char buf[128]={0};
    fscanf(fp, "%[^\n]", buf);// 读取所有非换行字符
    fgetc(fp);// 吃掉残留\n，防止下一次读取空
    printf("读取一行内字符：%s\n",buf);
    fclose(fp);
    fp = NULL;

}

/*
**********************************二进制整块读写 fwrite /fread（结构体 / 数组专用）
函数：fwrite /fread
文本函数会破坏二进制数据，存储车辆参数、结构体必须用这套。
*/
//
typedef struct{

    int speed;//速度
    float volt;//电压
    char model[16];//车型
}Carinfo;

void fun_bin_fwrite(void)
{
    FILE *fp = fopen("car.bin","wb");//b代表二进制
    if(fp==NULL) return;
    Carinfo car = {90,12.5,"SVIP"};
    fwrite(&car,sizeof(Carinfo),1,fp);
    fclose(fp);
    fp =NULL;
}

void fun_bin_fread(void)
{
    FILE *fp =fopen("car.bin","rb");
    if(fp==NULL) return;
    Carinfo tmp;
    fread(&tmp,sizeof(Carinfo),1,fp);
    printf("速度：%d 电压：%f 车型: %s\n",tmp.speed,tmp.volt,tmp.model);
    fclose(fp);
    fp=NULL;

}

/*
*****************************************文件指针偏移
*/
//文件指针偏移 / 定位函数（读写切换、跳转位置必用）
// 1、
// rewind(FILE *fp)
// 文件指针直接回到文件开头，等价 fseek(fp,0,SEEK_SET)

// 2、
// int fseek(FILE *fp, long offset, int whence);
// 自由跳转文件读写位置
// SEEK_SET：从文件开头偏移
// SEEK_CUR：从当前位置偏移
// SEEK_END：从文件末尾偏移（负数向前跳）
// fseek(fp, 10, SEEK_SET);  // 跳到第10字节
// fseek(fp, -4, SEEK_END);  // 跳到倒数第4字节

// 3、
// long ftell(FILE *fp)
// 获取当前指针距离文件开头的字节偏移值，返回当前位置。


int main(void)
{
    //单字符读写
    fun_fputc_write();
    fun_fgetc_read();

    //字符串读写
    fun_fputs_write();
    fun_pgets_read();

    //格式化读写
    fun_fprintf_write();
    fun_fscanf_read();
    read_full_line_by_scanf();

    //结构体文件写和读，二进制
    fun_bin_fwrite();
    fun_bin_fread();
    return 0;
}