sourceDir=./src/
privateDir=$(sourceDir)private/
publicDir=$(sourceDir)public/
objectDir=$(sourceDir)obj/

INC=$(privateDir) $(publicDir)
INC_PARAMS=$(INC:%=-I%)

SOURCES = $(wildcard $(privateDir)*.cpp)
OBJECTS = $(patsubst $(privateDir)%.cpp,$(objectDir)%.o,$(SOURCES))
DEPENDS = $(SOURCES:.cpp=.d)

LUAFLAGS = $(shell pkg-config --cflags lua5.1) -llua5.1
CPPFLAGS = $(LUAFLAGS) -lpng -pthread -std=c++14 -O2
CXXFLAGS = $(CPPFLAGS) -W -Wall -g

CXX = g++
MAIN = rt

all: $(MAIN) MKDIR

clean:
	@echo Cleaning...
	@rm -f $(sourceDir)*.o $(sourceDir)*.d $(MAIN)

$(MAIN): $(OBJECTS)
	@echo Creating $@...
	@$(CXX) -o $@ $(OBJECTS) $(INC_PARAMS) $(CPPFLAGS)

#Generate objects
$(objectDir)%.o: $(privateDir)%.cpp
	@echo Compiling $<...
	@mkdir -p $(@D)
	@$(CXX) -o $@ -c $(CXXFLAGS) $(INC_PARAMS) $<

MKDIR:
	@echo Creating image directory...
	@mkdir -p ./img

-include $(DEPENDS)
