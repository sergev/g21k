#
#  the 21k and Sharc binary utilities
#
#  Copyright (c) 1991-1997 Analog Devices, Inc.
#  Copyright (c) 1999 Egil Kvaleberg
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

-include config.def

DIRS = include lib modify cdump ldr link app asm doc

FILES = Makefile make.def config.def.in README TODO COPYING configure.in \
	acconfig.h install-sh acconfig.h configure config.h.in \
	g21k-binutils.spec g21k-binutils.spec.in

all: configure config.h
	@for a in ${DIRS}; do                     \
		if ! (cd $$a && ${MAKE} all) then \
			exit;                     \
		fi;                               \
	done

clean.here:
	rm -f *~ out

clean:  clean.here
	for a in ${DIRS}; do                      \
		(cd $$a && ${MAKE} clean)        ;\
	done

realclean: clean.here
	rm -f config.cache config.def config.log config.h config.status
	for a in ${DIRS}; do                      \
		(cd $$a && ${MAKE} realclean)    ;\
	done

distclean: clean.here
	rm -f configure config.h.in

dep:
	for a in ${DIRS}; do                      \
		(cd $$a && ${MAKE} dep)          ;\
	done

configure: configure.in
	${AUTOCONF}

config.h.in: configure.in
	${AUTOHEADER}

config.h: configure # config.h.in
	./configure

config.def: configure config.def.in
	./configure

install: all
	if ! [ -d ${etcdir} ]; then mkdir -p ${etcdir}; fi
	${INSTALL} asm/a21000    ${etcdir}
	${INSTALL} app/asm21k    ${bindir}
	${INSTALL} ldr/mem21k    ${bindir}
	${INSTALL} link/ld21k    ${bindir}
	${INSTALL} cdump/cdump   ${bindir}/cdump21k
	${INSTALL_DATA} doc/a21000.1 ${mandir}/man1
	${INSTALL_DATA} doc/asm21k.1 ${mandir}/man1
	${INSTALL_DATA} doc/mem21k.1 ${mandir}/man1
	${INSTALL_DATA} doc/ld21k.1  ${mandir}/man1

uninstall:
	rm -f                   ${etcdir}/a21000        \
				${bindir}/asm21k        \
				${bindir}/mem21k        \
				${bindir}/ld21k         \
				${bindir}/cdump21k      \
				${mandir}/man1/a21000.1 \
				${mandir}/man1/asm21k.1 \
				${mandir}/man1/mem21k.1 \
				${mandir}/man1/ld21k.1

dist:
	rm -rf g21k-binutils-${VERSION}
	mkdir g21k-binutils-${VERSION}
	cp -p ${FILES} g21k-binutils-${VERSION}
	for a in ${DIRS}; do                                     \
		mkdir g21k-binutils-${VERSION}/$$a              ;\
		cp -p $$a/*.[ch1] $$a/Makefile g21k-binutils-${VERSION}/$$a ;\
	done
	tar zcvf g21k-binutils-${VERSION}.tar.gz g21k-binutils-${VERSION}
	rm -rf g21k-binutils-${VERSION}

