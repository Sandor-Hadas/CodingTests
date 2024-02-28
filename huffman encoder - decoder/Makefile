CC            = gcc
CXX           = g++
DEFINES       = -LINUX
CFLAGS        = -pipe -O2 -Wall -g -D_REENTRANT $(DEFINES)
CXXFLAGS      = -pipe -O2 -Wall -g -D_REENTRANT $(DEFINES)
INCPATH       = .
LINK          = g++
SOURCES       = main.cpp huffman.cpp
OBJECTS       = main.o huffman.o
HEADERS       = huffman.h
DESTDIR       = .
TARGET        = Huffman

.SUFFIXES: .o .cpp 

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

all: Makefile $(TARGET)

$(TARGET):  $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)

main.o: main.cpp huffman.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o main.o main.cpp

huffman.o: huffman.cpp huffman.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o huffman.o huffman.cpp


