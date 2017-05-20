CC=gcc
CFLAGS=-g -Wall -O3 -lcurl `curl-config --cflags` -L/usr/lib/x86_64-linux-gnu
DEPS = webcurl.h
OBJ = webcurl.o

%.o: $.c $(DEPS) $(CC) -c -o $@ $< $(CFLAGS)

webcurl: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	$(RM) webcurl *.o *~