CXXFLAGS += -g
#CXXFLAGS += -O2

sparsearray: main.cpp sparsearray.h
	g++ $(CXXFLAGS) $< -o $@

runtest: test/main.o test/linkedlistsa.o test/bitmapsa.o test/chunksa.o
	g++ $(CXXFLAGS) $(filter %.o,$^) -o $@

test/linkedlistsa.o: sparsearray.h test/common.h
test/bitmapsa.o: sparsearray.h test/common.h
test/chunksa.o: sparsearray.h test/common.h
