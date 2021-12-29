obj-m += dogdoor.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

bingo:
	gcc -o bingo bingo.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm bingo
