#include <iostream>//头文件,C++98以后不用带.h后缀

using namespace std;////命名空间--- 展开std命名空间，简化书写，小工程可用；大型项目不建议全局展开

/*
std::      ---命名空间，类似文件夹，存放不同文件，防止和其他文件夹内文件重名
cout <<    ---命令命名空间里的一个指令去执行一个动作。<<就是输出动作
“”         ---引号内是需要输出的内容
"\n:"=endl ---换行符
*/

// /*
// 写法1，命名空间std::这样使用
// */
// int main(void)
// {
//     cout <<"hello world!"<< endl;//  \n = endl
//     return 0;
// }


/*
写法2，\n = endl 这样使用
*/
int main(void)
{
    cout <<"hello world!"<<"\n";//  \n = endl
    return 0;
}


// /*
// 写法3，命名空间std::这样使用
// */
// int main(void)
// {
//     std::cout <<"hello world!"<<"\n";
//     return 0;
// }