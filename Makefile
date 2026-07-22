CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = regexguard
SRCS = main.cpp

all: $(TARGET)

$(TARGET): $(SRCS) regex_parser.h nfa.h dfa.h
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET) main_app *.o

.PHONY: all clean
