CXXFLAGS += -g -Wall
CXXFLAGS += -O2

sparsearray: main.cpp sparsearray.h
	g++ $(CXXFLAGS) $< -o $@

runtest: test/main.o test/linkedlistsa.o test/doublelinkedlistsa.o test/bitmapsa.o test/chunksa.o
	g++ $(CXXFLAGS) $(filter %.o,$^) -o $@

test: runtest
	./runtest

test/doublelinkedlistsa.o: sparsearray.h test/common.h
test/linkedlistsa.o: sparsearray.h test/common.h
test/bitmapsa.o: sparsearray.h test/common.h
test/chunksa.o: sparsearray.h test/common.h

.PHONY: test
