FILES=main.o \
	../lib/util/dtime.o \
	../lib/util/tools.o \
	../lib/util/mutex.o \
	../lib/util/pf_thread.o \
	../lib/util/pf_log.o \
	../lib/util/key.o \
	../lib/scheduler/scheduler.o \
	../lib/scheduler/scheduler_queue.o \
	../lib/scheduler/job.o \
	../lib/files/file_chunk.o \
	../lib/net/network.o \
	../lib/net/packet.o \
	../lib/net/packet_type.o \
	../lib/net/packet_type_list.o \
	../lib/net/host.o \
	../lib/net/hosts_list.o \
	../lib/net/pf_addr.o \
	../lib/dht/chimera.o \
	../lib/dht/chimera_messages.o \
	../lib/dht/chimera_routing.o \
	../lib/dht/routing_table.o \
	../lib/dht/leafset.o \
	../lib/dht/check_leafset_job.o

COMPILE=g++ -O0 -g -Wall -Wextra -I.. -I../lib -I../lib/jobs -I../lib/net -I../lib/dht

.SUFFIXES: .cpp .o .c
%.o: %.cpp
	@echo '$(COMPILE) -o $@ -c $<'; \
	$(COMPILE) -o $@ -c $<

%.o: %.c
	gcc -o $@ -c $<

all: $(FILES)
	g++ -o main $(FILES) -lssl -lcrypto -lpthread

clean:
	rm -rf $(FILES) main
