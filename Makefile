Jarvis:Jarvis.cc
	g++ -std=c++11 -o Jarvis Jarvis.cc -Ispeech -ljsoncpp -lcurl -lcrypto -lpthread
.PHONY:clean
clean:
	rm -f Jarvis
