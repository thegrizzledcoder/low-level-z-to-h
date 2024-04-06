TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

run: clean default
	./$(TARGET) -f ./employees.db -n
	./$(TARGET) -f ./employees.db -a "Timmy H.,123 Sheshire Ln.,120"
	./$(TARGET) -f ./employees.db -a "John H.,818 W McKinley St.,120"
	./$(TARGET) -f ./employees.db -a "John A.,645 W McKinley St.,120"

default: $(TARGET)

clean:
	rm -rf obj/*.o
	rm -rf bin/*
	rm ./employees.db

$(TARGET): $(OBJ)
	gcc -o $@ $?

obj/%.o : src/%.c
	gcc -c $< -o $@ -Iinclude