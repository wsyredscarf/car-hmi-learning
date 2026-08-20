#include <iostream>

using namespace std;


// 1. C语言值传递：拷贝新变量，修改不影响外部原数据
void test_value(int arg)
{
    arg = 999;
}

// 2. C语言指针传递：需要取地址&，存在野指针、空指针风险
void test_ptr(int *arg)
{
    if(arg == nullptr) return; // 必须判空，否则段错误
    *arg = 999;
}

// 3. C++引用传递：arg是外部变量别名，操作等价原变量，无需判空//
void test_ref(int &arg)
{
    arg = 999;
}

// 【工程标准写法】const引用：只读传参，避免大对象拷贝，不允许修改数据
void test_const_ref(const int &arg)
{
    // arg = 100; // 编译报错，const保护数据只读,arg这里不准修改 ，仍然是传进来的300
    cout << "只读参数：" << arg << '\n';
}

int main(void)
{
    int num = 10;
    test_value(num);
    cout << "值传递后 num = " << num << '\n'; // 输出10，无修改

    test_ptr(&num);
    cout << "指针传递后 num = " << num << '\n'; // 输出999

    num = 10;
    test_ref(num);
    cout << "引用传递后 num = " << num << '\n'; // 输出999

    // 基础引用定义
    int x = 200;
    int &rx = x; // rx是x的别名
    rx = 300;
    cout << "x = " << x << '\n'; // x被修改为300

    test_const_ref(x);
    return 0;
}


// int fun_const(void)
// {
//     const int MAX_SPEED = 220;
//     // MAX_SPEED = 300; // 直接编译报错，禁止修改

//     // C++特有：const常量可定义数组长度，C语言不支持
//     int car_speed_arr[MAX_SPEED] = {0};

//     // const修饰指针三种写法（高频考点）
//     int num = 100;
//     const int *p1 = &num; // *p1只读，p1可换指向
//     int const *p2 = &num; // 等价p1
//     int *const p3 = &num; // p3指针本身固定，*p3可修改

//     return 0;
// }