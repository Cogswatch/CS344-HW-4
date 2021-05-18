main: main.c
	gcc --std=gnu99 -pthread -o line_processor main.c -I.