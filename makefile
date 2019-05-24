CXX = g++
CXXFLAGS = -g -W -Wall -Wextra -Wpedantic -fopenmp -march=native -std=c++11

sat: sat.cpp 
	$(CXX) $(CXXFLAGS) sat.cpp -o output

clean:
	rm -f *.o core output *.out part*