all: rippled

rippled: rippled.cpp Ripple.hpp RippleTask.hpp RippleUser.hpp
	g++ -g -o rippled rippled.cpp -I/usr/local/include/soci -lsoci_sqlite3 -lsoci_core
