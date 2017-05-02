All : bwtsearch

bwtsearch : bwtsearch.o
	g++ bwtsearch.o -o bwtsearch

bwtsearch.o : bwtsearch.cpp
	g++ -c bwtsearch.cpp 


clean : 
	rm *.o bwtsearch

clean : 
	rm *.o bwtsearch
