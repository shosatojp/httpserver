CXX_OPTIONS=-g -I ~/local/include -L ~/local/lib -std=c++1z

%.o:%.cpp
	g++ $(CXX_OPTIONS) $< -o $@

server.out: httpmessage.cpp server.cpp
	g++ $(CXX_OPTIONS) $^ -o $@

# client.out: client.cpp
# 	g++ $(CXX_OPTIONS) $^ -o $@

run: server.out
	./server.out

clean:
	-rm *.out *.o