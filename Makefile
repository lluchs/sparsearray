CXXFLAGS += -g -Wall -std=c++14
CXXFLAGS += -O2

sparsearray: main.cpp sparsearray.h
	$(CXX) $(CXXFLAGS) $< -o $@

runtest: test/main.o test/linkedlistbitmapsa.o test/linkedlistsa.o test/doublelinkedlistsa.o test/bitmapsa.o test/chunksa.o
	$(CXX) $(CXXFLAGS) $(filter %.o,$^) -o $@

test: runtest
	./runtest

# Runs the benchmark with gcc and clang.
benchmark:
	$(MAKE) -B sparsearray CXX=g++
	./sparsearray > benchmark/gcc.log
	./sparsearray -a 50 -i 1000000 > benchmark/lowload-gcc.log
	$(MAKE) -B sparsearray CXX=clang++
	./sparsearray > benchmark/clang.log
	./sparsearray -a 50 -i 1000000 > benchmark/lowload-clang.log
	(cd benchmark && ./performance.gpi)
	(cd benchmark && ./memoverhead.gpi)

test/doublelinkedlistsa.o: sparsearray.h test/common.h
test/linkedlistsa.o: sparsearray.h test/common.h
test/linkedlistbitmapsa.o: sparsearray.h test/common.h
test/bitmapsa.o: sparsearray.h test/common.h
test/chunksa.o: sparsearray.h test/common.h

.PHONY: test benchmark
