CXXFLAGS := -g -pthread -Wall -Wpedantic -Wextra -Wlogical-op
CXX := g++
CXX2 := c++
PYFLAGS := -g -pthread -O3 -Wall -shared -fPIC $(shell python3-config --includes) -Ipybind11/include
SERVER_FLAGS := -g -pthread -Wall -Wpedantic -Wextra -Wlogical-op
files = $(wildcard ./src/*.cpp)
deps = $(wildcard ./src/*.hpp)

build:  $(filter-out ./src/test.cpp, $(files)) $(deps)
	@echo Target $@
	@echo New Prerequsites: $? 
	@echo Compiling...
	@$(CXX) $(CXXFLAGS) $(filter-out ./src/test.cpp, $(files)) -o ./build/snn
	@echo Done!

buildp:  $(files) $(deps)
	@echo Target $@
	@echo New Prerequsites: $? 
	@echo Compiling with profiling information...
	@$(CXX) $(CXXFLAGS) $(files) -pg -o ./build/profile_snn
	@echo Done!

buildTest:  $(filter-out ./src/main.cpp, $(files)) $(deps)
	@echo Target $@
	@echo New Prerequsites: $? 
	@echo Compiling...
	@$(CXX) $(CXXFLAGS) $(filter-out ./src/main.cpp, $(files)) -o ./build/test.exe
	@echo Done!

pybind: $(filter-out ./src/test.cpp, $(files)) $(deps) ./src/pybind/snn.hpp ./src/pybind/snn.cpp ./src/pybind/pySNN_py_module.cpp
	@echo Target $@
	@echo New Prerequsites: $? 
	@echo Compiling...
	@$(CXX2) $(PYFLAGS) $(filter-out ./src/test.cpp, $(files)) ./src/pybind/snn.cpp -o ./extern/snn$(shell python3-config --extension-suffix)	
	@echo Done!

testpy: ./src/pybind/snn.cpp
	@echo Target $@
	@echo New Prerequsites: $? 
	@echo Compiling...
	@$(CXX2) $(PYFLAGS) ./src/pybind/snn.cpp -o ./extern/snn$(shell python3-config --extension-suffix)	
	@echo Done!

SERVER_FILTER_OUTS=./src/test.cpp ./src/main.cpp
build_server: ./src/server/*
	mkdir -p build/server
	$(CXX) $(SERVER_FLAGS) ./src/server/*.cpp  $(filter-out $(SERVER_FILTER_OUTS), $(files)) -o ./build/server/server.out

server: build_server


CPP_CLIENT_FILTER_OUTS=./src/test.cpp ./src/main.cpp
CPP_CLIENT_FILES = $(filter-out $(CPP_CLIENT_FILTER_OUTS), $(files)) ./src/rpc/rpc_main.cpp ./src/pybind/snn_connnect.cpp
build_cpp_client:  $(CPP_CLIENT_FILES)
	mkdir -p build/client
	$(CXX) $(CXXFLAGS) $(CPP_CLIENT_FILES) -o ./build/client/client.out

cpp_client: build_cpp_client

run:
	@echo Running build/ex2
	./build/snn
test:
	@echo Running build/ex2
	./build/test.exe

clean:
	@echo Removing build/*
	@rm ./build/*
