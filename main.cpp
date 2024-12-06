#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using namespace std;

//RETURN VARIABLES
int hits = 0;
int misses = 0;
int writebacks = 0;
int broadcasts = 0;
int cache_transfer = 0; //cache-to-cache-transfer or forwardings

//TO DO might need to change broadcasting rules

void updateCore(int caches[4][4][4], int core, int tag, int cacheCS, int newCS) //This is my function to update the CS of other Cores 
{
    if(cacheCS == -1) return; //This means the TAG is not in the Cache so it doesn't matter

    if(newCS == 4)
    {
        for(int i = 0; i < 4; i++)
        {
            if(caches[core][i][2] == tag)
            {
                
                if(caches[core][i][0] == 1)
                { 
                    caches[core][i][0] = 2;
                }
                else if(caches[core][i][0] == 3) //ONLY THE EXCLUSIVE CAN GO TO FORWARDER
                {
                   caches[core][i][0] = 6;
                }
                
            }
        }
    }
    else if(newCS == 1)
    {
        for(int i = 0; i < 4; i++)
        {
            if(caches[core][i][2] == tag)
            {
                if (caches[core][i][0] == 1 || caches[core][i][0] == 2)
                {
                    writebacks++;
                }
                
                //all other cores have to go to the I state
                caches[core][i][0] = 5;
                //LRU state stays the same
                caches[core][i][2] = 0; //Reset tag to 0
                caches[core][i][3] = 0; //dirty bit goes to 0
            }
        }
    }

}

bool checkInvalid(int cacheCS[4], int core)
{
    bool isI = true;
    for(int k = 0; k < 4; k++)
    {
        if(k != core) //check every core but the current core
        {
            if(cacheCS[k] != -1)
            {
                isI = false;
            }
        }
    }

    return isI;
}

void decreaseLRU(int caches[4][4][4], int core, int oldLRU, int line)
{
    for(int i = 0; i < 4; i++)
    {
        if(caches[core][i][1] > oldLRU && i != line)
        {
            caches[core][i][1]--;
        }

    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <filename>" << endl;
        return -1;
    }

    ifstream infile(argv[1]); // Open the file
    if (!infile.is_open()) {  // Check if file opened successfully
        cout << "Error opening file\n";
        return -1;
    }

    vector<string> instructions; // Vector to store instructions
    string line;
    int i = 0;

    while (getline(infile, line) && i < 4096) { // Read each line from the file
        if (line.empty()) continue; // Skip empty lines
        instructions.push_back(line); // Store instruction in vector
        i++; // Increment instruction count
    }

    infile.close(); // Close the file

    // DEBUG: Print the total number of instructions read
    //cout << "Total number of instructions: " << i << endl;

    // Initialize a single 3D array for 4 caches, each with 4 sets and 4 properties
    // Dimensions: [cache_index][set_index][property_index]
    int caches[4][4][4];

    // M->1, O->2, E->3, S->4, I->5, F->6
    for (int cache = 0; cache < 4; cache++) {     // Loop over each cache
        for (int set = 0; set < 4; set++) {       // Loop over each set
            caches[cache][set][0] = 5; // Coherency state = I (5)
            caches[cache][set][1] = set; // LRU state starts at whatever set it is
            caches[cache][set][2] = 0; // Tag = 0
            caches[cache][set][3] = 0; // Dirty bit = 0 (clean)
        }
    }

    // Process each instruction
    for (int j = 0; j < i; j++) {
        //DEBUG
        //cout << "Instruction " << j << ": " << instructions[j] << endl;

        string instruction = instructions[j];

        // Extract core (assuming "P<number>:")
        int core = instruction[1] - '0'; // Convert char to int
        core--;
        // Extract command (assuming "P<number>: <command>")
        size_t command_start = instruction.find(": ") + 2;
        size_t command_end = instruction.find(" <", command_start);
        string command = instruction.substr(command_start, command_end - command_start);

        // Extract tag (assuming "<number>")
        size_t tag_start = instruction.find('<') + 1;
        size_t tag_end = instruction.find('>', tag_start);
        int tag = stoi(instruction.substr(tag_start, tag_end - tag_start));

        // DEBUG Print extracted information
        //cout << "Instruction " << j << ": " << instructions[j] << endl;
        //cout << "  Core: " << core << endl;
        //cout << "  Command: " << command << endl;
        //cout << "  Tag: " << tag << endl;

        //NOW PARSE THE INSTRUCTION AND DO WHAT I NEEDED

        //First go through each cache and get what Coherency stage they are in MOESIF
        int cacheCS[4] = {-1, -1, -1, -1};

        //1. get the CS if the TAG is in the other cores
        for (int k = 0; k < 4; k++) 
        {
            if (caches[0][k][2] == tag) cacheCS[0] = caches[0][k][0];
            if (caches[1][k][2] == tag) cacheCS[1] = caches[1][k][0];
            if (caches[2][k][2] == tag) cacheCS[2] = caches[2][k][0];
            if (caches[3][k][2] == tag) cacheCS[3] = caches[3][k][0];
        }

        // 2. Check HIT/MISS
        bool hit = false;
        switch (core) {
            case 0: if (cacheCS[0] != -1) { hit = true; hits++; } break;
            case 1: if (cacheCS[1] != -1) { hit = true; hits++; } break;
            case 2: if (cacheCS[2] != -1) { hit = true; hits++; } break;
            case 3: if (cacheCS[3] != -1) { hit = true; hits++; } break;
            default: cout << "Invalid core specified: " << core << endl; break;
        }
        if(hit == false) misses++;

        //3. If it is a miss then we need to add the TAG line to the core we want
        int oldLRU = -1;
        if(cacheCS[core] == -1) //this means it is a miss
        {
            broadcasts++;
            //Debug
            //cout << "HERE IN MISS" << endl;
            //cout << "Core: " << core << endl;
            int line = -1;
            for(int j = 0; j < 4; j++) //find the first invalid line if it exists
            {
                if(caches[core][j][0] == 5)
                {
                    line = j;
                    break;
                }
            }
            if(line == -1) //this means that all the lines are not in the I state
            {
                for(int j = 0; j < 4; j++) //find the LRU cache line that is 0
                {
                    if(caches[core][j][1] == 0) line = j;
                    break;
                }
            }
            oldLRU = caches[core][line][1];
            //DEBUG
            //cout << line << endl;
            if(command == "read")
            {
                //if all other caches are invalid then go to E state
                if(checkInvalid(cacheCS, core))
                {
                    caches[core][line][0] = 3;
                    caches[core][line][1] = 3;
                    caches[core][line][2] = tag;
                    //caches[core][evict][3] Dirty bit doesn't need to be changed
                }
                //TO DO add logic for forwarding later when it comes up
                else //Core needs to go to shared state
                {
                    caches[core][line][0] = 4;
                    caches[core][line][1] = 3;
                    caches[core][line][2] = tag;
                    //caches[core][evict][3] Dirty bit doesn't need to be changed

                    //Now update all other caches to reflect this (So need to asign a forwarder)
                    //FORWARD HERE
                    cache_transfer++;
                    for(int k = 0; k < 4; k++)
                    {
                        if(k != core)
                        {
                            updateCore(caches, k, tag, cacheCS[k], caches[core][line][0]);
                        }
                    }
                }

            }
            else if(command == "write")
            {
                caches[core][line][0] = 1; //Go to M state
                caches[core][line][1] = 3;
                caches[core][line][2] = tag;
                caches[core][line][3] = 1; //Line is now dirty

                for(int k = 0; k < 4; k++)
                {
                    if(k != core)
                    {
                        updateCore(caches, k, tag, cacheCS[k], caches[core][line][0]);
                    }
                }
            }
            //Decrease the LRU values for all lines that weren't updated
            decreaseLRU(caches, core, oldLRU, line);
        }
        //4. If it wasn't a miss then we need to update the line
        else
        {
            int line = -1;
            for(int k = 0; k < 4; k++)
            {
                if(caches[core][k][2] == tag)
                {
                    line = k;
                    break;
                }
            }
            oldLRU = caches[core][line][1];

            if(caches[core][line][0] != 3) broadcasts++;
            if(command == "read")
            {
                //if it is a hit but we are in the M or O state then we braodcast
                //if(caches[core][line][0] == 1 || caches[core][line][0] == 2 || caches[core][line][0] == 4) broadcasts++;
                //Stay in the same Coherency level
                caches[core][line][1] = 3;
                //Tag stays the same
                //dirty bit stays the same

                //NO BROADCAST
            }
            else if(command == "write")
            {
                //broadcasts++;
                if(caches[core][line][0] == 3) //this means we are writing to an exclusive state so no need to broadcast or update the rest
                {
                    caches[core][line][0] = 1; //set to modified
                    caches[core][line][1] = 3; //LRU to 3
                    //Cache tag doesn't change
                    caches[core][line][3] = 1; //Line is now dirty since it will need to be written back

                    //NO BROADCAST
                    //Dont braodcast when we go from E-M or when we read from a M or O state
                }
                //TO DO what do you do if you write to a modified or the owner
                else //Writing to a shared 
                {
                    caches[core][line][0] = 1; //Go to M state
                    caches[core][line][1] = 3;
                    //caches[core][line][2] = tag; TAG stays the same
                    caches[core][line][3] = 1; //Line is now dirty

                    for(int k = 0; k < 4; k++)
                    {
                        if(k != core)
                        {
                            updateCore(caches, k, tag, cacheCS[k], caches[core][line][0]);
                        }
                    }
                }
            }
            //Decrease the LRU values for all lines that weren't updated
            decreaseLRU(caches, core, oldLRU, line);
        }
    }

    // Function to print a cache from the 3D array
    auto printCache = [](const int caches[4][4][4], const string& cacheName) {
        cout << "\n" << cacheName << ":\n";
        for (int cache = 0; cache < 4; ++cache) { // Loop over caches
            cout << "  Cache " << cache + 1 << ":\n"; // Display cache number (1-based)
            for (int line = 0; line < 4; ++line) { // Loop over cache lines (sets)
                cout << "    Line " << line
                    << ": Coherency=" << caches[cache][line][0]
                    << ", LRU=" << caches[cache][line][1]
                    << ", Tag=" << caches[cache][line][2]
                    << ", Dirty=" << caches[cache][line][3] << '\n';
            }
        }
    };

    // Print all caches
    //printCache(caches, "All Caches");

    //ANSWERS
    cout << hits << endl;
    cout << misses << endl;
    cout << writebacks << endl;
    cout << broadcasts << endl;
    cout << cache_transfer;
    return 0;
}
