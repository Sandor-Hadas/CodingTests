CXX = g++
CXXFLAGS = -Wall -std=c++17
TARGET = test
OBJS = DoublyLinkedList.o test.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm *.o test
