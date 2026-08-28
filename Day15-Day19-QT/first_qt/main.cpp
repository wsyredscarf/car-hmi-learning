//引入头文件
#include <QApplication>//qt的核心类，没有他，qt程序无法运行，主要负责键盘鼠标事件、窗口调度管理等
#include <QWidget> //所有窗口控件的基类。相当于一个空白的画布
#include <QPushButton>//按钮控件类，继承Qwidget
#include <QLabel>//文本显示标签类，用来显示文字--类似于lvgl的lv_label
#include <QVBoxLayout> //垂直布局管理，自动把空白画布上的控件从上到下垂直排列，不用手动计算坐标


//main，程序入口，argc命令行参数的个数，不传参数默认是1；argv命令行参数的字符串数组
int main(int argc, char *argv[])
{

    //初始化qt应用程序
    //创建QApplication对象，必须且只能有一个
    //参数：主函数的（argc,argv传进去），qt才能处理窗口系统相关的配置,相当于启动程序
    QApplication app(argc,argv);

    //创建窗口对象，注意：这里是栈区上创建的（没有用new创建），程序结束会自动释放
    QWidget window;
    window.setWindowTitle("哈喽,我第一个qt程序创建成功了!");//类似lvgl的set_title(“text”)；只不过c++可以通过 . 调用函数
    window.resize(600,300);//给与标题设置宽和高，单位是像素点

    //创建内部控件
    QLabel *label = new QLabel("点击后，果然实现了跳转显示");//创建文本，注意用用new在堆上创建的
    QPushButton *btn = new QPushButton("点我一下试试");//创建按钮，注意用new在堆上创建的

    //布局管理，自动布局排位
    //参数：&window：告诉这个布局，本页面属于这个窗口的，请自动填满
    //qt的独有的核心机制：布局管理器的父对象是&window
    //父对象销毁时，会自动删除所有绑定的子对象，这样避免忘记手动释放，造成内存泄露问题
    QVBoxLayout *layout = new QVBoxLayout(&window);
    layout->addWidget(label);
    layout->addWidget(btn);
    
    //信号与槽：当各种事件信号触发时，执行后面的槽函数
    //[=](){}  表示捕获外部所有变量副本，保证在匿名函数里能用
    QObject::connect(btn,&QPushButton::clicked,[=](){
        label->setText("你点了我");

        // if(label->text() == "你好,世界!!!"){
        //       label->setText("hello,world!!!");
        // }else
        // {
        //     // 调用标签的setText函数，修改标签显示的文字。
        //     label->setText("你好,世界!!!");
        // }

    });

    //显示与事件循环
    //窗口默认是隐藏的，必须显式的调用show().才会弹出来
    window.show();

    //核心，app.exec()，会进入一个无限循环（事件循环）,程序像一个监听器一样，不停的监听，等待外部触发的事件，只有点击右上角X号才会结束
    return app.exec();

}
