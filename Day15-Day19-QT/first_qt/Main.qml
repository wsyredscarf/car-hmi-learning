---**  QML 是**描述界面的语言**，长得像 JSON + JavaScript，不是 C++。
---**  格式：`控件名 { 属性: 值; 子控件 }`
    基础控件：`Window、Text、Button、Rectangle`
    布局：`Column垂直、Row水平、anchors锚点`
    事件：`onClicked`点击
    `StackView`页面跳转

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

// Window：整个窗口，对应我们的应用窗口，最外层
Window {
    id: mainWin
    width: 800      // 窗口宽度，模拟车机屏幕宽
    height: 480     // 窗口高度，模拟车机屏幕高
    visible: true   // 设置窗口显示出来，不写这个窗口看不见
    title: "极简QML演示窗口"

    // Column：垂直布局，子控件从上往下摆放
    Column {
        anchors.centerIn: parent  // 让整个布局在窗口居中
        spacing:20                // 控件之间间隔20像素

        // Text控件：显示文字
        Text {
            text: "你好，车载HMI"   // 显示的文本
            font.pixelSize:30      // 字体大小
        }

        // Button：按钮控件
        Button {
            text: "点我打印信息"
            font.pixelSize:24

            // onClicked：【重点】按钮点击触发，大括号内是JS代码
            onClicked: {
                console.log("按钮被点击了！") // 在Qt Creator的应用输出打印日志
            }
        }
    }
}