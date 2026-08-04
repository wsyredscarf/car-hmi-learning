#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* 指针、二级指针
>表现形式：            类型 *  一级指针例：int * 、 char * void *等等、二级指针例：int**、char**等等
>创建的初始化要求：     创建指针变量时必须初始化（int*p=NULL），否则为空指针或者野指针，很危险，造成dump，卡顿、内存问题等等
>引用方式：             指向某个变量空间。例：指针p->变量空间
>典型用途：访问变量、数组元素，动态内存管理等
*/
void Fun_pointer(void)
{
    printf("\n");
    printf("---------------Fun_pointer-----------\n");
    //一级指针 一级指针存储变量的地址
    int num = 10;
    int *p1 = &num; // p1 存储 num 的地址   int *是类型，可不是*p1存储num地址; x
    //解引用次数：一次 *p1,内存模型：p1 保存 num 的地址，*p1 得到 num 的值
    printf("解一级指针 = %d\n", *p1); // 输出 10
    
    //二级指针 二级指针存储的是一级指针的地址，需要两次解引用 ** 才能访问原始数据。例如：
    int numm = 20;
    int *p2 = &numm; // 一级指针存num变量的地址
    int **p3 = &p2; // 二级指针，存储 p1 的地址
    printf("解二级指针 = %d\n", **p3); // 输出 20

}

/*
二级指针用途：
 内存模型: p2 → p1 → num
 指向对象不同：一级指针指向数据地址，二级指针指向一级指针的地址
 解引用层数：一级指针一次，二级指针两次
 应用场景： 一级指针：直接访问数据、遍历数组   二级指针：管理指针数组、动态二维数组、修改函数外的指针值
示例：二级指针修改一级指针指向，这里 allocate_int 通过二级指针 int **ptr 修改了 p 的指向，实现了在函数内为外部指针分配内存。
*/
void allocate_int(int **ptr, int value) 
{
    printf("\n");
    printf("--------------allocate_int------------\n");
    *ptr = malloc(sizeof(int)); // 修改一级指针的指向
    if (*ptr) 
    **ptr = value;
    printf("ptr=%d\n", **ptr); // 输出 42
}

/*
数组和二维数组、字符数组
*/
void Fun_array(void)
{

// 最简单的数组初始化方法是使用初始化参数列表。这种方法可以直接在数组声明时指定每个元素的初始值。例如：

// 初始化所有元素为0
    // int a[10] = {0};

// 初始化为特定的值
    // int b[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

// 如果初始化列表包含所有元素，可以省略数组长度
    // int c[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

// 另一种初始化数组的方法是使用 for 循环，这种方法可以逐个设置数组元素的值。这种方法的优点是它可以用于任何类型的数组，并且在编译器优化后，其性能并不差。例如：

    // int e[10];
    // for (int i = 0; i < 10; i++) {
    // e[i] = i;
    // }
// memset 函数通常用于初始化字符数组，它可以将一段内存中的所有字节设置为特定的值。由于memset是按字节操作的，所以它特别适合用于字符数组。
// 需要注意的是，memset 对于非字符数组可能不会按预期工作，因为它会将每个字节都设置为相同的值。
    printf("\n");
    printf("---------------Fun_array-----------\n");
    char str[10];
    memset(str, 'a', sizeof(str)-1);//因为栈上是连续的地址，所以这里不手动加'\0',会导致aaaaaaaaaahello world。所以手动加'\0',两种方法（缩小一号）/手动重新补
    str[9] = '\0';
    char arry[12] = "hello world";
    printf("string =%s\n",str);
    printf("string =%s\n",arry);


//两行三列的二维数组，10为第0行第0列、20为第0行第1列、30为第0行第2列、40为第1行第0列、50为第1行第1列、60为第1行第2列
    int arr[2][3] =  {{10,20,30},
                     {40,50,60}};
    printf("arr= %p, %d\n", arr,arr[0][0]);//
    printf("arr[0]  = %p, %d\n", arr[0],arr[1][2]);//
    printf("&arr[0][0] = %p\n", &arr[0][0]);

    printf("arr+1  = %p(跨一行3个int)\n", arr+1);
    printf("arr[0]+1  = %p(跨单个int)\n", arr[0]+1);

}

/*
char [] 数组 和 char * 字符串字面量 区别
*/
void Fun_string(void)
{
    printf("\n");
    printf("------------Fun_string--------------\n");
    char str_buf[] = "test";     //栈内存，可以修改
    char *str_ptr  = "test";     //字符串常量，存rodata只读段,不可修改。（str_ptr为指向字符串常量的地址，地址不可修改）

    str_buf[0] = 'T';
    printf("str_buf = %s\n", str_buf);

    //str_ptr[0] = 'T'; //打开此行 → 段错误！
    printf("str_ptr = %s\n", str_ptr);

    printf("str_buf地址:%p\n", str_buf);
    printf("str_ptr地址:%p\n", str_ptr);

}


/* 指针数组和数组指针，形式展示、字节大小及地址打印 */
void Fun_pointerarray(void)
{
    printf("\n");
    printf("------------Fun_pointerarray--------------\n");
    //拥有五个元素的int类型数组
    int arr[5]={1,2,3,4,5};

    //数组指针：指向长度为5的int数组，接收&arr
    int (*p_arr)[5] = &arr;

    //指针数组：数组内存储5个int*指针
    int *p[5];

    //给指针数组第一个元素赋值，消除野指针
    p[0] = &arr[0];

    printf("arr地址=%p, &arr[0]=%p, sizeof(arr)=%ld\n",arr,&arr[0],sizeof(arr));
    printf("p_arr=%p, *p_arr=%p, (*p_arr)[0]=%d, sizeof(*p_arr)=%ld\n",
    p_arr, *p_arr, (*p_arr)[0], sizeof(*p_arr));
    printf("sizeof(p_arr)=%ld\n", sizeof(p_arr));

    printf("p[0]指针地址=%p ,*p[0]数值=%d,sizeof(p)= %ld\n", p[0], *p[0],sizeof(p));

    printf("arr      = %p\n", arr);
    printf("arr + 1  = %p\n", arr + 1);   // +4字节
    printf("&arr     = %p\n", &arr);
    printf("&arr + 1 = %p\n", &arr + 1);  // +20字节

}

int main(void)
{
    
    //指针和二级指针
    Fun_pointer();

    //数组和二维数组、字符数组
    Fun_array();

    //指针数组和数组指针
    Fun_pointerarray();
    
    //字符串字面量和字符串数组
    Fun_string();

    //二级指针应用
    int *pp = NULL;
    allocate_int(&pp, 42);
    free(pp);

    return 0;

}