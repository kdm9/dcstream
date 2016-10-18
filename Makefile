LIBS    = -lm $(shell pkg-config --libs libzstd zlib) -lbz2
CFLAGS ?= -O3 -g -fsanitize=address
CFLAGS += -std=gnu11 -Wall -iquote src

SOURCES = dcs_compr.c dcs_stream.c
HEADERS = $(patsubst %.c,%.h,$(SOURCES))
LOBJS   = $(patsubst %.c,%.lo,$(SOURCES))
OBJS    = $(patsubst %.c,%.o,$(SOURCES))

all: libdcstream.a libdcstream.la libdcstream.so

libdcstream.a: $(OBJS)
	$(AR) rcs $@ $^

libdcstream.la: $(LOBJS)
	$(AR) rcs $@ $^

libdcstream.so: $(LOBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o $@ $^ $(LIBS)


run_tests: $(wildcard src/test/*.c) | libdcstream.so
	$(CC) $(CFLAGS)  -o $@ $^ -L. -ldcstream -lcmocka  $(LIBS)

.PHONY: test
test: run_tests
	@./run_tests

.PHONY: clean
clean:
	rm -f *.o *.lo libdcstream.* run_tests

%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) -c -o $@ $<


%.lo: src/%.c src/%.h
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<


.PHONY: doc
doc:
	cldoc generate $(CFLAGS) -- --language c --report --output html $(SOURCES) $(HEADERS)
