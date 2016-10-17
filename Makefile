LIBS    = -lm -lz
CFLAGS += -std=gnu11 -Wall -O3 -iquote src

SOURCES = $(wildcard src/*.c)
HEADERS = $(wildcard src/*.h)
LOBJS   = $(patsubst %.c,%.lo,$(SOURCES))
OBJS    = $(patsubst %.c,%.o,$(SOURCES))

libdcstream.a: $(OBJS)
	$(AR) rcs $@ $^

libdcstream.la: $(LOBJS)
	$(AR) rcs $@ $^

libdcstream.so: $(LOBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

.PHONY: clean
clean:
	rm -f src/*.o libdcstream.*


CFLAGS += -fPIC
src/%.lo: src/%.c src/%.h
	$(CC) $(CFLAGS) -c -o $@ $<


.PHONY: doc
doc:
	cldoc generate $(CFLAGS) -- --language c --report --output html $(SOURCES) $(HEADERS)
