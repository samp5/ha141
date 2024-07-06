CXXFLAGS := -g -pthread -Wall -Wpedantic -Wextra -Wlogical-op
CXX := g++
CXX2 := c++
PYFLAGS := -g -pthread -O3 -Wall -shared -fPIC $(shell python3-config --includes) -Ipybind11/include

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

pybind: $(files) $(deps) ./src/pybind/snn.hpp ./src/pybind/snn.cpp
	@echo Target $@
	@echo New Prerequsites: $? 
	@echo Compiling...
	@$(CXX2) $(PYFLAGS) $(files) ./src/pybind/snn.cpp -o ./extern/snn$(shell python3-config --extension-suffix)	
	@echo Done!

testpy: ./src/pybind/snn.cpp
	@echo Target $@
	@echo New Prerequsites: $? 
	@echo Compiling...
	@$(CXX2) $(PYFLAGS) ./src/pybind/snn.cpp -o ./extern/snn$(shell python3-config --extension-suffix)	
	@echo Done!

run:
	@echo Running build/ex2
	./build/snn
test:
	@echo Running build/ex2
	./build/test.exe

clean:
	@echo Removing build/*
	@rm ./build/*
