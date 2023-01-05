all: server load_tester
	
server: http_server.hh server.cpp
	g++ -o server server.cpp

load_tester: generator.cpp
	g++ -o generator generator.cpp

clean:
	rm server