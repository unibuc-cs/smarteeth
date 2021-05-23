.PHONY: all build clean run

CXXFLAGS += -std=c++17
LDFLAGS += -lpistache -lcrypto -lssl -lpthread

all: build run

build: smarteeth

clean:
	-rm smarteeth

run:
	./smarteeth

smarteeth: smarteeth.cpp
	$(CXX) $< -o $@ $(CXXFLAGS) $(LDFLAGS)
