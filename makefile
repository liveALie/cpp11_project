TAGET:=bin/c11test
OBJ:=obj/main.o

F:=-Wall -g -O3 -std=c++11 -lpthread
CC:=g++

$(TAGET):$(OBJ)
	$(CC) -o $@ -Iinclude $^ $(F)

obj/%.o:src/%.cpp #通配符
	$(CC) -o $@ -Iinclude -c $^ $(F)

.PHONEY:clean
clean:
	rm -rf $(TAGET)
	rm -rf obj/*.o
	rm -rf *~ *.*~
	rm -rf include/*.*~
	rm -rf src/*.*~