#include <stdio.h>
#include <stdlib.h>
#include <string.h>



//-------------------------------------------------

//枚举变量sizeof永远等于 4（int），和枚举项数量无关。

enum{a,b,c,d,};
//sizeof(enum)永远是4字节（int类型）
//不初始化则从0开始，累加


//-------------------------------------------------
/*
 *结构体求大小*
*/
//最大基础值4（int），10+2=12，是int的整数倍
struct Ss1
{
    char c1;//1
    char c2;//1+1=2
    int i;//2+2+4=8
    short s;//8+2=10
};

//最大基础值为4（int类型）、结构体值为12
struct Inner1{
    char c;//1
    int arr[2]; //int数组，2个int
};

//2和结构体基础值对比填充为4，4+8（最大值）+3=15+5=20
struct Test1{
    short s;//2                       2和下面的最大基础值填充对比为4
    struct Inner1 in;//算出可知：对齐值：4，最大值是8      
    char buf[3];//3
};


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

//char值与最大基础值比：1比8变为8，加上结构体值8+24=32，32+4=36  因为是最大基础值为8，所以36+4=40（40%8=0）
struct Test2{
    char ch;
    struct Inner2 in;
    int val;
};

struct S10
{
    char c;
    int arr[2];
    char buf[7];
    long l;
};
struct Sub3
{
    short arr[3];
    char buf[3];
};
struct S3
{
    char ch;
    long l;
    struct Sub3 sub;
};

struct S4
{
    short s;
    char c1[3];
    char c2[2];
    int arr[1];
    short end;
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
};

struct B2
{
    unsigned int a :16;
    unsigned int b :17;
};

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
1+1+1=3，3不是2的倍数，所以3+1=4，4是变量s的偏移量2的倍数，所以符合。4+2（这个2是short大小）=6。 6是l的偏移量的整数倍，所以直接加6+8=14
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
#pragma pack(1)
struct Sub6
{
    char c;    //1
    short s;   //2
}; //子结构体总和 3。pack（1）时，直接类型大小相加，因为偏移量都为1，都符合整数倍
#pragma pack()   // 恢复默认对齐

struct S6
{
    char ch;            //1
    struct Sub6 sub;    //3
};
//---------------------------------------------
#pragma pack(2)
struct T1
{
    char c;
    long l;
    short s;
};
#pragma pack()

#pragma pack(2)
struct T4
{
    char c1;
    char c2[2];
    int a;
    short s;
};
#pragma pack()

#pragma pack(4)
struct T11
{
    char c1;
    char c2[2];
    int a;
    short s;
};
#pragma pack()   // 恢复默认对齐


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
    int a;       //min(4,2)=2，offset5是奇数，填充1→6；占4 →offset10
};
#pragma pack()   // 恢复默认对齐

#pragma pack(2)
struct S15
{
     long b;
    char a;
   
};
#pragma pack()

#pragma pack(2)
struct S7
{
    char a;
    char b;
    char c;
};
#pragma pack()


//-------------------------------------------
/*union 共用体
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

//字节最大为7
#pragma pack(2)
union Ub
{
    char buf[7];
    int i;
};
#pragma pack()

#pragma pack(2)
struct S
{
    char a;         // offset 0，占1字节
    unsigned int b :3; //位段，容器unsigned int(4字节)。A=min(4,2)=2。char后面补1padding到2，开启4字节容器。
    short c;        //short对齐 min(2,2)=2
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
    unsigned int c : 12;    // 位域 c，占 12 bit
    union {
        char d[5];          // 数组 5 个字节
        int e;              // 整型 4 个字节
    } u;
};
#pragma pack()
/*
解析：pack（2），10+14=24，32-24=8，8不够c12存，所以单独开辟一个单元，所以ab是4字节，c是4字节
     union内部，char为1字节（1，2）=1。int为4字节（4，2）=2.注意此处与pack做对比是类型大小去做对比取对比值，不是数组大小或者其他的。
     所以：union对外对齐值为2，大小还是为5。
     （4，2）=2，（4，2）=2，（4，2）=2。 所以：4+4=8，8%2=0.所以不用填充，8+5=13，结构体整体对齐值 = min(最大成员 int, pack2) = 2
     所以13+1=14%2=0，最终为14。这里原本是对齐值是最大成员变量4的，因为最大成员变量的有效对齐值是2，所以如此计算
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


//-------------------------------------------------
int main(void)
{
    printf("%ld\n",sizeof(struct Ss1));//
    printf("%ld\n",sizeof(union Ub));//
    printf("%ld\n",sizeof(struct B1));//
    printf("%ld\n",sizeof(struct T5));//
     printf("%ld\n",sizeof(struct sss1));//
    return 0;

}