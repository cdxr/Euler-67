CXX=g++
CPPFLAGS= -std=c++11 -Wall 

all: euler67

euler67: euler67.o
	$(CXX) -o euler67 euler67.o

euler67.o: euler67.cpp

clean:
	rm -f euler67.o

dist-clean:
	rm -f euler67
