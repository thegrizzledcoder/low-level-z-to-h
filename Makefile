TARGET = bin/final
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

run: clean default
	./$(TARGET) -f ./mynewdb.db -n
	./$(TARGET) -f ./mynewdb.db -a "Timmy H.,123 Sheshire Ln.,120"

default: $(TARGET)

clean:
	rm -rf obj/*.o
	rm -rf bin/*
	rm ./mynewdb.db

$(TARGET): $(OBJ)
	gcc -o $@ $?

obj/%.o : src/%.c
	gcc -c $< -o $@ -Iinclude