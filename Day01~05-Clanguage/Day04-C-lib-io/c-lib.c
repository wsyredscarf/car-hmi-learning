#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*求长度
****************************************strlen（s）
*/
//手动实现，注意：长度不包括末尾的 '\0'
size_t my_strlen(const char* str)
{
    if(str == NULL) return 0;//注意判空
    size_t len = 0;
    while(str[len] != '\0')
    {
        len++;

    }
    return len;

}
//指针-效率更高的写法
//在 C 语言里，两个指针是可以相减的，但有一个极其严格的前提：这两个指针必须指向“同一块连续的内存区域”（比如同一个数组）
//核心原理（因为 C 语言认识指针类型），编译器知道声明的是 char * 类型的指针。
//比如：char *p = "hello",这里就是五个字节，所以就相当于五个字符的长度，然后返回值是size_t类型，可以强转，无安全影响
size_t my_strlen_p(const char*str)
{
    if(str == NULL) return 0;//注意判空
    const char *p=str;
    while(*p != '\0')
    {
        p++;
    }
    return (size_t)(p - str);
}

/*拷贝，复制
*********************************strcpy(dest,src)
strcpy拷贝过去时，包含\0。 但是strncpy不包含\0，所以需要手动补
*/
//手动实现strcpy，字符串拷贝
char *my_strcpy(char* dest , const char *src)
{
    if(dest == NULL||src == NULL) return NULL;
    char *start=dest;// 保存 dest 最初的地址并返回
    while(*src!='\0')
    {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';//此处和标准的strcpy有差异

    return start;
}
//带n的实现，strncpy，规定好字节大小更安全，但是需要手动补\0
//二、对比 strncat 的 n 含义（完全不一样）
// strncat(dest, src, n)
// n = 最多追加多少个有效字符；
// dest 原本就有合法\0，缓冲区剩余空间开发者自行保证；
// 追加的\0不算在 n 配额里，是额外追加的，不占用 n；
// 循环只负责拷贝 n 个可见字符，所以条件 n>0。
char *my_strncpy(char *dest,const char* src, size_t n)
{
    if(dest==NULL||src==NULL) return NULL;
    char *start = dest;//保存首地址
    while(n>1 && *src!='\0') // 留一个位置给 \0。      注意不要和strncat  n>0搞混了。
    {
        dest++;
        src++;
        n--;
    }
    *dest = '\0';
    return start; // 手动强制补结束符（避开标准库 strncpy 的坑）
}

/*拼接
********************************strcat(dest,src)
//，把src拼接到dest的末尾,拼接会自动补齐\0，需要计算留出一个位置
*/
char* my_strcat(char *dest, const char* src)
{
    char *start= dest;//保存dest的首地址（起始地址），用于返回

    while(*dest != '\0')//寻找dest地址的\0
    {
        dest++;
    }
    while(*src!='\0')//从dest\0位置，开始拷贝src所有字符
    {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';//拼接完成，末尾手动补充\0

    return start; 
}

//strncat,设置最大拼接长度
char* my_strncat(char *dest, const char *src, size_t n)
{
    char *start = dest;
    // 走到dest末尾
    while (*dest) dest++;

    // 最多拷贝n个字符
    while (n > 0 && *src != '\0')
    {
        *dest = *src;
        dest++;
        src++;
        n--;
    }
    // 强制补结束符
    *dest = '\0';
    return start;
}

/*比较
********************************strcmp(dest,src)
//从首字符逐位对比 ASCII 值；出现不同字符：返回 *dest - *src。
结果 > 0：dest 字符串更大
结果 < 0：src 字符串更大
全部字符相等且同时到'\0'，返回 0（两字符串完全相同）
*/
int my_strcmp(char *str1,const char*str2)
{

    while(*str1!='\0'&& *str2!='\0' && (*str1 == *str2))
    {
        str1++;
        str2++;
    }
    
    return *str1-*str2;//// 返回当前两个字符ASCII差值
}
// int main(void)
// {
//     char s1[] = "qnx_hmi";
//     char s2[] = "linux_hmi";
//     char s3[] = "qnx_hmi";

//     int ret1 = my_strcmp(s1, s2);
//     int ret2 = my_strcmp(s1, s3);

//     if (ret1 > 0)
//         printf("s1 > s2\n");
//     else if (ret1 < 0)
//         printf("s1 < s2\n");
//     else
//         printf("s1 == s2\n");

//     if (ret2 == 0)
//         printf("s1 和 s3 完全相等\n");

//     return 0;
// }





int main(void)
{
     // 1. strlen 测试
    char buf1[] = "hello car";
    printf("标准库strlen= %zu ,手写实现strlen= %zu\n",strlen(buf1),my_strlen(buf1));

    //2. strcpy 危险拷贝（无边界） vs strncpy安全拷贝(需要手动补齐\0)
    char dest[10] = {0};
    char src_long[] = "abcdefghijklmnopqrstuvwxyz";
    printf("%ld\n",sizeof(dest));
    //strcpy(dest,src_long);//源数组字符总共24个字节，再加上\0，25个。目标数组10个字节，如果使用，则打印字符乱序，非a开头 // 缓冲区溢出，崩溃风险
    strncpy(dest,src_long,10-1);//或者 strncpy(dest,src_long,sizeof(dest)-1),因为此函数需要手动补齐\0
    dest[sizeof(dest)-1] = '\0';//补齐\0
    printf("字符串拷贝结果：%s\n",dest);

    //strcat，字符串拼接
    char buf2[20] = "auto_";
    strncat(buf2, "dashboard", sizeof(buf2)-my_strlen(buf2)-1);
    printf("拼接后：%s\n", buf2);

    // 4. strcmp 字符串比较
    char s1[] = "qnx";
    char s2[] = "linux";
    if(strcmp(s1, s2) > 0)
        printf("s1 > s2\n");
    else if(strcmp(s1, s2) < 0)
        printf("s1 < s2\n");
    else
        printf("相等\n");

    // 5. char[] 栈可修改 / char* 只读段不可修改对比
    char str_stack[] = "modify_ok";
    str_stack[0] = 'M';
    printf("栈数组修改：%s\n", str_stack);

    const char *str_ro = "read_only";//字符串字面量（常量区），不可修改
    // str_ro[0] = 'X'; // 只读段，运行段错误
    printf("只读字符串：%s\n", str_ro);

    return 0;

}
