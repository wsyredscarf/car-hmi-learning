import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

Window {
    width: 800
    height: 480
    visible: true
    title: "C++ ↔ QML 车辆数据双向同步Demo"
    color: "#101828" // 车载深色背景

    Column {
        anchors.centerIn: parent
        spacing: 40

        // 1. 展示C++传来的电量（只读，CONSTANT属性）
        Label {
            text: "剩余电量：" + g_car.battery + " %"
            font.pixelSize: 28
            color: "#ffdd44"
        }

        // 2. 实时显示C++车速，信号变更自动刷新，不用手动刷新
        Label {
            text: "当前车速：" + g_car.speed + " km/h"
            font.pixelSize: 30
            color: "#4ee088"
        }

        // 3. 滑动条双向绑定C++车速：拖动滑块 → 自动调用C++ setSpeed
        Slider {
            width: 500
            from: 0
            to: 220
            // value绑定C++的speed，C++数据变滑块自动动；滑块拖动自动修改C++数据
            value: g_car.speed
            onValueChanged: g_car.setSpeed(value)
        }

        // 4. 按钮一键清零车速
        Button {
            text: "重置车速为0"
            width: 280
            height: 55
            font.pixelSize: 22
            onClicked: {
                g_car.setSpeed(0)
            }
        }
    }
}