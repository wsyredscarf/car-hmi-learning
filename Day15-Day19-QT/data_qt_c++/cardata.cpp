#include "cardata.h"

CarData::CarData(QObject *parent) : QObject(parent)
{
}

int CarData::getSpeed() const
{
    return m_speed;
}

void CarData::setSpeed(int newSpeed)
{
    // 业务校验：车速合法区间0~220，非法值直接过滤
    if(newSpeed >= 0 && newSpeed <= 220 && m_speed != newSpeed)
    {
        m_speed = newSpeed;
        // 发送信号，通知所有绑定这个数据的QML界面刷新
        emit speedChanged(m_speed);
    }
}

int CarData::getBattery() const
{
    return m_battery;
}