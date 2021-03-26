.PHONY: all build clean run

LDFLAGS += -lpistache -lcrypto -lssl -lpthread

all: build run

build: smarteeth

clean:
	-rm smarteeth

run:
	./smarteeth

smarteeth: smarteeth.cpp
	$(CXX) $< -o $@ $(CXXFLAGS) $(LDFLAGS)
