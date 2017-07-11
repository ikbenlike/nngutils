CC = gcc

ODIR = ./bin

src = $(wildcard src/*.c)
obj = $(src:.c=)

objs = $(patsubst src/%,bin/%,$(obj))

$(ODIR)/%: src/%.c
	$(CC) -o $@ $<

build: $(objs)
	@echo build complete

.PHONY: clean

clean: $(objs)
	rm $(objs)
