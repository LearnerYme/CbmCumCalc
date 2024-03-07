all: runCumulant

runCumulant: Cumulant.cpp ECorr.o Loader.o CentralityTool.o
	g++ $^ -o $@ `root-config --libs --cflags` -O3 -std=c++17 

cbwc: Cbwc.cpp Loader.o CentralityTool.o
	g++ $^ -o $@ `root-config --libs --cflags` -O3 -std=c++17 

ECorr.o: ECorr.cpp
	g++ -c $^ -o $@ `root-config --libs --cflags` -O3 -std=c++17 

Loader.o: Loader.cxx
	g++ -c $^ -o $@ `root-config --libs --cflags` -O3 -std=c++17 

CentralityTool.o: CentralityTool/CentralityTool.cxx
	g++ -c $^ -o $@ `root-config --libs --cflags` -O3 -std=c++17 