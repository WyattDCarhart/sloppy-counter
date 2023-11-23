.SILENT: all sloppySim clean

all: sloppySim
	echo "\nTesting w/ default arguments (few threads, i/o bound)\n" > testing.txt
	time ./sloppySim 2 10 10 100 false true >> testing.txt 2>&1
	echo "\nNow with many threads (probably a little slower, but very similar because threads can work while others sleep)\n" >> testing.txt
	time ./sloppySim 30 10 10 100 false true >> testing.txt 2>&1
	echo "\nTesting defaults but CPU Bound (a little slower than i/o bound, but not enough threads to cause many collisions in the CPU)\n" >> testing.txt
	time ./sloppySim 2 10 10 100 true true >> testing.txt 2>&1
	echo "\nNow CPU bound with many threads (significantly slower, as 'working' threads hog CPU cores\n" >> testing.txt
	time ./sloppySim 20 10 10 100 true true >> testing.txt 2>&1

sloppySim: sloppy.cpp
	g++ --std=c++17 -o sloppySim sloppy.cpp

clean:
	rm sloppySim testing.txt