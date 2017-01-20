CXXFLAGS += -g
CXXFLAGS += -O2

sparsearray: main.cpp sparsearray.h
	g++ $(CXXFLAGS) $< -o $@

runtest: test/main.o test/linkedlistsa.o
	g++ $(CXXFLAGS) $^ -o $@
