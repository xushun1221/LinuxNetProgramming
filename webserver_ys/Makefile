src = $(wildcard ./*.cpp)  
obj = $(patsubst ./%.cpp, ./%.o, $(src)) 

myArgs = -Wall -pthread
target = server
CC = g++

ALL:$(target)

$(target):$(obj)
	$(CC) $^ -o $@ $(myArgs) 

$(obj):%.o:%.cpp
	$(CC) -c $^ -o $@ $(myArgs)

clean:
	-rm -rf $(obj) $(target)

.PHONY: clean ALL