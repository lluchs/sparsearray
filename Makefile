CXXFLAGS += -g -O2

sparsearray: main.cpp sparsearray.h
	g++ $(CXXFLAGS) $< -o $@
