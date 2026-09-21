/*
数据结构
    |——基本数据类型
        |——字符型char、短整型short、整型int、长整型long、浮点型 ：单精度float、双精度double
    |——构造类数据类型
        |——数组、结构体、指针、函数、链表（学好后，可以深入更复杂的图和树）
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/*
结构体struct、共用体union、枚举enum
*/
//-------------------------------------------------

//枚举变量sizeof永远等于 4（int），和枚举项数量无关。
enum{a,b,c,d,};
//sizeof(enum)永远是4字节（int类型）
//不初始化则从0开始，累加

//-------------------------------------------------
/*
 *结构体求大小*
*/
//1
//最大基础值4（int），10+2=12，是int的整数倍
struct Ss1
{
    char c1;//1
    char c2;//1+1=2
    int i;//2+2+4=8
    short s;//8+2=10
};


//2
//最大基础值或者叫对比值为4（int类型）、结构体值为12
struct Inner1{
    char c;//1
    int arr[2]; //int数组，2个int
};

// 2和结构体基础值对比填充为4，4+12（最大值）=16，16+3=19，19+1=20,20是最大基础变量4的整数倍。所以结构体大小是20
struct Test1{
    short s;//2                       2和下面的最大基础值填充对比为4
    struct Inner1 in;//算出可知：对齐值：4，最大值是12
    char buf[3];//3
};


//3
//最大基础值8，结构体值为16
struct Sub{
    long l;
    char c;
};

//short的值与最大基础值比：2和8比变为8，加上结构体值8+16=24
struct Inner2{
    short s;
    struct Sub sub;
};

//char值与最大基础值比：1比8变为8，加上结构体值8+24=32，32+4=36  因为是最大基础值为8，所以36+4=40（40%8=0）取余
struct Test2{
    char ch;
    struct Inner2 in;
    int val;
};


//-------------------------------------------------
/*
结构体位段
unsigned int a:N：N 是 bit 位数，存储容器是 unsigned int（4 字节 32bit），容器大小不会缩小，哪怕只用很少 bit。
连续位段：尽量塞进同一个容器；bit 不够，就开辟下一个同类型容器。
遇到普通变量（char/short/int），直接开启新存储单元，不会复用前面容器剩下的 bit 缝隙。
位段结构体整体尾部对齐，和普通 struct 一样：A = min(内部最大基础类型，pack值)，向上补齐 A 倍数。

*/
struct B1
{
    unsigned int a : 3;
    unsigned int b : 5;
};//3+5bit=8bit，所以一个字节int，为4字节大小结构体

struct B2
{
    unsigned int a :16;
    unsigned int b :17;
};//16+17=33>32，所以是两个四字节，所以是8字节

#pragma pack(2)
struct B3
{
    unsigned int x : 10;
    unsigned int y : 10;
};
#pragma pack()



//-------------------------------------------------
/*
#pragma pack(n)
1. 成员对齐值 = min(类型字节长度，n)
2. 结构体整体对齐值 = min(结构最大基础类型长度，n)
3.pack（1）直接所有数据类型大小相加即可不用填充。pack（2/4/8）需要填充，且必须是强制对齐值的整数倍
#pragma pack() //恢复默认
每个成员自己的对齐模数（摆放每个变量时用）       成员对齐 = min( 当前这个成员自身字节 , pack(n) )
结构体【尾部】整体对齐模数（全部成员摆完之后才用）整体对齐A = min( 整个结构体里最大基础类型字节 , pack(n) )

******#pragma pack (N) 的作用：把每一个成员的对齐强制限制为 min(原生对齐, N)
******结构体【整体对齐值】= 结构体所有成员经过 min 之后的对齐里面的最大值，不再看成员原始原生对齐。

结果在 C 语言内存对齐的语境中，最标准、最专业的日常叫法有两个：
结构体整体对齐值（最常用）
结构体有效对齐值（或叫 对齐边界、对齐模数）


*/

/*---------------------------------------------
下题详细解析：
n=2要求，然后第一个变量c1为1字节，
列出来每个成员和要求的对齐值对比：
（1，2）=1  取小得1偏移量值（或者叫有效对齐值）
（1，2）=1  取小得1偏移量值（或者叫有效对齐值）
（4，2）=2  最大类型和强制对比值，最终得到偏移量值2（或者叫有效对齐值）
（2，2）=2  取小得2偏移量值（或者叫有效对齐值）
开始：第一个变量偏移量为1，1%1=0，符合。第二个偏移量为1，1%1=0，也符合。前两个大小 1+1=2，2去加第三个时（int=4），
但是强制pack（2）或者说是有效偏移值为2，所以前两个大小之和不用填充2变为4（2+2=4，4%4=0，4+4=8），不用填充地址，
前两个变量之和和第三个变量得偏移量刚好 2%2=0，所以直接相加2+4=6。最后一个变量偏移量为2，6%2=0，所以6+2=8

*/
#pragma pack(2)  // 强制 2 字节对齐（结果为8），取消自动填充（自动填充结果为10）
struct sss1 {
    char c1;
    char c2;
    int i;
    short s;
};
#pragma pack()   // 恢复默认对齐

//---------------------------------------------
/*
带数组的解析：
先列取对比值偏移量
（1，4）=1，（1，4）=1，（1，4）=1，（2，4）=2，（8，4）=4
得结果：1+1+1=3，short有效偏移量为2，所以前面基数（3不行，补1为4），4是偏移量2的倍数，所以可以偏移，类型大小再加上short变量大小2=6
6不是long的偏移量4的倍数，所以6+2=8，8是4的整数倍，所以加上最后一个变量大小8+8=16(强制对齐4的要求)，凑巧不强制也是16

如果pack（2）的话，即：
（1，2）=1，（1，2）=1，（1，2）=1，（2，2）=2，（8，2）=2
1+1+1=3，3不是2的倍数，所以3+1=4，4是变量s的偏移量2的倍数，所以符合。4+2（这个2是short大小）=6。 6是2的偏移量的整数倍，所以直接加6+8=14
*/

#pragma pack(4)
struct T5
{
    char buf[3]; //offset0,1,2，其实就是三个（1，4）=1，
    short s;     //对齐2，offset2刚好满足，占用offset2、3
    long l;      //min(8,2)=2，offset4开始占8
};
#pragma pack() // 恢复默认对齐

//---------------------------------------------
//嵌套的解析
//Sub6对齐值是1、char的1是对齐值1的倍数（1,1）=1，short 2（1,2）=1也是short对齐值的倍数，所以不用补齐，对齐值就是1直接相加直接1+2=3
#pragma pack(1)
struct Sub6
{
    char c;    //1
    short s;   //2
}; //子结构体总和 3。pack（1）时，直接类型大小相加，因为偏移量都为1，都符合整数倍
#pragma pack()   // 恢复默认对齐

struct S6//char的1，和Sub6的对齐值1和最大值3.char1与对齐值是整数倍，所以不用补齐，直接1+最大值3=4.所以S6结构体大小为4字节
{
    char ch;            //1
    struct Sub6 sub;    //3
};
//---------------------------------------------

#pragma pack(4)
struct N1{
    char c;     //offset0
    short s;    //min(2,4)=2，填充1→offset2，占2 →offset4
    short end;  //offset4，占2 →offset6
};
#pragma pack()   // 恢复默认对齐

#pragma pack(2)
struct N2{
    char buf[5]; //0‑4，offset=5
    int a;       //min(4,2)=2，offset5是奇数，填充1→6；占4 →offset10,10是a的偏移量2的整数倍，所以结构体大小为10
};
#pragma pack()   // 恢复默认对齐

//---------------------------------------------------不应该是6吗，为啥是4，因为unsigned int b :3只开辟了一个字节
#pragma pack(2)
struct S
{
    char a;         // offset 0，占1字节
    unsigned int b :3; //位段，容器unsigned int(4字节)。A=min(4,2)=2。char后面补1padding到2，开启4字节容器。
    short c;        //short对齐 min(2,2)=2
};
#pragma pack()

//-------------------------------------------
/*union 共用体，共同使用同一块内存
1、union（共用体）本身永远不做尾部补齐
2、union（共用体）外部有结构体，只对外展示对齐值和最大值，让结构体最后一步看最大类型时、再尾部补齐
3、⚠️重点：M是字节大小，Align是对齐，二者不要混在一起做min！
*/
//字节大小为7最大，
union U3
{
    char buf[7];
    short val;
};

//字节最大为7,7不是实际对比值2的倍数，所以7+1=8
#pragma pack(2)
union Ub
{
    char buf[7];
    int i;
};
#pragma pack()


//-------------------------------------------------
/*
“终极缝合怪”大题
把结构体、共用体、位域、内存对齐（带 pack 和不带 pack）全部揉在一起。
*/
//一、在 #pragma pack(2) 的压缩环境下，求以下结构体的大小。
#pragma pack(2)
struct TestA {
    unsigned int a : 10;    // 位域 a，占 10 bit
    unsigned int b : 14;    // 位域 b，占 14 bit
    unsigned int c : 12;    // 位域 c，占 12 bit（根据GCC-linux下规则，a和b的字节单元多填充c的8位，c此处剩余4位 只分配2字节位进行保存）
    union {
        char d[5];          // 数组 5 个字节
        int e;              // 整型 4 个字节
    } u;
};
#pragma pack()
/*
解析：pack（2），10+14=24，32-24=8，8不够c12存，所以单独开辟一个单元，所以ab是4字节，c是4字节

     union内部，char为1字节（1，2）=1。int为4字节（4，2）=2.注意此处与pack做对比是类型大小去做对比取对比值，不是数组大小或者其他的。
     所以：根据union对外对齐值规则此共用体最大值为2，字节大小还是为5。
        union 最大成员字节 = 5 → union 原始大小 5
        union 对外对齐模数 = max (min (1,2), min (4,2) ) = 2
        union 自身尾部补齐到对齐模数 2 的倍数：5%2=1，补 1 字节 → union 整体占 6 字节

     （4，2）=2，（4，2）=2。 所以：4+4=8，8%2=0.所以不用填充，8+5=13，结构体整体对齐值 = min(最大成员 int, pack2) = 2
     所以13+1=14%2=0，最终为14。这里原本是对齐值是最大成员变量4的，因为最大成员变量的有效对齐值是2，所以如此计算

     GCC 会缩小位域存储单元至 pack 指定的 2 字节粒度，不再强制使用完整 4 字节容器，节省空间，最终结构体大小 12。

    union成员之间：不填充；
    union自己最后：必须补齐到自身对齐模数整数倍；
    struct 内部放成员（包含 union）：成员前面按需填充；
    struct整体最后：必须补齐到全结构最大对齐模数整数倍。


*/


//二、不带 pack求以下结构体的大小。
union U {
    double d;               // 8 个字节，要求 8 对齐
    unsigned int bits : 20; // 位域，占 20 bit（底层按 unsigned int 4字节单元存放）
};

struct TestB {
    char c;                 // 1 个字节
    union U u;              // 共用体 u
    short s;                // 2 个字节
};
//解析：union内部：最大类型是8字节，对外对齐值也是8（union 的对外对齐 = 它所有成员「原生对齐」里面的最大值）
//c偏移量为1，不是8的倍数，填充7=8，8+8=16，16是2的倍数，16+2=18。但是18不是成员最大类型的整数倍，18+6=24



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
    //注意sizeof和strlen的区别----------
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

int main(int vrgc, char*vagv[])
{
    //基础数据类型-------------------------------------------
    char str='A';
    short nu= 10;
    int num=10;
    long longnum = 1000;
    float t = 3.1415;
    double T = 3.1015926;

    //构造类数据类型------------------------------------------
    //数组和二维数组、字符数组
    Fun_array();
    
    //指针和二级指针
    Fun_pointer();

    //指针数组和数组指针
    Fun_pointerarray();

    //字符串字面量和字符串数组
    Fun_string();

    //二级指针应用
    int *pp = NULL;
    allocate_int(&pp, 42);
    free(pp);

    //结构体struct、共用体union、枚举enum
    printf("s6-------%ld\n",sizeof(struct S6));//
    printf("S-------%ld\n",sizeof(struct S));//
     printf("TestA-------%ld\n",sizeof(struct TestA));//

    return 0; 
}
