CXXFLAGS = -g -pthread -Wall -Wpedantic -Wextra -Wlogical-op
CXX = g++

files = $(wildcard ./src/*.cpp)
deps = $(wildcard ./src/*.hpp)

build: $(filter-out ./src/main_ng.cpp, $(files)) $(deps)
	@echo Target $@
	@echo New Prerequsites: $? 
	@echo Compiling...
	@$(CXX) $(CXXFLAGS) $(filter-out ./src/main_ng.cpp, $(files)) -o ./build/ex1
	@echo Done!

build2: $(filter-out ./src/main.cpp, $(files)) $(deps)
	@echo Target $@
	@echo New Prerequsites: $? 
	@echo Compiling...
	@$(CXX) $(CXXFLAGS) $(filter-out ./src/main.cpp, $(files)) -o ./build/ex2
	@echo Done!

run1:
	@echo Running build/ex1
	./build/ex

run2:
	@echo Running build/ex1
	./build/ex2

clean:
	@echo Removing build/*
	@rm ./build/*
