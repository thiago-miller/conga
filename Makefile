PROGNAME := conga
SRC      := conga.c

all: $(PROGNAME)

$(PROGNAME): $(SRC)
	gcc -o $@ $^
