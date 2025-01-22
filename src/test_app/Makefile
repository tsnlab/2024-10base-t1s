HOSTARCH := $(shell uname -m | \
	sed -e s/i.86/i386/ \
	    -e s/sun4u/sparc64/ \
	    -e s/arm.*/arm/ \
	    -e s/sa110/arm/ \
	    -e s/macppc/ppc/)

HOSTOS := $(shell uname -s | tr A-Z a-z)

CROSS_COMPILE =
export	CROSS_COMPILE HOSTARCH

TOPDIR	:= $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)
export	TOPDIR
SRCDIR	:= $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)
export	SRCDIR

include $(TOPDIR)/Makefile.inc

EXE = xbase-t1s

SUBDIRS	= ../device_driver/LAN8650 \
		  libcli \
		  app

SRCDIRS	= include \
	      libcli

CLEANDIR = $(SUBDIRS)

#########################################################################

OBJS  =	libcli/libcommon.a

all: $(EXE)

install: all

$(EXE):
	@for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir || exit 1 ; done

subdirs:
	@for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir || exit 1 ; done

depend dep:
	@for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir .depend ; done

touch:
	@for dir in $(SRCDIRS) ; do touch $$dir/* ; done

clean:
	rm -f `find . -type f \
		\( -name 'core' -o -name '*.bak' -o -name '*~' \
		-o -name '*.o'  -o -name '*.a' -o -name '.depend' \) -print`
	@for d in $(CLEANDIR); do $(MAKE) -C $$d clean; done
