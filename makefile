CFLAGS:=-c -Wall -Weffc++ -g -std=c++11 -Iinclude
LDFLAGS:=-lboost_system -pthread

all: Target
	g++ -o bin/StompBookClubClient bin/connectionHandler.o bin/bookClubClient.o bin/stompClient.o $(LDFLAGS)

Target: bin/bookClubClient.o bin/stompClient.o bin/connectionHandler.o

bin/connectionHandler.o: src/connectionHandler.cpp
	g++ $(CFLAGS) -o bin/connectionHandler.o src/connectionHandler.cpp



bin/bookClubClient.o: src/bookClubClient.cpp
	g++ $(CFLAGS) -o bin/bookClubClient.o src/bookClubClient.cpp


bin/stompClient.o: src/stompClient.cpp
	g++ $(CFLAGS) -o bin/stompClient.o src/stompClient.cpp


.PHONY: clean
clean:
	rm -f bin/*