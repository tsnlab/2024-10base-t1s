LIB	= libcommon.a

OBJS	= common.o lib_menu.o helper.o log.o

INCLUDES = 

CFLAGS = -I./ -I../ -I../include/ -I../include/libcom 

all:	$(LIB) $(OBJS)

$(LIB):	.depend $(OBJS)
	$(AR) crv $@ $(OBJS)
	cp -f $@ ../lib

#########################################################################

.depend:	Makefile $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(INCLUDES) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################

clean :
	rm -rf *.o *.a
