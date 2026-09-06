objects/main.o: source/main.c
	gcc -c source/main.c -o objects/main

scheduler: objects/main
	gcc objects/main.o -o scheduler

clean:
	rm -f objects/*.o scheduler

.PHONY: clean