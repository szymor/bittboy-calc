.PHONY: all clean pc
all:
	arm-linux-gcc main.c -o calc -ggdb -lSDL -lSDL_ttf
pc:
	gcc main.c -o calc -lSDL -lSDL_ttf
clean:
	rm -rf calc
