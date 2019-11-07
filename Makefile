CXX=g++
CXX_OPTIONS=-g -pthread -std=c++17
SERVER_FILE=util.o http.o mimetypes.o server.o route.o main.o
SERVER_PATH=$(addprefix src/,$(SERVER_FILE))
TARGET=main.out

%.o:%.cpp
	$(CXX) -c $(CXX_OPTIONS) $< -o $@

$(TARGET): $(SERVER_PATH)
	$(CXX) $(CXX_OPTIONS) $^ -o $@

run: $(TARGET)
	-kill -9 `lsof -t -i:8082`
	./$<

clean:
	-rm *.out $(SERVER_PATH)