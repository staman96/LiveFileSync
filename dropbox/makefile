CC = g++

mirror_client: main.o commonIDs.o child.o interparent.o list.o
	$(CC) -lm -g main.o commonIDs.o child.o interparent.o list.o -o mirror_client

commonIDs.o: commonIDs.cpp commonIDs.h
	$(CC) -c -g commonIDs.cpp

child.o: child.cpp child.h
	$(CC) -c -g child.cpp

interparent.o: interparent.cpp interparent.h child.h
	$(CC) -c -g interparent.cpp

list.o : list/list.cpp list/list.h
	$(CC) $(CFLAGS) -c list/list.cpp

main.o: main.cpp commonIDs.h child.h interparent.h list/list.h
	$(CC) -c -g main.cpp

clean:
	rm -rvf *.o mirror_client