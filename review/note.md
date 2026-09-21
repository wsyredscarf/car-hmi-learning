
## 2026.9.21

## 目前为止框架学习思路总结为此，驱动工程师主要为：硬件层和内核驱动层；   应用工程师主要为：系统调用、网络协议套接字io复用、产品业务逻辑、界面开发
根据个人经验的职业规划：
    MCU硬件驱动+软件编写+freertos+各种协议+lvgl-----两年经验
    软件层（server维护）+linux应用开发+lvgl界面开发-----两年经验
    下一步linux应用开发（预计两到三年 2027~2030）
    下一步linux驱动开发（预计三到五年 2031~2035）
    ** Linux应用 → Linux驱动 → 系统架构

【硬件层】 
  └─ 传感器、执行器、WiFi/4G模块、串口芯片、CAN收发器、
       ↓
【内核驱动层】 
  └─ 字符设备（UART、SPI、IIC）、网络设备（eth0、wlan0）、CAN设备
       ↓
【系统调用层 (Linux系统编程)】 
  └─ open/read/write/ioctl、socket、termios、epoll、pthread
       ↓
【网络与协议层 (应用核心)】 
  └─ TCP/UDP、Socket API、IO多路复用(epoll)、Modbus RTU/TCP、MQTT
       ↓
【业务逻辑层】 
  └─ 数据解析（状态机、环形缓冲区）、业务控制、多线程任务调度、数据库
       ↓
【应用层】 
  └─ 网关核心程序、LVGL界面、QT界面、云端通信