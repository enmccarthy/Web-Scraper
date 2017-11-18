CC=gcc
CFLAGS=-g -Wall -O3 -ltidy -lcurl 
DEPS = webcurl.c
OBJ = webcurl.o

%.o: $.c $(DEPS) $(CC) -c -o $@ $< $(CFLAGS)

webcurl: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	$(RM) webcurl *.o *~
