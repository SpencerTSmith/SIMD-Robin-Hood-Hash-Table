run:
	g++ -O3 -g -DDEBUG reptest.cpp -mavx2 -o reptest.x -fpermissive
	./reptest.x 3 2048
