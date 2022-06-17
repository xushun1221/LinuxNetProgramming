src = $(wildcard *.c)
target = $(patsubst %.c, %, $(src)) # 目标是.c文件编译为可执行文件
myArgs = -Wall -pthread
ALL:$(target)

%:%.c
	gcc $< -o $@ $(myArgs)

clean:
	-rm -rf $(target)

.PHONY: clean ALL