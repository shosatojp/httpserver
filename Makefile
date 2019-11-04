CXX_OPTIONS=-g -pthread -I ~/local/include -L ~/local/lib -std=c++1z
%.o:%.cpp
	g++ $(CXX_OPTIONS) $< -o $@

main.out: src/httpmessage.cpp src/mimetypes.cpp src/server.cpp src/route.cpp src/main.cpp
	g++ $(CXX_OPTIONS) $^ -o $@

# client.out: client.cpp
# 	g++ $(CXX_OPTIONS) $^ -o $@

run: main.out
	./main.out

clean:
	-rm *.out *.o