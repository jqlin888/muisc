
music:main.o file.o music.o
	gcc -o $@ $^

%.o : %.c
	gcc -Wall -c -o $@ $< -I.

clean:
	rm -rf *.o music

