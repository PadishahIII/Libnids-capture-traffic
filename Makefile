PCAPDIR    = ./lib
LIBPCAP    = ${PCAPDIR}/libpcap.a 

PFRINGDIR  = ./lib
LIBPFRING  = ${PFRINGDIR}/libpfring.a

PFRING_KERNEL= ./lib/kernel

LIBTHIRD   = ./lib/third-party

INCLUDE    = -I${PCAPDIR} -I${PFRINGDIR} -I${PFRING_KERNEL} -I${LIBTHIRD} `lib/pfring_config --include`
LIBS       = -lnids ${LIBPCAP} ${LIBPFRING} ${LIBPCAP} ${LIBPFRING} `lib/pfring_config --libs` `lib/pcap-config --additional-libs --static` -lrt -ldl -lrt -lgthread-2.0 -lglib-2.0 -lnet -lpthread 

CC         = gcc #--platform=native
O_FLAG     = -DHAVE_PF_RING 
WFLAGS     = -Wall -Wno-unused-function -Wno-format-truncation -Wno-address-of-packed-member
# CFLAGS     = -O2 ${O_FLAG} ${WFLAGS} ${INCLUDE} -D ENABLE_BPF -g
CFLAGS     = ${O_FLAG} ${INCLUDE} -D ENABLE_BPF -g


%.o: %.c 
	${CC} ${CFLAGS} -c $< -o $@

TARGETS    = nids processor

all: ${TARGETS}

nids : nids.o ${LIBPCAP} Makefile
	${CC} ${CFLAGS} nids.o ${LIBS} -o $@

processor : processor.o  Makefile
	${CC} ${CFLAGS} processor.o -o $@

clean:
	@rm -f ${TARGETS} *.o *~
