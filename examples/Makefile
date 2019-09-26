CXX = g++
CXXFLAGS += -std=c++11 -I../src
LDFLAGS += -L/usr/local/lib -Wl,--no-as-needed 

all: sope_simple_test sope_record_test

sope_simple_test: sope_simple_test.o
	$(CXX) $^ $(LDFLAGS) -o $@

sope_record_test: sope_record_test.o sope_types.o
	$(CXX) $^ $(LDFLAGS) -o $@

clean:
	rm -f *.o
	rm sope_simple_test sope_record_test
