target: exe
TARGET_CODE = heap-memory-allocator.c
FLAGS = -w

exe: 
	$(CC) $(TARGET_CODE) $(FLAGS) -o exe

debug:
	$(CC) $(TARGET_CODE) $(FLAGS) -g -o exe

clean:
	rm exe
