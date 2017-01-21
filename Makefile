CXXFLAGS += -g
#CXXFLAGS += -O2

sparsearray: main.cpp sparsearray.h
	g++ $(CXXFLAGS) $< -o $@

runtest: test/main.o test/linkedlistsa.o test/bitmapsa.o
	g++ $(CXXFLAGS) $(filter %.o,$^) -o $@

test/linkedlistsa.o: sparsearray.h
test/bitmapsa.o: sparsearray.h
