# makefile

all: consumer producer

consumer: consumer.cpp
	g++ -g -w -std=c++11 -o consumer consumer.cpp -lpthread -lrt

producer: producer.cpp
	g++ -g -w -std=c++11 -o producer producer.cpp -lpthread -lrt

clean:
	rm -rf *.o consumer producer
