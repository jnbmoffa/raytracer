srcdir = ./src/
SOURCES = $(wildcard $(srcdir)*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
DEPENDS = $(SOURCES:.cpp=.d)
LDFLAGS = $(shell pkg-config --libs lua5.1) -llua5.1 -lpng -pthread -std=c++14 -O2
CPPFLAGS = $(shell pkg-config --cflags lua5.1) -pthread -std=c++14 -O2
CXXFLAGS = $(CPPFLAGS) -W -Wall -g
CXX = g++
MAIN = rt

all: $(MAIN)

clean:
	rm -f $(srcdir)*.o $(srcdir)*.d $(MAIN)

$(MAIN): $(OBJECTS)
	@echo Creating $@...
	@$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	@echo Compiling $<...
	@$(CXX) -o $@ -c $(CXXFLAGS) $<

-include $(DEPENDS)
