#补充：泛规则 %    如果文件多的话，需要.o文件。
#可以用 % 符号写一条万能规则，告诉编译器：“只要遇到 .c 结尾的文件，就自动把它变成同名的 .o 目标文件”
#	%.o: %.c
#	$(CC) $(CFLAGS) -c $< -o $@
#

#可以先定义类似宏，这样简洁方便
TARGET = main
SRCS = main.c
CC = gcc
CFLAGS = -Wall -g

#第一层先写依赖：main 以来main.c
$(TARGET):$(SRCS)
	$(CC) $(CFLAGS) -o $@ $^


#第二层建立伪目标，无依赖，单纯清理删除文件命令
.PHONY: clean
clean: 
	rm -f $(TARGET)