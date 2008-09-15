
#
#
HERE = `pwd | sed "sX.*/XX"`

all:
	@echo "Please read the file README first"
	cd g21k ;\
	./configure --target=21k --host=i686-unknown-linux ;\
	make clean;\
	make

which.host:
	echo -n "HOST=" >which.host
	./config.guess >>which.host

where:
	@echo ${HERE}

build-g21k: which.host
	. which.host ;\
	cd g21k ;\
	./configure --target=21k --host=$$HOST ;\
	make clean;\
	make

install-g21k:
	cd g21k ;\
	make install

build-g21c: which.host
	. which.host ;\
	cd g21c ;\
	./configure --target=21csp --host=$$HOST ;\
	make clean;\
	make

install-g21c:
	cd g21k ;\
	make install

build-g21: which.host
	. which.host ;\
	cd g21xx ;\
	./configure --target=21xx --host=$$HOST ;\
	make clean;\
	make

install-g21:
	cd g21xx ;\
	make install

install-man:
	cd g21k ;\
	make install-man

clean:
	-(cd g21k  ; make clean)
	-(cd g21xx ; make clean)

distclean:
	-(cd g21k ;  make distclean)
	-(cd g21xx ; make distclean)

realclean:
	-(cd g21k ;  make realclean)
	-(cd g21xx ; make realclean)

fullclean: which.host
	. which.host;\
	cd g21k ;\
	configure --target=21k --host=$$HOST ;\
	make fullclean
	. which.host;\
	cd g21xx ;\
	configure --target=21xx --host=$$HOST ;\
	make fullclean
	-rm -f *~* *.orig out *.jps* which.host

dist: fullclean
	echo "cd ..;" tar zcvf ${HERE}.tar.gz ${HERE} | sh
