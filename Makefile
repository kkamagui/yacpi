# ----------------------------------------------
#  YACPI Makefile by Nico Golde <nico@ngolde.de>
#  Latest change: Mi Aug 30 14:08:37 CEST 2006
#  ---------------------------------------------

BIN     = yacpi
prefix  = /usr/local
INSPATH = ${prefix}/bin/
CFLAGS  = -O2 -Wall -g -DVERSION=\"${VERSION}\"
CC      = cc
DOCPATH = ${prefix}/share/doc/yacpi
MANPATH = ${prefix}/share/man/man1
VERSION = 3.0.1
SRC_yacpi       = yacpi.c
SRC_get_cpu     = get_cpu.c
OBJ_yacpi       = ${SRC_yacpi:.c=.o}
OBJ_get_cpu     = ${SRC_get_cpu:.c=.o}

.c.o :
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

all : options ${OBJ_yacpi} ${OBJ_get_cpu}
	@${CC} -Wall get_cpu.o yacpi.o -o ${BIN} -lncurses -lacpi
	@strip ${BIN}
	@echo built yacpi

dist : clean
	@mkdir -p yacpi-${VERSION}
	@cp -R Makefile CHANGELOG README THANKS COPYING *.c *.h yacpi.1 yacpi-${VERSION}
	@tar -cf yacpi-${VERSION}.tar yacpi-${VERSION}
	@gzip yacpi-${VERSION}.tar
	@rm -rf yacpi-${VERSION}
	@echo created distribution yacpi-${VERSION}.tar.gz

options : 
	@echo yacpi build options:
	@echo "CFLAGS = ${CFLAGS}"
	@echo "CC     = ${CC}"
	@echo "BIN    = ${BIN}"

install :
	@mkdir -p ${DOCPATH}
	@mkdir -p ${INSPATH}
	@mkdir -p ${MANPATH}
	install -m644 CHANGELOG README THANKS COPYING ${DOCPATH}
	install -m644 yacpi.1 ${MANPATH}
	install ${BIN} ${INSPATH}

uninstall :
	rm -f ${INSPATH}${BIN}
	rm -rf ${DOCPATH}
	rm -f ${MANPATH}/yacpi.1*

clean :
	rm -f ${BIN}
	rm -f *.o

love :
	@echo "not war!"
