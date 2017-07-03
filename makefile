BASEFLAGS := -Wall -pedantic -pipe -std=c++14 -fopenmp
DEBUGFLAGS := -fsanitize=undefined -g
RELEASEFLAGS := -O3 -march=native -flto -ftracer -funswitch-loops -funroll-loops -funit-at-a-time -DNDEBUG
CLIBS := -pthread
APPNAME := Chess
ODIR := bin
SRC := src
HEAD := $(SRC)/headers
OBJS := $(ODIR)/%.o
SRCOBJS := $(SRC)/%.cpp
SRCWILD := $(wildcard $(SRC)/*.cpp)
HEADWILD := $(wildcard $(HEAD)/*.h)
EXEC := $(ODIR)/$(APPNAME)
DEPS := $(EXEC).d

all release debug: $(patsubst $(SRCOBJS), $(OBJS), $(SRCWILD))
# Command takes all bin .o files and creates an executable called chess in the bin folder
	$(CXX) $(CXXFLAGS) $^ $(CLIBS) -o $(EXEC)

$(ODIR):
	@mkdir -p $(ODIR)

# Create dependency file for make and manually adjust it silently to work with other directories
$(DEPS): $(SRCWILD) $(HEADWILD) | $(ODIR) 
# Compile the non-system dependencies and store it in outputdir/execname.d
	@$(CXX) -MM $(CXXFLAGS) $(SRCWILD) > $(DEPS)
# Copy the contents to a temp file
	@cp $(DEPS) $(DEPS).tmp
# Take the temp file contents, do a regex text replace to change all .o strings into
# outputdir/.o strings, and store the result in outputdir/execname.d
	@sed -e 's/\w\+\.o/$(ODIR)\/&/g' < $(DEPS).tmp > $(DEPS)
# Delete the temp file
	@$(RM) $(DEPS).tmp

# Add the dependencies into make and don't throw an error if it doesn't exist
# Also don't generate dependency file during a clean
ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif

#Check if in debug mode and set the appropriate compile flags
ifeq (,$(filter debug, $(MAKECMDGOALS)))
$(eval CXXFLAGS := $(BASEFLAGS) $(RELEASEFLAGS))
else
$(eval CXXFLAGS := $(BASEFLAGS) $(DEBUGFLAGS))
endif

# Target is any bin .o file, prereq is the equivalent src .cpp file
$(OBJS): $(SRCOBJS)
# Command compiles the src .cpp file with the listed flags and turns it into a bin .o file
	$(CXX) -c $(CXXFLAGS) $< -o $(patsubst $(SRCOBJS), $(OBJS), $<)

# Prevent clean from trying to do anything with a file called clean
.PHONY: clean

# Deletes the executable and all .o and .d files in the bin folder
clean: | $(ODIR)
	$(RM) $(EXEC) $(wildcard $(EXEC).*) $(wildcard $(ODIR)/*.d*) $(wildcard $(ODIR)/*.o)

