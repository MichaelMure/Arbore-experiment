CMAKE_OPTIONS = -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=0

## If PF_SERV_MODE environnement variable is not set, then disable server mode
PF_SERVER_MODE ?= OFF
CMAKE_OPTIONS := $(CMAKE_OPTIONS) -DPF_SERVER_MODE=$(PF_SERVER_MODE)

all: build.peerfuse/Makefile
	make -C build.peerfuse all

build.peerfuse/Makefile:
	[ -d build.peerfuse ] || mkdir build.peerfuse; \
	cd build.peerfuse && cmake $(CMAKE_OPTIONS) ..

install:
	make -C build.peerfuse install

clean:
	rm -rf build.peerfuse
	rm -rf release

doc:
	cd doc/ && /usr/bin/doxygen

.PHONY: all clean install doc
