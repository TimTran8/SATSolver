#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <stdio.h>
// #include "stack.h"
#include "math.h"
#include <iterator>
#include <vector>
// #include <unistd.h>
#include <omp.h>
#include <sys/time.h>
// #include <thread>
#include <chrono>

using namespace std;

// bool solutionFound = false;
int clauseCount = 0; // total clause count
unsigned int varCount = 0; // total unique variables 
int literalCount = 0; // total literals in input file
// int varTotal = 0; // total variable count in input file
int backtrackCounter = 0;

typedef struct {
	bool satisfied;
	int litPos;
	int sat;
}clause;

typedef struct {
	// int limit;
	// int counter;
	// vector<int> varSat;
	bool sat;
	int value;
}variable;

void getClauseInfo(ifstream& file) {
	string text;
	int pCounter = 0;
	bool done = false;
	// int numberOfLines = 0;
	while(getline(file, text)) {
		// if (done == true) {
		//     break;
		// }
		stringstream ss(text);
		for(string s; ss >> s;) {
			pCounter++;
			if (s == "c") {
				// cout << "skipping comment" << endl;
				break; // skip to next line
			}
			else if (s == "p") {
				continue;
			}
			else if (s == "cnf" || s == "CNF" || s == "txt") {
				continue;
			}
			else if (pCounter == 3){
				varCount = stoi(s);
				// cout << "varCount: " << varCount << endl;
				continue;
			}
			else if (pCounter == 4) {
				clauseCount = stoi(s);
				// cout << "clauseCount: " << clauseCount << endl;
				done = true;
				break;
			}
			else {
				literalCount++;
				// cout << "number of literals: " << literalCount << endl;
			}
			// clauseCount++;
		}
		// cout << "next line " << endl;
		if (done == false) {
			// cout << "testing" << endl;
			pCounter = 0;
		}
		// numberOfLines++;
	}
	literalCount = literalCount - clauseCount;
	// cout << "total literals: " << literalCount << endl;

	// return numberOfLines;
}

void fillInput(ifstream& file, int *input, clause **litPosArr) {
	// cout << "in fillInput" << endl;
	string text;
	// int cCount = 0; // clauseCount
	int vCount = 0; // variableCount
	int litCount = 0; // literal count
	int cCount = 0; // clause count
	int temp = 0;
	int litPos = 0; // line literal position
	// bool done = false;
	
	while(getline(file, text)) {
		// cout << "literal position: " << litPos << endl;
		litPosArr[cCount] = (clause*)malloc(sizeof(clause));
		stringstream ss(text);
		for(string s; ss >> s;) {
			if (s == "c") {
				// cout << "skipping comment" << endl;
				break; // skip to next line
			}
			else if (s == "p") {
				break;
			}
			else {
				temp = stoi(s);
				if (temp == 0) {
					continue; // skip 0
				}
				if (litPos == 0) {
					// cout << "cCount: " << cCount << endl;
					litPosArr[cCount]->litPos = vCount;
					// cout << "litPosArr: " << litPosArr[cCount]->litPos << endl;
					litPosArr[cCount]->satisfied = false;
					cCount++;
				}
				input[vCount] = temp;
				// cout << "input var: " << input[vCount] << endl;
			}
			litPos++;
			vCount++;
		}
		litPos = 0;
		litCount++;
		
	}
}

void removeNumber(int *numbers, int idx, int &size) {
	int i;
	for(i = idx; i < size - 1; i++)
		numbers[i] = numbers[i + 1];
	size--;
}

void removeDuplicate(int *numbers, int &size) {
	int i, j;
	int number;
	for(i = 0; i < size; i++) {
		number = numbers[i];
		for(j = i + 1; j < size; j++) {
			if(number == numbers[j]) {
				removeNumber(numbers, j, size); j--;			
			}
		}
	}
}

void printVector(vector<int> &solVector) {
	cout << "v: ";
	for (unsigned int i = 0; i < solVector.size(); i++) {
		cout << solVector[i] << " ";
	}
	cout << endl;
}

bool falsify(clause **clauseArray, vector<int> &solVector, int *litArray) {
	// check if any falsified with variables so far in solVector
	// undo false if any falsified
	int var, bar, upperBound;
	unsigned int length;
	int unsat = 0;
	for (int i = 0; i < clauseCount; i++) {
		if ((i+1) == clauseCount) {
			upperBound = literalCount;
			length = (literalCount-1) - clauseArray[i]->litPos;

		}
		else {
			upperBound = clauseArray[i+1]->litPos;
			length = upperBound - clauseArray[i]->litPos;
			
		}
		if (clauseArray[i]->satisfied == true) {
			// cout << "skipping" << endl;
			continue; // skip to next clause
		}
		for (int j = clauseArray[i]->litPos; j < upperBound;j++) {
			if (length <= solVector.size()) {
				for (unsigned int k = 0; k < solVector.size(); k++) {
					var = solVector[k];
					bar = var * -1;
					if (var == litArray[j] || bar == litArray[j]) {
						// cout << "houston, we got a problem" << endl;
						unsat++;
						// return;
					}
				}
			}
			// cout << "none matched, falsify" << endl;
		}
		if (unsat > 0) {
			return true;
		}
	}
	return false;
}

bool checkClause(clause **clauseArray) { // marks which clauses are satisfied
	for (int i = 0; i < clauseCount; i++) {
		if (clauseArray[i]->satisfied == false) {
			// cout << "clause " << i << " is unsatisfied" << endl;
		}
		else {
			// cout << "clause " << i << " satisfied" << endl;
		}
	}
	// while (solved == false) {
		// for (int i = 0; i < clauseCount; i++) {
		//     if (clauseArray[i]->satisfied == false) {
		//         for (int j = 0; j < clauseCount; j++) {
		//             clauseArray[j]->satisfied = false; // reset satisfied as soon as false detected
		//         }
		//         return true; // need to backtrack
		//     }
		// }
	//     solved = true;
	// }
	return false;
}
void satClause(clause **clauseArray, int *litArray, int var) { // marks which clauses are satisfied
	int upperBound;
	// #pragma omp parallel for
	for (int i = 0; i < clauseCount; i++) {
		if (clauseArray[i]->satisfied == true) {
			// cout << "skipping" << endl;
			continue; // skip to next clause
		}
		if ((i+1) == clauseCount) {
			upperBound = literalCount;
		}
		else {
			upperBound = clauseArray[i+1]->litPos;
		}
		// varSat.push_back(vector<int>());
		for (int j = clauseArray[i]->litPos; j < upperBound;j++) {
			if (var == litArray[j]) {
				// cout << "satisfied: " << litArray[j] << endl;
				clauseArray[i]->sat = 1;
				clauseArray[i]->satisfied = true;
			}
			else {
				// cout << "unsatisfied" << endl;
			}
		}
	}
}

void unsatClause(clause **clauseArray, int *litArray, vector<int> solVector) { // marks which clauses are satisfied
	int upperBound;
	int back = solVector.back();
	// bool unsat = false;
	for (int i = 0; i < clauseCount; i++) {
		bool match = false;
		// cout << "clause: " << i+1 << endl;
		if ((i+1) == clauseCount) {
			upperBound = literalCount;
		}
		else {
			upperBound = clauseArray[i+1]->litPos;
		}
		for (unsigned int k = 0; k < solVector.size() -1; k++) {
			if (match == true) {
				break; // go to next clause
			}
			// cout << "solVector: " << solVector[k] << " k: " << k << endl;
			// match = false;
			for (int j = clauseArray[i]->litPos; j < upperBound;j++) {
				// only unsat if no other literals are < var
				if (solVector[k] == litArray[j]) {
					// unsat = false;
					match = true;
					// cout << "litArray: " << litArray[j] << endl;
					// cout << "satisfied" << endl;
					break;
				}
				else {
					// cout << "litArray: " << litArray[j] << endl;
					// cout << "skip" << endl;
				}
			}
		}
		// cout << "match = " << match << endl;
		if (match == false) {
			for (int j = clauseArray[i]->litPos; j < upperBound;j++) {
				// only unsat if no other literals are < var
				if (back == litArray[j]) {
					// unsat = false;
					// match = true;
					// cout << "litArray: " << litArray[j] << endl;
					// cout << "satisfied" << endl;
					// cout << "unsatisfying clause: " << i+1 << endl;
					clauseArray[i]->sat = 0;
					clauseArray[i]->satisfied = false;
					break;
				}
			}
				
		}
		// cout << endl;
	}
}

void backtrack(vector<int> &solVector, clause **clauseArray, int *litArray, int *solArray) {
	// if backtracking, have undoer to unsatisfy previous variable's satisfaction of clauses
	#pragma omp parallel
	{

	backtrackCounter++;
	// cout<<"couter = "<< backtrackCounter<<endl;
	// int counter = 0;
	// int vSize = solVector.size();
	int back = solVector.back();
	// cout << "back: " << back << endl;
	while (back < 0) {
		unsatClause(clauseArray, litArray, solVector);
		solVector.pop_back();
		back = solVector.back();
	}
	unsatClause(clauseArray, litArray, solVector);
	solVector.pop_back();
	solVector.push_back(back * -1);
	satClause(clauseArray, litArray, solVector.back());
	if (solVector.size() < varCount) {
		for (unsigned int i = solVector.size(); i < varCount; i++) {
			solVector.push_back(solArray[i]);
			satClause(clauseArray, litArray, solVector.back());
		}
	}
	}
}

int startTime = time(NULL);
time_t start;
// bool isPrinting = false;
// struct timeval t0, t1;
// long int startime = gettimeofday(&t0, NULL);
// auto start = chrono::system_clock::now();
int checkdiff;
void printBacktrack(){
	// auto end = chrono::system_clock::now();

	// isPrinting = true;
	// if(isBacktracking){
		// long int endtime = gettimeofday(&t1, NULL);
		// long int difference = t1.tv_usec - t0.tv_usec;
	   start = time(NULL);
	   int diff = difftime(start, startTime);
	//    std::chrono::duration<double> diff = end-start;
		// cout<<"time "<<difftime(t_ini, startTime) <<endl;
		// if( fmod(diff.count(), 2 ) == 0 ){
			if(((diff % 2) == 0) && (diff != checkdiff)){
				cout << "time: "<< diff << endl;
				cout << "Number of backtracks: "<< backtrackCounter <<endl;
				
				checkdiff = diff;
			}
			// cout << "time :"<< diff.count() <<endl;
			
		// }
		// isPrinting = false;
		// this_thread::yield();
		// this_thread::sleep_for (std::chrono::seconds(2));
	// }     //sleep(2);
}

void reverse_array( int array[], int arraylength ) {
	for (int i = 0; i < (arraylength / 2); i++) {
		int temporary = array[i];                 // temporary wasn't declared
		array[i] = array[(arraylength - 1) - i];
		array[(arraylength - 1) - i] = temporary;
	}
}

int main(int argc, char** argv) {
	string fileName = argv[1];

	ifstream infile;
	infile.open(fileName);
	getClauseInfo(infile);           // gets varCount and clauseCount
	infile.close();
	
	// int litPosArray[clauseCount]; // array holding starting position of each clause beginning literal
	int litArray[literalCount]; // literals arrays containing all literals in file
	
	int solArray[literalCount]; // array to create tree
	vector<int> solVector;
	// typedef vector<variable> solVector;
	// clause *clauseArray = new clause[clauseCount];
	clause *clauseArray[clauseCount];
	vector< vector<int> > varSat; // stores which clauses satisfied in which variable
	// variable *varArray = new variable[varCount];

	infile.open(fileName);
	fillInput(infile, litArray, clauseArray);

	infile.close();
	for(int i = 0; i < literalCount; i++)
	{
		//cout << "litArray: " << litArray[i] << endl;
		solArray[i] = litArray[i];
		solArray[i] = abs(solArray[i]);
	}
	int totalLiterals = literalCount;
	sort(solArray, solArray+literalCount);
	removeDuplicate(solArray, literalCount);
	literalCount = totalLiterals;

	// for (int i = 0; i < varCount; i++) {
	//     // varArray[i].value = solArray[i];
	//     // cout << "varArray| value: " << varArray[i].value << endl;
	//     //cout << "solArray| value: " << solArray[i] << endl;

	//     // clauseArray[i]->sat = 0; //init sats to 0 where neither sat or unsat
	// }
	cout << "bout to fuck up some commas" << endl;
	// for (int i = 0; i < varCount; i++) {
	//     // vector<int> clauseSat[i];
	//     // varSat.push_back(clauseSat[i]);
	// }
	cout << endl;
	bool falsified;
	// bool bTrack;
	// int seconds = time(NULL);
	// seconds = seconds-seconds;
	// while ( bTrack != true) {
	#pragma omp parallel 
		for (unsigned int i = 0; i < varCount; i++) {
			#pragma omp critical
			{
				if (solVector.size() < varCount) {
					solVector.push_back(solArray[i]);
					// solVector.varSat.push_back(solArray[i]);
				}
				// cout << "solVector: " << solVector[i] << endl;
				satClause(clauseArray, litArray, solVector[i]); // change to check latest node
				// printVector(solVector);
				falsified = falsify(clauseArray, solVector, litArray);
				// if falsified, backtrack
				// checkClause(clauseArray, litArray, solVector);
				if (falsified == true) {
					// cout << endl << "backtracking" << endl;
					backtrack(solVector, clauseArray, litArray, solArray);
					i--;
					printBacktrack();
				}
			}
			// bTrack = checkClause(clauseArray, litArray, solVector);
			// if statement to reset i
			// cout << endl;
			// isPrinting = false;
		}
	// }
	cout << "Number of backtracks: "<< backtrackCounter <<endl;
	checkClause(clauseArray);
	printVector(solVector);

	return 0;
} 