FILES=main.o \
	../common/dtime.o \
	../common/tools.o \
	../common/mutex.o \
	../common/pf_thread.o \
	../common/pf_log.o \
	../common/scheduler.o \
	../common/scheduler_queue.o \
	../common/files/file_chunk.o \
	../common/jobs/job.o \
	../common/net/network.o \
	../common/net/packet.o \
	../common/net/packet_type.o \
	../common/net/packet_type_list.o \
	../common/net/host.o \
	../common/net/hosts_list.o \
	../common/net/pf_addr.o \
	../common/dht/chimera.o \
	../common/dht/key.o \
	../common/dht/chimera_messages.o \
	../common/dht/chimera_routing.o \
	../common/dht/routing_table.o \
	../common/dht/leafset.o \
	../common/dht/check_leafset_job.o

COMPILE=g++ -O0 -g -Wall -Wextra -I.. -I../common -I../common/jobs -I../common/net -I../common/dht

.SUFFIXES: .cpp .o .c
%.o: %.cpp
	@echo '$(COMPILE) -c $<'; \
	$(COMPILE) -c $<

%.o: %.c
	gcc -c $<

all: $(FILES)
	g++ -o main `ls *.o` -lssl -lcrypto -lpthread

clean:
	rm -rf *.o main
