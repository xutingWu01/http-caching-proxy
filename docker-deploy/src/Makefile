CCFLAGS=--std=gnu++98 -pedantic -Wall -Werror -ggdb3 -std=c++11

test:  proxy.o server.o main.o request.o client.o cache.o response.o
	g++ -o test $(CCFLAGS)  proxy.o server.o main.o request.o client.o cache.o response.o -lpthread

%.o: %.cpp proxy.hpp server.hpp request.hpp client.hpp cache.hpp response.hpp
	g++ -c $(CCFLAGS) $<

clean:
	rm -f *.o  *~ test
