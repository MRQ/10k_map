CXXFLAGS   := -DNO_ECLIPSE -std=c++11 -O3 -g $(shell pkg-config eigen3 libxml++-2.6 --cflags) -c -Winvalid-pch -fpch-preprocess 
CXXFLAGS_H := -DNO_ECLIPSE -std=c++11 -O3 -g $(shell pkg-config eigen3 libxml++-2.6 --cflags)
LDFLAGS    := -O3 -g  $(shell pkg-config eigen3 libxml++-2.6 --libs)

HEADERS    := $(shell ls src/*.h)
OBJECTS    := $(shell ls src/*.cpp | sed -e 's,src/,build/,' -e 's,.cpp$$,.o,')

PROJECT    := 10k_map_generator

.PHONY: all clean

all: build/${PROJECT} build/cochem.svg | build

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

build/cochem.paths: build/${PROJECT}
	./build/${PROJECT} 50.147 7.168 < cochem.osm > build/cochem.paths

build/cochem.svg: build/cochem.paths make_map.sh template.svg
	./make_map.sh build/cochem.paths > build/cochem.svg

build/aachen.paths: build/${PROJECT} aachen-overpass04_edited.osm
	#bzip2 -d < nordrhein-westfalen.osm.bz2 | ./build/${PROJECT} 50.7795 6.0998 > build/aachen.paths
	./build/${PROJECT} 50.7856 6.1071 < aachen-overpass04_edited.osm > build/aachen.paths

build/aachen.svg: build/aachen.paths make_map.sh template.svg
	./make_map.sh build/aachen.paths > build/aachen.svg

build/%.svgz: build/%.svg
	gzip -9 < $< > $@
	stat --format="$@: %s bytes" $@
	gvfs-copy build/aachen.svgz sftp://mrq@dasevil.de/var/customers/webs/mrq/public_html/up/10k-map/map.svgz
	gvfs-copy index.html sftp://mrq@dasevil.de/var/customers/webs/mrq/public_html/up/10k-map/index.html
	