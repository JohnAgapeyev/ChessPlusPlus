COMP=g++
FLAGS=-c -Wall -pedantic
DEPS=%.o
SRC=./src
HEAD=./headers

all: $(DEPS)
	$(COMP) -o Chess $^

%.o: %.c $(HEAD)/*.h
	$(COMP) $(FLAGS) $@ $<

.PHONY: clean

clean:
	rm -f $(SRC)/*.o
