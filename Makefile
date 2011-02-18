all: rippled tests/ripple_tests

clean:
	rm *.o rippled tests/ripple_tests ripple.db tests/ripple.db

rippled: rippled.cpp Ripple.o RippleInterface.o ripple.db mongoose.o
	g++ -g -o rippled rippled.cpp Ripple.o RippleInterface.o mongoose.o -lsoci_sqlite3 -lsoci_core -lpthread

tests/ripple_tests: tests/ripple_tests.cpp Ripple.o tests/ripple.db
	g++ -g -o tests/ripple_tests tests/ripple_tests.cpp Ripple.o -lsoci_sqlite3 -lsoci_core -lboost_unit_test_framework

Ripple.o: Ripple.cpp Ripple.hpp RippleTask.hpp RippleUser.hpp RippleLog.hpp
	g++ -g -c Ripple.cpp -I/usr/local/include/soci 

RippleInterface.o: RippleInterface.hpp RippleInterface.cpp 
	g++ -g -c RippleInterface.cpp 

mongoose.o: mongoose/mongoose.c
	gcc -g -c mongoose/mongoose.c

ripple.db: ripple_schema.sql
	sqlite3 ripple.db < ripple_schema.sql

tests/ripple.db: ripple_schema.sql
	sqlite3 tests/ripple.db < ripple_schema.sql

# DO NOT DELETE

Ripple.o: Ripple.hpp RippleUser.hpp RippleException.hpp RippleTask.hpp
Ripple.o: RippleDefines.hpp RippleLog.hpp /usr/include/time.h
Ripple.o: /usr/include/features.h /usr/include/bits/predefs.h
Ripple.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
Ripple.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h
Ripple.o: /usr/include/bits/time.h /usr/include/bits/types.h
Ripple.o: /usr/include/bits/typesizes.h /usr/include/xlocale.h JSON.hpp
rippled.o: Ripple.hpp RippleUser.hpp RippleException.hpp RippleTask.hpp
rippled.o: RippleDefines.hpp RippleLog.hpp /usr/include/time.h
rippled.o: /usr/include/features.h /usr/include/bits/predefs.h
rippled.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
rippled.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h
rippled.o: /usr/include/bits/time.h /usr/include/bits/types.h
rippled.o: /usr/include/bits/typesizes.h /usr/include/xlocale.h
rippled.o: RippleInterface.hpp /usr/include/stdlib.h
rippled.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
rippled.o: /usr/include/endian.h /usr/include/bits/endian.h
rippled.o: /usr/include/bits/byteswap.h /usr/include/sys/types.h
rippled.o: /usr/include/sys/select.h /usr/include/bits/select.h
rippled.o: /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h
rippled.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
rippled.o: mongoose/mongoose.h
RippleInterface.o: /usr/include/stdlib.h /usr/include/features.h
RippleInterface.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
RippleInterface.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
RippleInterface.o: /usr/include/gnu/stubs-64.h /usr/include/bits/waitflags.h
RippleInterface.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
RippleInterface.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
RippleInterface.o: /usr/include/sys/types.h /usr/include/bits/types.h
RippleInterface.o: /usr/include/bits/typesizes.h /usr/include/time.h
RippleInterface.o: /usr/include/bits/time.h /usr/include/xlocale.h
RippleInterface.o: /usr/include/sys/select.h /usr/include/bits/select.h
RippleInterface.o: /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h
RippleInterface.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
RippleInterface.o: /usr/include/stdio.h /usr/include/libio.h
RippleInterface.o: /usr/include/_G_config.h /usr/include/wchar.h
RippleInterface.o: /usr/include/bits/stdio_lim.h
RippleInterface.o: /usr/include/bits/sys_errlist.h /usr/include/assert.h
RippleInterface.o: /usr/include/string.h RippleInterface.hpp
RippleInterface.o: mongoose/mongoose.h Ripple.hpp RippleUser.hpp
RippleInterface.o: RippleException.hpp RippleTask.hpp RippleDefines.hpp
RippleInterface.o: RippleLog.hpp JSON.hpp
