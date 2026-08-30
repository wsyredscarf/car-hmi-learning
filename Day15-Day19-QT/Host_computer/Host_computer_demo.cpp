#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>      // 定时器，用来模拟串口周期性发数据
#include <QLabel>      // 标签，显示文字用

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 1. 创建主窗口（QWidget 是空白窗口）
    QWidget window;
    window.setWindowTitle("串口模拟上位机 (打包测试)");
    window.resize(500, 400);

    // 2. 创建控件
    QTextEdit *logArea = new QTextEdit();   // 日志显示区
    logArea->setReadOnly(true);             // 设为只读，防止用户手改日志

    QLineEdit *sendLine = new QLineEdit();  // 用户输入指令的框
    sendLine->setPlaceholderText("在这里输入指令，然后点发送...");

    QPushButton *startBtn = new QPushButton("开始模拟接收");
    QPushButton *sendBtn = new QPushButton("发送指令");

    // 3. 创建定时器（QTimer）用来模拟“不断有串口数据进来”
    QTimer *timer = new QTimer();

    // 4. 布局管理（自动排列位置，不用手算坐标）
    QVBoxLayout *mainLayout = new QVBoxLayout(&window); // 垂直布局

    QHBoxLayout *topLayout = new QHBoxLayout();         // 水平布局
    topLayout->addWidget(startBtn);                     // 把按钮放上面

    QHBoxLayout *bottomLayout = new QHBoxLayout();      // 水平布局
    bottomLayout->addWidget(sendLine);                  // 输入框
    bottomLayout->addWidget(sendBtn);                   // 发送按钮

    mainLayout->addLayout(topLayout);                   // 把上面的排到第一行
    mainLayout->addWidget(logArea);                     // 日志区放中间
    mainLayout->addLayout(bottomLayout);                // 把下面的排到最下面

    // 5. 【核心】连接信号与槽（相当于 C 语言里的回调函数）
    // 点击“开始模拟接收”按钮：如果定时器在跑就停掉，没跑就启动
    QObject::connect(startBtn, &QPushButton::clicked, [=]() {
        if (timer->isActive()) {
            timer->stop();
            startBtn->setText("开始模拟接收");
        } else {
            timer->start(1000); // 参数1000代表：每隔1000毫秒（1秒）触发一次
            startBtn->setText("停止模拟接收");
        }
    });

    // 定时器每触发一次，就自动生成一行模拟数据打印出来
    QObject::connect(timer, &QTimer::timeout, [=]() {
        // 随机生成一个车速 (0~119) 和水温 (25~35)
        int speed = rand() % 120;
        int temp = 25 + rand() % 10;

        // 拼接成字符串，%1 和 %2 是占位符，会被 arg 里的数字替换
        QString data = QString("收到CAN报文 -> 车速: %1 km/h, 水温: %2 ℃").arg(speed).arg(temp);

        // 追加到日志框里
        logArea->append(data);
    });

    // 点击“发送指令”按钮：把你输入框里的字打印到日志框
    QObject::connect(sendBtn, &QPushButton::clicked, [=]() {
        QString text = sendLine->text();
        if (text.isEmpty()) return; // 如果空的就不发
        logArea->append("【我发送的】: " + text);
        sendLine->clear();
    });

    // 6. 显示窗口并进入事件循环（程序在这里一直等）
    window.show();
    return app.exec();
}