
//FINAL VERSION
//SANJAY
//306258619

#include <iostream>
#include <bitset>
using namespace std;

enum type 
{
	Rtype, 
    Itype, 
    Stype, 
    Btype, 
    Utype, 
    Jtype
};

enum ALUop
{
    ZERO, //00
    ONE, //01
    TWO, //10
    THREE, //11
};

enum command 
{
    NOOP,
	ADD, 
    OR, 
    XOR, 
    SRA, 
    SUB
    
};

struct DataProcess
{
	DataProcess() {}; // constructor

    bitset<32> instr;//instruction
    std::bitset<7> opcode;    // bits 6-0
    std::bitset<5> rd;        // bits 11-7
    std::bitset<3> funct3;    // bits 14-12
    std::bitset<5> rs1;       // bits 19-15
    std::bitset<5> rs2;       // bits 24-20
    std::bitset<7> funct7;    // bits 31-25
    /* NOT ACTUALLY USING THESE ANYMORE JUST DIRECTLY WRITING INTO IMM
    std::bitset<12> Iimm;     // For I-type
    std::bitset<20> Uimm;     // For LUI
    std::bitset<12> Simm;     // For S-type
    std::bitset<13> Bimm;     // For B-type
    std::bitset<21> Jimm;     // For J-type (JAL)
    */
};

struct Controller
{
    type InstructionType;
    ALUop ALU_Op;
    //BOOLS
    bool regWrite;
    bool ALUSrc;
    bool Branch;
    bool Jump;
    bool memRead;
    bool memWrite;
};

class CPU {
public:
	CPU();
	unsigned long readPC() { return PC; };
	void incPC() { PC++; };

    //THIS HERE IS OUR COMPLETE CPU STEP by STEP
    //decode -> control -> immediate gen -> ALU control -> ALU -> Memory -> Write Back rd -> next PC
	void decode(bitset<32> instr); //1
    void Control(); //2
    void ImmGenerator(); //3
    void ALUControl(); //4
	void ALU(); //5
    void LoadandStore(); //6
    void WriteBack(); //7
    void NEXT_PC(); //8

    //Memory and registers are int values
	int32_t dmemory[4096];
	int32_t regfile[32]; 

    bool ZeroFlag;
    bool isWord;
    //DEBUG
    void printRegisters()
    {
        for(int k = 0; k < 32; k++)
        {
            cout << k << ": " << regfile[k] << endl;
        }
    }
	unsigned long PC; //pc 

    DataProcess dp;
    Controller control;

    //FINAL REG VALUES
	uint32_t rs1;
	uint32_t rs2;
	uint32_t rd;

    //FINAL IMM VALUES
    int32_t result;
    int32_t imm;


    //HELPERS
	command operation; //this is the result of out ALU Control
};

bitset<32> instructions(uint32_t tempPC, char instMem[]);
