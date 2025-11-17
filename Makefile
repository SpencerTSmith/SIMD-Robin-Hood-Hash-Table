run:
	g++ -O3 reptest.cpp -mavx2 -o reptest.x -fpermissive
	./reptest.x 7 2048
