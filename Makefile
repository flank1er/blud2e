CC=g++
CFLAGS=--std=gnu++11 -Wall -g -I .
OBJ=blud2e.o conver.o io.o obj.o
TARGET=blud2e
.PHONY: all clean

%.o:	%.cpp
	$(CC) -c -o $@ $< $(CFLAGS)
	
all:	$(OBJ)
	$(CC)  -o $(TARGET)  $^ $(CFLAGS)

clean:
	@rm -v $(OBJ) $(TARGET)
