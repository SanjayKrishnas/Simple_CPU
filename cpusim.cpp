#include "CPU.h"
#include <iostream>
#include <bitset>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;


int main(int argc, char* argv[]) 
{
	char instMem[4096]; //STORE EACH Byte char by char

	//ERRORS
	if (argc < 2) {
		return -1;
	}

	//Built in error checkers 
	ifstream infile(argv[1]); 
	if (!(infile.is_open() && infile.good())) {
		return 0; 
	}

	string line; 
	int i = 0;
	while (infile >> line) {
        std::stringstream input(line);
        char placeholder;

        // Read two characters (each represents a 4-bit value)
        for (int j = 0; j < 2; j++) {
            input >> placeholder; // Read the character
            instMem[i++] = placeholder; // Store in instMem
        }
    }
	int maxPC= i/8; //we divide by 8 since we have each instruction split by characters

	CPU myCPU;  
	bool done = true;

	while (done == true) { 
        //get instruction
		uint32_t tempPC = myCPU.readPC() * 8;

        //retrieve instruction
		bitset<32> instr = instructions(tempPC, instMem);

        //Data process stage
		myCPU.decode(instr);

		//Call Controller
		myCPU.Control();

		//Immediate generator
		myCPU.ImmGenerator();

		//ALu Controller
        myCPU.ALUControl();

        //ALU 
		myCPU.ALU();

		//MEM
		myCPU.LoadandStore();

		//WRITE BACK
		myCPU.WriteBack();

        //UPDATE PC
        myCPU.NEXT_PC();
        
		if (myCPU.readPC() >= maxPC)
        {
			break;
        }
	}
	//myCPU.printRegisters();

	int a0 = myCPU.regfile[10];
	int a1 = myCPU.regfile[11];
	cout << "(" << a0 << "," << a1 << ")" << endl;
	
	return 0;
}