prefix := /usr/local
PREFIX := $(prefix)

ifdef DEBUG
CFLAGS ?= -g -fsanitize=address -Wpedantic
else
CFLAGS ?= -O3
endif

CFLAGS += -std=gnu11 -Wall -iquote src
LIBS    = -lm $(shell pkg-config --libs libzstd zlib) -lbz2

SOURCES = dcs_compr.c dcs_stream.c
HEADERS = $(patsubst %.c,%.h,$(SOURCES))
LOBJS   = $(patsubst %.c,%.lo,$(SOURCES))
OBJS    = $(patsubst %.c,%.o,$(SOURCES))

SOVERSION=0

all: libdcstream.a

libdcstream.a: $(OBJS)
	$(AR) rcs $@ $^

libdcstream.la: $(LOBJS)
	$(AR) rcs $@ $^

libdcstream.so.$(SOVERSION): $(LOBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o $@ $^ $(LIBS)

libdcstream.so: libdcstream.so.$(SOVERSION)
	ln -sf $< $@

run_tests: src/test/main.c $(wildcard src/test/test_*.c) | libdcstream.so
	$(CC) $(CFLAGS)  -o $@ $< -L. -ldcstream -lcmocka  $(LIBS)

.PHONY: test
test: run_tests
	@./run_tests

.PHONY: clean
clean:
	rm -f *.o *.lo libdcstream.* run_tests dcstream.pc
	rm -rf html

%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) -c -o $@ $<


%.lo: src/%.c src/%.h
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<


.PHONY: doc
doc: $(addprefix src/,$(SOURCES) $(HEADERS))
	cldoc generate $(CFLAGS) -- --language c --report --output html $^


INCLUDEDIR   = $(PREFIX)/include/dcstream
LIBDIR       = $(PREFIX)/lib
PKGCONFIGDIR = $(LIBDIR)/pkgconfig

INSTALLED_LIBS=libdcstream.a libdcstream.so libdcstream.so.$(SOVERSION)

INSTALL = install -p

.PHONY: install
install: $(INSTALLED_LIBS)
	sed -e 's,@PREFIX@,$(PREFIX),' < dcstream.pc.in >dcstream.pc
	mkdir -p -m 755 $(INCLUDEDIR) $(LIBDIR) $(PKGCONFIGDIR)
	$(INSTALL) -m 644 src/dcs_stream.h $(INCLUDEDIR)/dcstream.h
	$(INSTALL) -m 755 $(INSTALLED_LIBS) $(LIBDIR)
	$(INSTALL) -m 644 dcstream.pc $(PKGCONFIGDIR)
