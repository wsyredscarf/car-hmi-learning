C语言
    |———五大区：
        |——栈区（stack）：静态开辟，由系统进行自动开辟和释放空间，编译器进行空间分配（程序运行函数时），包括：局部变量，const修饰的局部变量、形参
            |——特点：空间小，速度较快，地址很大，向下增长
        |——堆区（heap）：动态开辟，由开发者手动通过malloc、calloc等函数进行开辟空间，通过free释放，不释放容易造成内存泄漏和碎片化问题
            |——特点：需要寻找内存块，开辟空间大小自由自定义，响应略慢，需要手动释放内存、地址略小、向上增长
        |——全局区（静态区）：分为.data和.bss ，.data区域存放初始化为非零的全局变量，.bss存放初始化为零和未初始化的全局变量。static修饰的局部变量由栈区转移到此区 域。static修饰的静态变量，程序运行期间全程有效。

        |——只读常量区（.rodata）：字符串字面量、const修饰的全局变量。 （函数内部的const修饰的局部变量在栈区）
        |——代码区（.bin）：存放编译后的机器指令，只读
    |————数据类型
        |——基础数据类型
            |——char、short、int、long、float、double
        |——构造类数据类型
            |——数组int arr[]、结构体struct、指针*p、枚举enum、共用体union
        |——九大语句
            |——条件语句：if()else()、switch（）
            |——循环语句：while（）、for（）、do{}while（）
        |——控制语句
            |——break，用于跳出循环，或者switch语句
            |——continue，用于结束本层循环，继续下一层循环
            |——return，结束函数，用于抛出返回值
            |——go to ，跳转指定位置
    |——关键字：int、char、short、long、float、double、if、else、continue、break、return、default、switch、for、while、do、static、const、void、case、goto、sigend、unsigned、extern、struct、enum、union、typedef、sizeof、auto、volatile、register
    |——lib
        |——strlen：计算字符串长度，末尾不包含\0,返回值是长度
        |——strcpy：字符串拷贝，strcpy（dest，src），把src拷贝到dest，自动包含\0。如果strncpy(dest，src，n)，手动末尾加\0
        |——strcmp: 字符串比较，strcmp（dest，src），两个比较，返回值0，则相等。返回值大于0，则dest大于src，反之则小于
        |——strcat：拼接，strcat（dest，src），把src拼接到dest，末尾自动补齐\0。如果是strncat（dest，src，n）则手动补齐末尾\0
------------------------------------------------------------------------------------------------------------------------------------------
重点：
1、五大区的特点、作用域、生命周期
2、数组、指针、数组指针、指针数组、结构体、结构体数组、函数指针、回调函数
3、c库字符串操作手写：strlen、strcpy、strcmp、strcat、strncpy、strncat
4、文件操作：fputc、fgetc、fputs、fgets、fprintf、fscanf | fopen、fclose  |  rewink、sfeek  |  w、w+、r、r+、a、a+
5、结构体字节对齐求大小，位域计算
