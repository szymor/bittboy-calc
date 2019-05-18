.PHONY: all clean pc
all:
	arm-linux-gcc main.c -o sdltest -ggdb -lSDL
pc:
	gcc main.c -o sdltestpc -lSDL
clean:
	rm -rf sdltest sdltestpc
