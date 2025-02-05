include $(TOPDIR)/Makefile.inc

APP_XBASE_T1S_OBJS = xbase-t1s.o command_parser.o thread.o buffer_handler.o icmp.o

OBJS = $(APP_XBASE_T1S_OBJS)

LIBS = $(TOPDIR)/lib/libcommon.a $(TOPDIR)/lib/libdevice_drv.a
LIBS_INCLUDE = -L.. -L../lib -lcommon -ldevice_drv -lpigpio -lpthread -lreadline -lm -lrt

CFLAGS += -I.. -I../include -I../include/libcom -I../include/api_device

all: xbase-t1s

xbase-t1s: .depend $(LIBS) $(APP_XBASE_T1S_OBJS)
	$(CC) $(CFLAGS) $(APP_XBASE_T1S_OBJS) -o $@ $(LIBS_INCLUDE)

clean:
	rm -rf *.o *~ core .depend
	rm -rf xbase-t1s 

#########################################################################

.depend:	Makefile $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
