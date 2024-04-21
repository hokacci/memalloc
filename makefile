run: memalloc
	./memalloc

memalloc:
	gcc -o memalloc main.c yhok_memalloc.c -I. -g

clean:
	rm -f memalloc