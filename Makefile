CXXFLAGS   := -DNO_ECLIPSE -std=c++11 -O3 -g $(shell pkg-config libxml++-2.6 --cflags) -c -Winvalid-pch -fpch-preprocess 
CXXFLAGS_H := -DNO_ECLIPSE -std=c++11 -O3 -g $(shell pkg-config libxml++-2.6 --cflags)
LDFLAGS    := -O3 -g  $(shell pkg-config libxml++-2.6 --libs)

HEADERS    := $(shell ls src/*.h)
OBJECTS    := $(shell ls src/*.cpp | sed -e 's,src/,build/,' -e 's,.cpp$$,.o,')

PROJECT    := 10k_map_generator

.PHONY: all clean

all: build/${PROJECT} | build

clean:
	rm -f src/libs.h.gch
	rm -f -R build

build:
	mkdir -p build

build/%.o: src/%.cpp src/libs.h.gch ${HEADERS} | build
	g++ ${CXXFLAGS} $< -o $@

build/${PROJECT}: ${OBJECTS}
	g++ ${OBJECTS} ${LDFLAGS} -o build/${PROJECT}

src/libs.h.gch: src/libs.h
	g++ ${CXXFLAGS_H} src/libs.h -o src/libs.h.gch


