objects/main.o: source/main.c
	gcc -c source/main.c -o objects/main.o

scheduler: objects/main.o
	gcc objects/main.o -o scheduler

clean:
	rm -f objects/*.o scheduler

.PHONY: clean