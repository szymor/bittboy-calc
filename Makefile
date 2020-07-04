.PHONY: retrofw miyoo clean pc
retrofw:
	mipsel-linux-gcc main.c -o calc -ggdb -lSDL -lSDL_ttf -lm
	cp calc opkg/
	mksquashfs opkg calc.opk -noappend -no-xattrs
miyoo:
	arm-linux-gcc main.c -o calc -ggdb -lSDL -lSDL_ttf -lm
pc:
	gcc main.c -o calc -lSDL -lSDL_ttf -lm
clean:
	rm -rf calc
