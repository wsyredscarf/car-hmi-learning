// C 结构体：只有数据，无函数;     C++ 类：数据 + 操作数据的函数封装一体（封装特性）

// 访问权限三档：
// private（class 默认）：私有，仅类内部可访问，工程成员变量全部放 private
// public              ：公开，外部可直接调用（对外接口 get/set）
// protected：         ：仅自身 + 子类访问，继承章节使用

// 构造函数 & 析构函数规则
// 构造函数：与类同名、无返回值、对象创建自动调用，支持重载
// 析构函数：~类名()、无参数无返回、一个类仅 1 个，对象销毁自动调用，释放堆内存
// new/delete 对比 C malloc/free：new 自动调用构造，delete 自动调用析构

//C++类的使用，其实就是内部对数据和函数的封装、继承、多态、抽象的操作，四大特性都是在数据结构——类的基础上进行的

#include <iostream>
#include <cstring>

using namespace std;

/*******************************************************************************简单继承*/
//类延伸：基类、派生类
    // 基类
class Animal {
    // eat() 函数
    // sleep() 函数
};

//派生类
class Dog : public Animal {
    // bark() 函数
};

/*******************************************************************************多继承*/
//多继承即一个子类可以有多个父类，它继承了多个父类的特性。
//class <派生类名>:<继承方式1><基类名1>,<继承方式2><基类名2>,…
// {
// <派生类类体>
// };

// 基类 Shape
class Shape 
{
   public:
      void setWidth(int w)
      {
         width = w;
      }
      void setHeight(int h)
      {
         height = h;
      }
   protected:
      int width;
      int height;
};
 
// 基类 PaintCost
class PaintCost 
{
   public:
      int getCost(int area)
      {
         return area * 70;
      }
};
 
// 派生类
class Rectangle: public Shape, public PaintCost
{
   public:
      int getArea()
      { 
         return (width * height); 
      }
};
 
// int main(void)
// {
//    Rectangle Rect;
//    int area;
 
//    Rect.setWidth(5);
//    Rect.setHeight(7);
 
//    area = Rect.getArea();
   
//    // 输出对象的面积
//    cout << "Total area: " << Rect.getArea() << endl;
 
//    // 输出总花费
//    cout << "Total paint cost: $" << Rect.getCost(area) << endl;
 
//    return 0;
// }
/*******************************************************************************多态的简单使用*/
// 基类 Animal
// class Animal {
// public:
//     // 虚函数 sound，为不同的动物发声提供接口
//     virtual void sound() const {
//         cout << "Animal makes a sound" << endl;
//     }
   
//     // 虚析构函数确保子类对象被正确析构
//     virtual ~Animal() {
//         cout << "Animal destroyed" << endl;
//     }
// };

// // 派生类 Dog，继承自 Animal
// class Dog : public Animal {
// public:
//     // 重写 sound 方法
//     void sound() const override {
//         cout << "Dog barks" << endl;
//     }
   
//     ~Dog() {
//         cout << "Dog destroyed" << endl;
//     }
// };

// // 派生类 Cat，继承自 Animal
// class Cat : public Animal {
// public:
//     // 重写 sound 方法
//     void sound() const override {
//         cout << "Cat meows" << endl;
//     }
   
//     ~Cat() {
//         cout << "Cat destroyed" << endl;
//     }
// };

// // 测试多态
// int main() {
//     Animal* animalPtr;  // 基类指针

//     // 创建 Dog 对象，并指向 Animal 指针
//     animalPtr = new Dog();
//     animalPtr->sound();  // 调用 Dog 的 sound 方法
//     delete animalPtr;    // 释放内存，调用 Dog 和 Animal 的析构函数

//     // 创建 Cat 对象，并指向 Animal 指针
//     animalPtr = new Cat();
//     animalPtr->sound();  // 调用 Cat 的 sound 方法
//     delete animalPtr;    // 释放内存，调用 Cat 和 Animal 的析构函数

//     return 0;
// }


/*******************************************************************************类的简单使用*/
// 模拟车载仪表盘车辆信息类，封装车速、电量
class CarInfo
{
// 私有成员：数据隐藏，外部禁止直接读写，封装核心
private:
    int m_speed;    // m_ 前缀区分成员变量，车载项目统一规范
    int m_battery;

// 公开接口：外部仅能通过函数访问私有数据，可加业务校验
public:
    // 【无参构造函数】对象定义自动调用，初始化默认值
    CarInfo()
    {
        cout << "无参构造函数执行，车辆初始化" << '\n';
        m_speed = 0;
        m_battery = 100;
    }

    // 【有参构造函数】构造函数重载，支持创建时直接赋值
    CarInfo(int speed, int battery)
    {
        cout << "有参构造函数执行" << '\n';
        // 业务校验，C结构体无法实现
        if(speed >= 0 && speed <= 220)
            m_speed = speed;
        else
            m_speed = 0;

        if(battery >=0 && battery <= 100)
            m_battery = battery;
        else
            m_battery = 0;
    }

    // 【析构函数】对象销毁自动执行，释放堆资源
    ~CarInfo()
    {
        cout << "析构函数执行!CarInfo对象销毁" << '\n';
    }

    // set接口：修改私有成员，带业务校验
    void setSpeed(int speed)
    {
        if(speed >= 0 && speed <= 220)
            m_speed = speed;
        else
            cout << "车速参数非法!限制0~220" << '\n';
    }

    // const成员函数：只读，不修改成员变量，查询接口标准写法
    int getSpeed() const
    {
        return m_speed;
    }

    void setBattery(int bat)
    {
        if(bat >= 0 && bat <= 100)
            m_battery = bat;
        else
            cout << "电量参数非法! 限制0~100" << '\n';
    }

    int getBattery() const
    {
        return m_battery;
    }

    // 打印整车信息
    void showCarData() const
    {
        cout << "车速：" << m_speed << "km/h | 电量：" << m_battery << "%" << '\n';
    }
};

int main(void)
{
    // 栈对象：出作用域自动调用析构，无需手动释放
    CarInfo car1; // 调用无参构造
    car1.setSpeed(65);
    car1.setBattery(78);
    car1.showCarData();

    CarInfo car2(110, 55); // 调用有参构造
    car2.showCarData();

    // 堆对象：new创建，必须delete释放，否则内存泄漏
    CarInfo *heap_car = new CarInfo(30, 92);
    heap_car->showCarData();
    delete heap_car; // delete触发析构，释放堆内存

    cout << "main函数执行结束,栈对象自动析构" << '\n';
    return 0;
}


