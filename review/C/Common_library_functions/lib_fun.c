/*
C函数库常用函数
    |——字符拼接与拷贝
        strlen(buf):求长度，返回值长度大小，不带\0。

        strcpy(char*dest,char*src):字符串拷贝，包含\0。
        strncpy(char*dest,char*src,size_t n):不包含\0,末尾需要自己添加，n为有效字节数

        strcat(char *dest ,char * src):包含\0。
        strncat(char *dest ,char*sc,size_t n):不包含\0

        strcmp(char *dest ,charr *src):返回值int类型，等于0则相等，大于0则dest大，小于零则dest小
    |——输入输出
        printf
        sprintf
        snprintf
        scanf
        fscanf
    |——文件读写操作
        fopen / fclose
        fgets/fputs
        fget/fput
        fread / fwrite（二进制读写）
    |——类型转换
        atoi/itoa
        atof
        strtol
    |——内存分配与转换
        memset
        malloc
        free
        memcpy
        memcmp
        memmove
    |——时间
        time / localtime
    |——调试与处理
        assert
        perror
    |——随机数与数学函数
        rand / srand
        sqrt / pow / fabs


*/
/*
strchr
char *strchr(const char *s, int c);
查找字符第一次出现位置。
例：char *p = strchr("hello", 'e');

strstr
char *strstr(const char *haystack, const char *needle);
查找子串。
例：char *p = strstr("hello world", "world");

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//字符拼接与拷贝函数--------------------------------------------------------------------------------
/*
//1、strlen库函数使用与模仿库函数。此函数为  --->求长度
//特点：返回长度，不包含 /0
//指针更高效的写法 
//在 C 语言里，两个指针是可以相减的，但有一个极其严格的前提：这两个指针必须指向“同一块连续的内存区域”（比如同一个数组）
//核心原理（因为 C 语言认识指针类型），编译器知道声明的是 char * 类型的指针。
//比如：char *p = "hello",这里就是五个字节，所以就相当于五个字符的长度，然后返回值是size_t类型，可以强转，无安全影响
*/
size_t my_strlen(const char* str)
{
    if(str == NULL) return 0;
    size_t len = 0;
    while(str[len] != '\0')
    {
        len++;
    }
    return len;
}
size_t my_strlen_p(const char *str)
{
    if(str == NULL) return 0;
    const char *p= str;
    while(*p != '\0')
    {   
        p++;
    }
    return (size_t)(p-str);
}

/* 
//2、strcpy库函数使用与模仿库函数。此函数为  --->拷贝
//strcpy（A，B）包含\0,strncpy不包含\0所以需要手动补
*/
char *my_strcpy(char*dest ,const char *src)//此函数等于strcpy(char*dest,char*src)
{
    if(dest == NULL || src == NULL) return NULL;
    char *start = dest;//保留原始地址并返回
    while(*src != '\0')
    {
        *dest = *src;
        dest++;
        src++;

    }
    *dest = '\0';//补充\0
    return start;
}
char *my_strncpy(char *dest, char*src,size_t n)
{
    if(dest == NULL || src ==NULL|| n==0) return NULL;
    char *start = dest;
    while(n>1 && *src!='\0')//n>1 留一个空位给\0
    {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
    return start;
}
// 严格模拟标准库 strncp:
char *my_strncpy_std(char *dest, const char *src, size_t n)
{
    char *start = dest;
    while (n > 0 && *src != '\0') {
        *dest = *src;
        dest++;
        src++;
        n--;
    }
    while (n > 0) {   // 剩余空间补 '\0'
        *dest = '\0';
        dest++;
        n--;
    }
    return start;
}



/*
//3、字符串拼接strcat()包含\0.  strncat(char *dest ,char *src,size_t n)不包含\0
*/
char *my_strcat(char *dest ,const char *src,size_t n)
{
    if(dest ==NULL || src== NULL) return NULL;
    char *start = dest;
    while(*dest != '\0')//先找到末尾的\0,走到末尾，，拼接时直接在末尾的位置，而不是\0。不找的话直接覆盖\0可能会出问题
    {
        dest++;
    }
    while(n>0 && *src != '\0')
    {
        *dest = *src;
        dest++;
        src++;
        n--;
    }
    *dest = '\0';
    return start; 
}
char * my_strncat(char *dest,const char*src ,size_t n)
{
    if(dest==NULL || src ==NULL) return NULL; 

    char *start =dest;
    while(*dest != '\0')
    {
        dest++;
    }
    while(n>0 && *src != '\0')
    {
        *dest = *src;
        dest++;
        src++;
        n--;
    }
    *dest = '\0';
    return start ;
}

/*
strcmp(char *dest, char *src),字符串比较
//从首字符逐位对比 ASCII 值；出现不同字符：返回 *dest - *src。
结果 > 0：dest 字符串更大
结果 < 0：src 字符串更大
全部字符相等且同时到'\0'，返回 0（两字符串完全相同）
*/
int my_strcmp(char *dest ,char*src)
{
    while(*dest!= '\0'&& *src!= '\0'&& (*dest==*src))
    {
        dest++;
        src++;
    }
    return *dest- *src;
}
// int my_strcmp(const char *str1, const char *str2)更标准写法
// {
//     const unsigned char *p1 = (const unsigned char *)str1;
//     const unsigned char *p2 = (const unsigned char *)str2;

//     while (*p1 && (*p1 == *p2)) {
//         p1++;
//         p2++;
//     }

//     return (int)*p1 - (int)*p2;
// }


//输入输出--------------------------------------------------------------------------------
/*

\\字符串格式化-------
//printf
int printf(const char *format, ...);
格式化输出到标准输出。
例：printf("n=%d\n", n);

//sprintf（不安全，少用）不检查缓冲区大小，容易溢出
char buf[64];
sprintf(buf, "x=%d, y=%d", 1, 2);
buf = "x=1, y=2"

//snprintf
int snprintf(char *str, size_t size, const char *format, ...);
安全格式化输出到字符串。
例：snprintf(buf, sizeof(buf), "n=%d", n);

//scanf
int age;
char name[20];
scanf("%d", &age);        // 输入整数，必须加 &
scanf("%19s", name);      // 输入字符串，不加 &，限制最多19字符
%s 遇到空格停止；字符串数组名本身是地址。

//fscanf
FILE *fp = fopen("data.txt", "r");
if (!fp) { perror("fopen"); return -1; }
int id;
char name[20];
float score;
while (fscanf(fp, "%d %19s %f", &id, name, &score) == 3)//：检查返回值，等于成功匹配的项数。
{
    printf("%d %s %.1f\n", id, name, score);
}
fclose(fp);
*/


/*
类型装换---
//atoi
int atoi(const char *nptr);
字符串转 int。
例：int n = atoi("123");
注意：无错误检测，推荐 strtol。

//atof
double atof(const char *nptr);
字符串转 double。
例：double d = atof("3.14");

//strtol
long int strtol(const char *nptr, char **endptr, int base);
字符串转 long，可指定进制，可检测错误。
例：char *end; long v = strtol("0xFF", &end, 16); // 255
*/


/*
内存分配与转换---
//memset
void *memset(void *s, int c, size_t n);
把内存块前 n 字节设为 c。
例：memset(buf, 0, sizeof(buf));

memcpy
void *memcpy(void *dest, const void *src, size_t n);
内存拷贝。
注意：不处理重叠内存。

memmove
void *memmove(void *dest, const void *src, size_t n);
内存拷贝，处理重叠。
例：memmove(buf+1, buf, 10);

memcmp
int memcmp(const void *s1, const void *s2, size_t n);
比较内存块。相等返回 0。

malloc
void *malloc(size_t size);
申请堆内存。
例：int *p = malloc(10 * sizeof(int));
注意：检查返回值，用完 free。

free
void free(void *ptr);
释放堆内存。
注意：free 后指针置 NULL，防止野指针。
*/


int main(int vrgc,char vrgv[])
{
    //字符拼接与拷贝函数---------
    char buf[] = "hello";
    printf("库函数strlen求长度= %zu,\n 自写普通my_strlen求长度= %zu, 自写指针my_strlen_p求长度= %zu\n"
        ,strlen(buf),my_strlen(buf),my_strlen_p(buf));

    char str_stack[] = "modify_ok";
    str_stack[0] = 'M';
    printf("栈数组修改：%s\n", str_stack);

    const char *str_ro = "read_only";//字符串字面量（常量区），不可修改
    // str_ro[0] = 'X'; // 只读段，运行段错误
    printf("只读字符串：%s\n", str_ro);

    return 0;
}
