# Day08 学习日志

Linux工具链与系统编程（Day08）
    |——gcc编译四阶段、静态库、动态库编译实操
        |——四阶段
        |——静态库和动态库

    |——预处理宏专项练习
    |——Makefile使用、cmake基础
    |——GDB调试
    |——git的项目管理高级用法

补充：预处理阶段---编译阶段---链接阶段

## 学习内容
一、gcc编译四阶段：-------------------------------------------------------------
    # 1. 预处理 (把 #include 和 #define 展开，生成 .i 文件)
    gcc -E gcc_demo.c -o gcc_demo.i

    # 2. 编译 (把 C 代码转成汇编语言，生成 .s 文件)
    gcc -S gcc_demo.i -o gcc_demo.s

    # 3. 汇编 (把汇编语言转成机器二进制目标文件，生成 .o 文件)
    gcc -c gcc_demo.s -o gcc_demo.o

    # 4. 链接 (把 .o 文件和系统库绑在一起，生成最终可执行程序)
    gcc gcc_demo.o -o gcc_demo

    # 运行最终程序看看
    ./gcc_demo

*******补充*******：
-E (预处理)作用：只做预处理。展开 #include 头文件，替换 #define 宏，处理 #if 条件编译。
    输出：.i 文件。 gcc -E main.c -o main.i
-S (编译)作用：编译成汇编语言。将 C 代码转换成 CPU 能看懂的汇编代码。
    输出：.s 文件。 gcc -S main.i -o main.s
-c（小写 c）：代表编译和汇编，但不链接，生成 .o 文件。将汇编代码转换成机器能看懂的二进制目标文件。到这一步它还没有连接成可执行程序，所以不能运行。
-C（大写 C）：代表告诉预处理器不要丢弃注释（通常配合 -E 使用，比如 gcc -E -C source.c，这样生成的 .i 文件里还会保留自己写的注释）。
-链接器阶段。把刚才生成的 .o 目标文件，和系统自带的标准库（比如 libc）拼在一起。输出：最终可执行程序。 gcc main.o -o main（-o 也是参数，指定输出名字）。
./main
-Wall作用：打开绝大部分警告信息（极其实用！能帮你发现变量未使用、类型不匹配等潜在隐患）。
    例子：gcc -Wall main.c -o main。
-g作用：在编译出的程序里加入调试符号表信息。不加这个，后面用 gdb 调试或者看 core 崩溃文件时，GDB 只会给一堆乱码地址，找不到哪一行代码崩溃的。只要想调  试，必须加 -g
    例子：gcc -g main.c -o main。
-I <目录路径>作用：告诉 GCC 去哪里找 .h 头文件。如果把 header.h 放到了 include 文件夹里，不写 -I 编译器会报错“找不到文件”。
    例子：gcc -I ./include main.c -o main。
-L <目录路径>作用：告诉 GCC 去哪里找 .a 或 .so 静态库/动态库文件。
    例子：gcc -L ./lib main.c -o main -lm。
-l <库名>作用：告诉 GCC 需要链接哪个库。注意：库名要去掉 lib 前缀和 .a/.so 后缀。
    例子：链接数据库 libm.so 写成 -lm；链接线程库 libpthread.so 写成 -lpthread。
-lcalc：找libcalc.a或者libcalc.so

例子：gcc -Wall -g -I ./include main.c utils.c -o my_program -L ./lib -lm
    -Wall -g：开启警告，带上调试信息。
    -I ./include：去 include 目录找头文件。
    main.c utils.c：要编译的多个源文件。
    -o my_program：生成程序名叫 my_program。
    -L ./lib：去 lib 目录找库文件。
    -lm：链接数学动态库。

二、静态库和动态库打包-------------------------------------------------------------
静态库：
    特点：静态库 libxxx.a 、编译参数：ar rcs、需拷贝库代码进执行程序内，占用空间较大，每个进程备份一份。链接运行时不依赖原库文件，直接给exe即可
          空间换时间
动态库：
    特点：动态库 libxxx.so、编译参数：gcc -fPIC -shared 、链接时仅记录依赖，不拷贝代码，运行时必须存在.so，占用体积小，内存只加载一份，多进程共享。exe + 配套 so 一起发布
    时间换空间

三、Makefile工程管理---------------------------------------------------------------
    1、先列变量、
    2、整理依赖项
    3、增加clean逻辑作为保护措施
   
    语法：
    目标：通常是你想生成的文件名，比如 main 或者 main.o。
    依赖：生成目标所需要的文件。如果依赖文件比目标文件“新”，Make 就会执行下面的命令。
    命令：开头必须是一个 Tab 制表符（不能是空格！），后面跟着要执行的 shell 命令（比如 gcc ...）。
        目标文件:源文件
            编译器 编译参数 -o 目标文件 源文件
    例：
        #可以先定义类似宏，这样简洁方便
        TARGET = main
        SRCS = main.c
        CC = gcc
        CFLAGS = -wall -g

        #第一层先写依赖：main 以来main.c
        $(TARGET):$(SRCS)
            $(CC) $(CFLAGS) -o $@ $^

        #第二层建立伪目标，无依赖，单纯清理删除文件命令
        .PHONY: clean
        clean: 
            rm -f $(TARGET)
四、GDB调试core文件打印出crash地址
    1、 -g                    编译（必须加 -g，否则 GDB 看不懂行号）
    2、 ulimit -c unlimited    开启系统记录崩溃现场（允许生成 core 文件）
    3、 ./gdb_crash            运行，看到 "Segmentation fault (core dumped)"或者Program received signal SIGSEGV（段错误）, Segmentation fault
    4、 gdb ./gdb_crash core    使用 GDB 分析 core 文件；有的平台会找不到文件，因为被隐藏了不在当前路径生成，gdb ./gdb_crash  然后输入run，然后输入bt，即可打印出来
    
    例：
    gcc -g gdb_crash.c  -o gdb_crash
    ulimit -c unlimited
    gdb ./gdb_crash core  然后输入bt  或者 gdb ./gdb_crash（报错找不到文件的时候使用，然后输入run，然后输入bt）

    #0  #1 等等会有这样的打印，是从下而上的执行的，#0代表正在执行的函数，#1代表里面调用了#0，运行#1走到里面#0函数接口死掉

    ***补充：
        1、手动调试，增加断点：
            gcc -g Macro.c -o macro     //添加-g
            gdb ./macro                 //进入后按回车
            break 7                    //在代码第 7 行打断点；break 行号 或 break 函数名
            run                         //让程序跑起来，它会停在断点位置
            print 变量                  //打印一下当前变量的值（看看有问题不）
            next/n                      //单步执行一步，执行第 6 行的 *p = 100
            frame                       //程序崩了，但我们想知道它具体卡在哪一行。输入：
                                    （或者用 bt），GDB 会明确告诉你：当前停在了第 几行。
            continue                    //让程序继续跑完
            c                           /(继续)。
            list                        //(或 l - 显示代码，方便看行号，虽然他们可以在编辑器中看到)。
            step                        //(缩写 s)单步（进入）如果当前行调用了其他函数，进入该函数内部一步步看。
            finish                      //进入某个函数后，想立刻执行完该函数并返回上一级。

        2、可视化增加缎断点调试 
            左侧第四个爬虫表示，需要配置
            第一步： 点击 VS Code 左侧边栏那个“运行和调试”的小虫子图标（或者按快捷键 Ctrl+Shift+D）。
            第二步： 点击顶部的“创建 launch.json 文件”（或者“create a launch.json file”）。
            第三步： 在弹出的下拉列表里，选择 C++ (GDB/LLDB) （或者选择 WSL 也行）。
            第四步： VS Code 会生成一个 launch.json 文件，只需要把里面的 "program" 一行稍微改一下，改成自己现在这个 gdb_crash 程序的名字和路径即可：


## 代码练习

1、编译流程：gcc_demo
2、静态库和动态库打包：static-Dynamic-library
3、make工程管理：Makefile
4、GDB调试：GDB

## 遇到问题 & 解决方案
1、gcc 优先链接静态还是动态？
    同一个目录同时存在libcalc.a、libcalc.so，默认优先链接动态库。
    强制只用静态库：gcc main.c -L. -l:libcalc.a -o app
2、-fPIC 干什么？
    位置无关代码，动态库需要加载到内存任意地址，必须生成 PIC 代码。静态库不需要。
3、
    .h头文件：只有声明，不参与 gcc‑c 编译，不会产出.o
    main.c 是主程序，编译链接时使用，禁止打包进静态库 / 动态库
    静态库原材料：是实现功能的.o（由.c编译而来）。

4、makefile不能起名字同名，容易路径迷糊


## 重要知识点总结
1、编译流程每个阶段会生成什么文件
2、静态库流程：
    删除libmymath.a还是可以运行
    制作静态库 libmymath.a
    # 1.先编译成目标文件 -c 只汇编不链接
        gcc -c add.c sub.c 
    # 2.ar工具打包静态库  rcs 参数固定记住:  r:插入 c:创建 s:生成索引
        ar rcs libmymath.a add.o sub.o
    静态库命名强制规范：必须以 libxxx.a，链接的时候写‑lmymath，自动补 lib 和.a
    链接使用静态库，两种写法:
    #写法1：直接把.a放编译参数
        gcc main.c libmymath.a -o app_static
    #写法2：-L 指定库搜索路径  -l指定库名
       gcc main.c -L. -lmymath -o static
        ./static
3、动态库流程：
    删除libmymath.so是不可以运行，程序崩溃
    制作动态库 libmymath.so
    # 1. gcc -fPIC -c add.c sub.c
        -c：只编译不链接，产出 add.o、sub.o
        -fPIC：生成位置无关代码（Position‑Independent Code），动态库 .so 必须要 PIC；程序加载时，so 可以被映射到内存任意地址，多个进程可以共享同一份库代码。静态库不需要 -fPIC。
    # 2. gcc -shared add.o sub.o -o libmymath.so
        -shared：生成共享库（动态库 libxxx.so），不是可执行程序
        -o libmymath.so：输出动态库文件，命名强制约定 libxxx.so
        静态库用 ar rcs；动态库用 gcc -shared。
    # 3. gcc main.c -L. -lmymath -o dynamic
        main.c：主程序源码
        -L.：库搜索路径，.代表当前目录，告诉链接器去哪里找库文件
        -lmymath：链接 libmymath.so   -l后面填名字，自动补前后缀 lib + 名字 + .so（优先找 so，找不到再找.a）
        这里只是编译链接阶段找到 so；程序运行的时候操作系统默认不会去当前目录找 so，所以要下面这条。
        export LD_LIBRARY_PATH=$PWD:$LD_LIBRARY_PATH：
            LD_LIBRARY_PATH：Linux 动态链接器环境变量，指定运行时去哪里搜索.so动态库
            $PWD：当前工作目录路径
            $LD_LIBRARY_PATH：旧的搜索路径，拼在后面，不覆盖原有配置

4、Makefile
    写工程，先列变量（CC、CFLAGS、TARGET、SRCS）。
    按需写出第一条依赖规则。
    给 clean 加上 .PHONY 保护罩。
    缩进用 Tab，这是坑！

5、宏（Macro）---预处理阶段
    分为：
    1、基础宏（定义常量）：原理：预处理后，代码里的 PI 会被编译器替换成 3.14159，相当于没占用任何内存空间。
        #define PI 3.14159
        #define MAX_COUNT 100
        #define CHAR "string"

    2、带参数的宏（模拟函数）
        // 定义一个求最大值的宏
        #define MAX(a, b) ((a) > (b) ? (a) : (b))
        int main() {
            int x = 10, y = 20;
            int max_val = MAX(x, y);
            printf("最大值是: %d\n", max_val); // 输出 20
            return 0;
        }
    无脑的“防御性口诀，只要宏里写了参数，不管三七二十一，直接套两层括号：#define 宏名(参数) ( (参数) 操作 (参数) )

   对比宏和函数：
    宏：                          
        处理阶段： 预处理阶段（gcc -E）
        内存占用：不占额外的栈空间，但代码体积会膨胀（每处使用都替换，变大）
        执行速度：极快（没有函数调用、压栈、返回的开销）
        调试难度：地狱级。因为 GDB 里看不到宏，只能看到展开后的汇编。
        *副作用：极其危险。比如 MAX(i++, j++) 会变成 (i++) > (j++) ...，导致 i 和 j 被自增两次！

    普通函数 (Function)：
        处理阶段： 编译/链接阶段
        内存占用：只占用一份代码（堆/栈存参数，不会膨胀）
        执行速度：相对较慢（有函数调用开销）
        调试难度：可以单步调试（step 进函数里）
        *副作用：安全（函数参数在调用前只求值一次）

    拓展：
        #号的作用（字符串化）：把宏的参数变成一个双引号包裹的字符串
            #define STR(x) #x
            //用法：#define(hello)会被展开为 "hello"
        ##号的作用（粘贴），把两个标识符粘在一起，形成新的标识符
            #define VAR(x) g_val_##x
            //用法：int VAR(100) h展开为int g_val_100

## 截图
无


                    
