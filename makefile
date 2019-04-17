a.out: main.c
	gcc -pthread -O2 main.c

clean:
	rm ./a.out