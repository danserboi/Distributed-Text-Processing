build:
	mpicc main.c glist.c master.c worker.c -o main -lpthread
clean:
	rm main
