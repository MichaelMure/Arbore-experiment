CMAKE_OPTIONS = -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=0
PF_NET_OPT = $(CMAKE_OPTIONS) -DPF_NET:BOOL=ON
PF_LAN_OPT = $(CMAKE_OPTIONS) -DPF_NET:BOOL=OFF

all: pfnet pflan

pfnet: build.peerfuse-net/Makefile
	make -C build.peerfuse-net all -j4

pflan: build.peerfuse-lan/Makefile
	make -C build.peerfuse-lan all -j4

build.peerfuse-net/Makefile:
	[ -d build.peerfuse-net ] || mkdir build.peerfuse-net; \
	cd build.peerfuse-net && cmake $(PF_NET_OPT) ..

build.peerfuse-lan/Makefile:
	[ -d build.peerfuse-lan ] || mkdir build.peerfuse-lan; \
	cd build.peerfuse-lan && cmake $(PF_LAN_OPT) ..

clean:
	rm -rf build.peerfuse-net
	rm -rf build.peerfuse-lan
	rm -rf release

.PHONY: all clean pfnet pflan
