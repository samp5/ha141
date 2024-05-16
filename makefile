CXXFLAGS = -g -pthread -Wall -Wpedantic -Wextra -Wlogical-op
CXX = g++

files = $(wildcard ./src/*.cpp)
deps = $(wildcard ./src/*.hpp)

build:  $(files) $(deps)
	@echo Target $@
	@echo New Prerequsites: $? 
	@echo Compiling...
	@$(CXX) $(CXXFLAGS) $(files) -o ./build/snn
	@echo Done!

run:
	@echo Running build/ex2
	./build/snn

clean:
	@echo Removing build/*
	@rm ./build/*
