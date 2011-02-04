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
