#include <iostream>

using namespace std;//命名空间--- 展开std命名空间，简化书写，小工程可用；大型项目不建议全局展开

/*
std::   命名空间
strring  类型
cin >>   一个命令，命令命名空间的一个对象去执行操作
*/
//-std=c++17  编译时用到，使用最新标准
//printf和scanf的类似使用


/*
方式1、命名空间前缀加上，正常输入
*/
// int main(void)
// {
//     std::string input;//string是类型，input是变量，声明一个装再输入内容的容器
//     std::cin >> input;//等待输入，（注意：不输入内容，只回车，空格之类的空字符就会一直等待输入）
//     std::cin >> input;//复用input变量输入
//     return 0;
// }


/*
方式2、命名空间前缀去掉，正常输入
*/
int main(void)
{

    string input;//string是类型，input是变量，声明一个装再输入内容的容器
    int age;
    cout <<"请输入姓名,并按回车" << "\n";
    cin >> input;//等待输入，（注意：不输入内容，只回车，空格之类的空字符就会一直等待输入）

    cout << "请输入年龄,并按回车" << "\n";
    cin >> age;

    cout <<"你输入的姓名是："<< input <<"\n"<< "你输入的年龄是："<< age <<"\n"; 
    return 0;
}
