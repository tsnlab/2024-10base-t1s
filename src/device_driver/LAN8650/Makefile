LIB	= libdevice_drv.a

OBJS	= arp_test.o register.o spi.o drv_main.o

INCLUDES = 

CFLAGS = -I./

all:	$(LIB) $(OBJS)

$(LIB):	.depend $(OBJS)
	$(AR) crv $@ $(OBJS)
	cp -f $@ ../../test_app/lib

#########################################################################

.depend:	Makefile $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(INCLUDES) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################

clean :
	rm -rf *.o *.a
