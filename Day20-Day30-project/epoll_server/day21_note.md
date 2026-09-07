
## 项目：
## 基于Linux的C/C++高并发通信与数据处理服务 

## 项目描述：
  独立完成了底层数据采集端的软件开发与上位机监控工具的对接。底层使用 C 语言在 WSL 中编写epoll服务器，通过 arm-none-linux-gnueabihf-gcc 交叉编译，并使用 rz 命令将程序部署到正点原子 STM32MP157 开发板的真实 Linux 系统中后台运行。上位机使用 C++/Qt 开发，利用 QSerialPort 模块与底层硬件建立 UART 串口通信，实现了数据（车速、水温等）的实时解析与显示，完成了从软件编译、硬件部署到 PC 端可视化的完整闭环。

## 技术栈与工具
    软件
        |——开发语言
            |——C/C++
    硬件
        |—— 正点原子 STM32MP157 (Cortex-A7+M4 双核)
    系统环境	    
        |——WSL (Ubuntu 26.04), Windows
    集成开发环境
        |——Visual Studio Code, Qt Creator
    交叉编译工具链	
        |——arm-none-linux-gnueabihf-gcc (Linaro GCC 9.2) 
    通信方式	
        |——UART 串口 (USB转TTL CH340, 波特率 115200)
    上位机框架	
        |——Qt6 (Qt Widgets, QSerialPort, CMake)


## 项目流程实现
第一步：纯软件开发（WSL / Ubuntu 环境）
    ## 编写 C 代码：在 WSL 里写好 car_data_sim.c（模拟产生车速/水温数据）与 epoll的单线程高并发的服务器 epoll_server.c。
    ## 配置交叉编译器：安装 arm-none-linux-gnueabihf-gcc，并把它加到环境变量（~/.bashrc）里。
    交叉编译：
    arm-none-linux-gnueabihf-gcc car_data_sim.c -o car_arm
    （核心点：PC 是 x86 架构，板子是 ARM 架构，必须用交叉编译器生成 ARM 指令集的程序。）

第二步：程序移植到真实硬件（开发板端）
    ## 文件传输（串口）：在 MobaXterm（COM口）里敲 rz 命令，把 WSL 里生成的 car_arm与server_arm 传到板子的 Linux 系统里。
    ## 赋予运行权限：chmod +x car_arm（Linux 文件默认没有执行权限）。
    后台常驻运行：./car_arm & disown
    （核心点：加 & 让它后台运行，加 disown 保证断开 MobaXterm 软件时，程序不会被系统杀死。）

第三步：上位机 UI 软件开发（Windows / Qt 环境）
    ## 编写 Qt 串口上位机：用 C++/Qt 写界面，通过 QSerialPort 读取数据。
    ## 配置 CMake 依赖：在 CMakeLists.txt 里添加 SerialPort 模块。
    解决“串口独占”问题（经典坑）：Windows 不允许两个软件同时用 COM6，所以先断开 MobaXterm 串口会话，给 Qt 让路。

第四步：软硬件合体（最终闭环）
    ## 打开 Qt 软件：选择对应 COM 口，设置波特率 115200，点击“打开串口”。
    ## 真实 ARM 板子上的数据，通过串口物理线，飞进了电脑 Qt 界面，数据打印实现

## 掌握程度-个人
    熟悉：
        ## 软硬结合架构：独立完成了从底层 C 语言数据采集、Linux 环境交叉编译、ARM 板卡真实部署，到上位机 Qt UI 交互的完整全栈闭环。

        ## 环境与工具链掌握：熟练配置 WSL、Linaro交叉编译工具链、Makefile/CMake工程管理，并解决了 串口被MobaXterm占用导致的排他性问题 以及nohup/后台运行等真实工程痛点。
         
        ## 进阶扩展：附带 epoll_server.c，可在有网线/热点环境下，通过 TCP 协议将数据由 ARM 板直接发送给 Windows 端，实现高并发网络通信。

    了解：
        qt上位机编写简单可行，复杂上位机实现能力暂时不足
       
    