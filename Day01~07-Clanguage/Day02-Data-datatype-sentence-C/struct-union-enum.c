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

*/
#pragma pack(1)
struct Sub6
{
    char c;    //1
    short s;   //2
}; //子结构体总和 3

struct S6
{
    char ch;            //1
    struct Sub6 sub;    //3
};

//（3，4）=3，（2，4）=2，（8，4）=4 .    A=4对齐模数。变量1不是变量2的倍数，所以1+1+2+2=6，6不是变量3的倍数所以6+2=8+8=16
//pack（n）只是和结构体最大成员变量做对比求A的：A=min（最大成员变量，n）
#pragma pack(4)
struct T5
{
    char buf[3]; //offset0,1,2
    short s;     //对齐2，offset2刚好满足，占用offset2、3
    long l;      //min(8,2)=2，offset4开始占8
};

#pragma pack(2)
struct T1
{
    char c;
    long l;
    short s;
};
#pragma pack()

//结构体尾部对齐 A = min (内部最大基础数据类型, pack 值)不是直接拿 pack 值去对齐尾部！全 char 成员时 A 恒等于 1。
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
#pragma pack(4)
struct N1{
    char c;     //offset0
    short s;    //min(2,4)=2，填充1→offset2，占2 →offset4
    short end;  //offset4，占2 →offset6
};

#pragma pack(2)
struct N2{
    char buf[5]; //0‑4，offset=5
    int a;       //min(4,2)=2，offset5是奇数，填充1→6；占4 →offset10
};

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
union共用体计算三步：
1. M：所有成员占用内存的最大字节数
2. Align：所有成员对齐要求取最大值；有#pragma pack(n)，则Align = min(Align , n)
3. 总大小：把M向上圆整为Align的整数倍
⚠️重点：M是字节大小，Align是对齐，二者不要混在一起做min！
*/
//字节大小为7最大，但是类型大小是short（2）.先找字节最大的，然后再找类型最大的，如果不是最大类型的整数倍，那就直接补齐数倍即为大小。
union U3
{
    char buf[7];
    short val;
};

//字节最大为7，类型最大为int（4）A=（4，2）=2。最大成员大小不是A的倍数，所以7+1=8.是A的整数倍
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
int main(void)
{
    printf("%ld\n",sizeof(struct Ss1));//
    printf("%ld\n",sizeof(union Ub));//
    printf("%ld\n",sizeof(struct B1));//
    printf("%ld\n",sizeof(struct S));//
    return 0;

}