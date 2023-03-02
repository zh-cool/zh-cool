all:serv cli test
CFLAGS += -g
serv:tcserver.o
	gcc $< -o $@
cli:cli.o
	gcc $< -o $@
#.c.o:
#	gcc -c -g  $< -o $@

clean:
	rm -rf *.o serv 