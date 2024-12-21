#include "CPU.h"
#include <iostream>
#include <bitset>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;

// Constructor to initialize CPU
CPU::CPU() {
    PC = 0; // Set program counter to 0
    for(int i = 0; i < 32; i++)
    {
        regfile[i] = 0;
    }
    // Initialize data memory to 0
    for (int i = 0; i < 4096; i++) 
    {
        dmemory[i] = 0;
    }
}

// Decode function to parse a 32-bit instruction
void CPU::decode(bitset<32> instr) 
{
    dp.instr = instr;

    // Extract opcode (bits [0:6])
    for(int i = 0; i < 7; i++) {
        dp.opcode[i] = instr[i];
    }

    // Extract rd (bits [7:11]), funct3 (bits [12:14]), rs1 (bits [15:19]), rs2 (bits [20:24]), funct7 (bits [25:31])
    for(int i = 0; i < 5; i++) {
        dp.rd[i] = instr[i + 7];
    }
    rd = dp.rd.to_ulong(); // Convert to unsigned long

    for(int i = 0; i < 3; i++) {
        dp.funct3[i] = instr[i + 12]; //get funct3
    }

    for(int i = 0; i < 5; i++) {
        dp.rs1[i] = instr[i + 15]; //get rs1
    }
    rs1 = dp.rs1.to_ulong();

    for(int i = 0; i < 5; i++) {
        dp.rs2[i] = instr[i + 20]; //get rs2
    }
    rs2 = dp.rs2.to_ulong();

    for(int i = 0; i < 7; i++) {
        dp.funct7[i] = instr[i + 25]; //get funct7
    }
}

void CPU::Control()
{
    // CONTROL
    if(dp.opcode == bitset<7>("0110011"))
    {
        control.ALU_Op = TWO; //R OR I Type

        control.InstructionType = Rtype;
        control.regWrite = true;
        control.Branch = false;
        control.Jump = false;
        control.memRead = false;
        control.memWrite = false;
        control.ALUSrc = false;
    }
    else if(dp.opcode == bitset<7>("0010011"))
    {
        control.ALU_Op = TWO;

        control.InstructionType = Itype;
        control.regWrite = true;
        control.Branch = false;
        control.Jump = false;
        control.memRead = false;
        control.memWrite = false;
        control.ALUSrc = true;
    }
    else if(dp.opcode == bitset<7>("0000011"))
    {
        control.ALU_Op = TWO;

        control.InstructionType = Itype;
        control.regWrite = true;
        control.Branch = false;
        control.Jump = false;
        control.memRead = true;
        control.memWrite = false;
        control.ALUSrc = true;
    }
    else if(dp.opcode == bitset<7>("0110111"))
    {
        control.ALU_Op = THREE; //NOOP

        control.InstructionType = Utype;
        control.regWrite = true;
        control.Branch = false;
        control.Jump = false;
        control.memRead = false;
        control.memWrite = false;
        control.ALUSrc = true;
    }
    else if(dp.opcode == bitset<7>("0100011"))
    {
        control.ALU_Op = ZERO; //ADD always

        control.InstructionType = Stype;
        control.regWrite = false;
        control.Branch = false;
        control.Jump = false;
        control.memRead = false;
        control.memWrite = true;
        control.ALUSrc = true;
    }
    else if(dp.opcode == bitset<7>("1100011"))
    {
        control.ALU_Op = ONE; //SUB ALWAYS

        control.InstructionType = Btype;
        control.regWrite = false;
        control.Branch = true;
        control.Jump = false;
        control.memRead = false;
        control.memWrite = false;
        control.ALUSrc = false;
    }
    else if(dp.opcode == bitset<7>("1101111"))
    {
        control.ALU_Op = THREE; //NOOP

        control.InstructionType = Jtype;
        control.regWrite = true;
        control.Branch = false;
        control.Jump = true;
        control.memRead = false;
        control.memWrite = false;
        control.ALUSrc = true;
    }
}

void CPU::ImmGenerator()
{
    // Generate immediate based on instruction type
    bitset<32> immediate;
    immediate.reset();
    switch (control.InstructionType) {
        case Rtype:
            // R-type does not use immediate
            break;
        case Itype:
            // I-type immediate (bits [20:31])
            for(int i = 0; i < 12; i++) {
                immediate[i] = dp.instr[i + 20];
            }

             // Sign-extend by setting upper bits if bit 11 is set
            if (immediate[11] == 1) {
                for (int i = 12; i < 32; i++) {
                    immediate[i] = 1;
                }
            }
            imm = static_cast<int32_t>(immediate.to_ulong());
            break;
        case Utype:
            // U-type immediate (bits [12:31])
            // U-type immediate (bits [12:31]), no sign extension needed
            for (int i = 0; i < 20; i++) {
                immediate[i] = dp.instr[i + 12];
            }
            imm = immediate.to_ulong();
            imm = imm << 12;
            break;
        case Stype:
            // S-type immediate split across bits [7:11] and [25:31]
            for(int i = 0; i < 5; i++) {
            immediate[i] = dp.instr[i + 7];
            }
            for(int i = 0; i < 7; i++) {
                immediate[i + 5] = dp.instr[i + 25];
            }
            // Sign-extend by setting upper bits if bit 11 is set
            if (immediate[11] == 1) {
                for (int i = 12; i < 32; i++) {
                    immediate[i] = 1;
                }
            }
            imm = static_cast<int32_t>(immediate.to_ulong());
            break;
        case Btype:
            // B-type immediate (bits [31, 7, [25:30], [8:11]])
            immediate[0] = 0;  // Not part of the immediate, LSB is unused
            for (int i = 0; i < 4; i++) {
                immediate[i + 1] = dp.instr[i + 8];
            }
            for (int i = 0; i < 6; i++) {
                immediate[i + 5] = dp.instr[i + 25];
            }
            immediate[11] = dp.instr[7];
            immediate[12] = dp.instr[31];
            // Sign-extend by setting upper bits if bit 12 is set
            if (immediate[12] == 1) {
                for (int i = 13; i < 32; i++) {
                    immediate[i] = 1;
                }
            }
            imm = static_cast<int32_t>(immediate.to_ulong());
            break;
        case Jtype:
            // J-type immediate (bits [31, [12:19], 20, [21:30]])
            immediate[0] = 0;  // Set LSB to 0, not part of the J-type immediate
            for (int i = 0; i < 10; i++) {
                immediate[i + 1] = dp.instr[i + 21];
            }
            immediate[11] = dp.instr[20];
            for (int i = 0; i < 8; i++) {
                immediate[i + 12] = dp.instr[i + 12];
            }
            immediate[20] = dp.instr[31];
           
            // Sign-extend by setting upper bits if bit 20 is set
            if (immediate[20] == 1) {
                for (int i = 21; i < 32; i++) {
                    immediate[i] = 1;
                }
            }
            imm = static_cast<int32_t>(immediate.to_ulong());
            break;
        default:
            break;
    }
}

void CPU::ALUControl()
{
    //ALU CONTROL
    switch (control.ALU_Op) 
    {
        case TWO: //ALL RTYPE ITYPE INSTRUCTIONS
            if (dp.opcode == bitset<7>("0000011")) //LW LB
            { 
                if (dp.funct3 == bitset<3>("000")) //This is LB
                { 
                    operation = ADD;
                    isWord = false;
                } 
                else if (dp.funct3 == bitset<3>("010")) //LW
                { 
                    operation = ADD;
                    isWord = true;
                }
            } 
            else if (dp.funct3 == bitset<3>("000")) //R type ADD
            { 
                operation = ADD;
            } 
            else if (dp.funct3 == bitset<3>("100")) //XOR
            { 
                operation = XOR;
            }
            else if (dp.funct3 == bitset<3>("110")) //OR or ORI but we only use ORI
            { 
                operation = OR;
            } 
            else if (dp.funct3 == bitset<3>("101")) //SRAI since we don't use SRA
            { 
                operation = SRA;
            }
            break;
        case ZERO:
            if (dp.funct3 == bitset<3>("000")) //This is for SB
            {
                operation = ADD;
                isWord = false;
            } 
            else if (dp.funct3 == bitset<3>("010")) //SW
            {
                operation = ADD;
                isWord = true;
            }
            break;
        case ONE: //All Branch instructions (B- Type)
            operation = SUB;
            break;
        case THREE: //LUI and Jump have ALU do nothing so we have a NOOP - no operation 
            operation = NOOP;
            break;
        default:
            break;
    }
}

void CPU::ALU() {
    //This includes ALU
    //Write Back stage
    int32_t OPERAND2;
    if (control.ALUSrc == false) OPERAND2 = regfile[rs2];
    else if (control.ALUSrc == true) OPERAND2 = imm;

    switch (operation) {
        case ADD:
            result = regfile[rs1] + OPERAND2;
            break;
        case XOR:
            result = regfile[rs1] ^ OPERAND2;
            break;
        case OR:
            result = regfile[rs1] | OPERAND2;
            break;
        case SRA:
            result = regfile[rs1] >> OPERAND2;
            break;
        case SUB:
            //WE CHECK ZERO FLAG for BEQ
            ZeroFlag = ((regfile[rs1] - OPERAND2) == 0);
        case NOOP: //Simply return just the immediate
            result = imm;
            break;

        default:
            break;
    }
}

void CPU::WriteBack()
{
    if(control.regWrite == false) return;

    if(control.Jump) //If jump then we must change return register to PC + 1
    {
        regfile[rd] = (PC + 1) * 4;
    }
    else
    {
        regfile[rd] = result;
    }
}

void CPU::LoadandStore()
{
    if (control.memRead && !isWord) //This indicates that we have a load word
    {
        result = dmemory[result] & 0xff; //Extract lower 8 btis
    }
    else if(control.memRead && isWord)
    {
        result = dmemory[result]; //Full 32 bits
    }
    else if(control.memWrite && !isWord)
    {
        dmemory[result] = regfile[rs2] & 0xff; //Extract only the lower 8 bits
    }
    else if(control.memWrite && isWord)
    {
        dmemory[result] = regfile[rs2]; //Full 32 bits
    }

}

void CPU::NEXT_PC() 
{

    if (ZeroFlag && control.Branch) //If we have a zero flag and a branch then BEQ is true
    {
        PC += imm/4;
    }
    else if (control.Jump) //If we have a jump command
    {
        PC += imm/4;
    }
    else //FOR ANY OTHER COMMAND JUST DO PC++
    {
        incPC();
    }
    //PC increment Stage
}

//HELPER FUNCTIONS

bitset<32> instructions(uint32_t tempPC, char instMem[]) {
	// Create a string to hold the full instruction
    string instruction;

    // Append the instruction bytes in the specified order
    instruction += instMem[tempPC + 6]; //Little Endian ordering so this is a must do
    instruction += instMem[tempPC + 7];
    instruction += instMem[tempPC + 4];
    instruction += instMem[tempPC + 5];
    instruction += instMem[tempPC + 2];
    instruction += instMem[tempPC + 3];
    instruction += instMem[tempPC + 0];
    instruction += instMem[tempPC + 1];

    //This convert our chars into bit representation
    stringstream sstream;
	uint32_t shift;

	sstream << hex << instruction; 
	sstream >> shift;

    //we then return a full 32 bit instruction which is 8 total characters
	bitset<32> bit_instruction(shift);

	return bit_instruction;
}