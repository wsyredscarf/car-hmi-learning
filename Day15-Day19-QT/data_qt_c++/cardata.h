#ifndef CARDATA_H
#define CARDATA_H

#include <QObject>

// 必须继承QObject，才能使用信号槽、Q_PROPERTY
class CarData : public QObject
{
    // 【重中之重】启用Qt元对象系统，没有这个宏：信号、Q_PROPERTY全部失效，编译报错
    Q_OBJECT

    // Q_PROPERTY格式：类型 对外名称 READ 读函数 WRITE 写函数 NOTIFY 数据变更信号
    // 作用：把C++的m_speed变量暴露给QML读写，数据变化自动发信号刷新UI
    Q_PROPERTY(int speed READ getSpeed WRITE setSpeed NOTIFY speedChanged)
    // CONSTANT：电量固定不变，不需要写接口、不需要变更信号
    Q_PROPERTY(int battery READ getBattery CONSTANT)

public:
    // 构造函数，父对象默认空
    explicit CarData(QObject *parent = nullptr);

    // READ绑定的读接口：QML读取g_car.speed时自动调用
    int getSpeed() const;

    // WRITE绑定的写接口：QML滑块修改g_car.speed自动调用
    void setSpeed(int newSpeed);

    // 电量只读接口
    int getBattery() const;

signals:
    // NOTIFY绑定的信号：车速发生合法修改时发送，QML收到自动刷新界面
    void speedChanged(int val);

private:
    int m_speed = 0;    // 私有成员：实际存储车速，外部不能直接访问
    const int m_battery = 76; // 固定电量
};




#endif