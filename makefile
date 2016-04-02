CXX = g++
CXXFLAGS = -c -Wall -pedantic -std=c++14 -O3 -D DEBUG
ODIR = bin
SRC = src
HEAD = $(SRC)/headers
APPNAME = Chess
OBJS = $(ODIR)/%.o
SRCOBJS = $(SRC)/%.cpp
SRCWILD = $(wildcard $(SRC)/*.cpp)
HEADWILD = $(wildcard $(HEAD)/*.h)
EXEC = $(ODIR)/$(APPNAME)

# Prereqs are all .o files in the bin folder; assuming each source .cpp file is turned into a .o
all: $(patsubst $(SRCOBJS), $(OBJS), $(SRCWILD))
# Command takes all bin .o files and creates an executable called chess in the bin folder
	$(CXX) $^ -o $(EXEC)

# Target is any bin .o file, prereq is the equivalent src .cpp file and all header files
$(OBJS): $(SRCOBJS) $(HEADWILD)
# Command compiles the src .cpp file with the listed flags and turns it into a bin .o file
	$(CXX) $(CXXFLAGS) $< -o $(patsubst $(SRCOBJS), $(OBJS), $<)

# Prevent clean from trying to do anything with a file called clean
.PHONY: clean

# Deletes the executable and all .o files in the bin folder
clean:
	$(RM) $(ODIR)/*
