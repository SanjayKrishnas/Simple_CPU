class my_update : public branch_update {
public:
    unsigned int index = 0;   // Index for the first predictor table
    unsigned int index2 = 0;  // Index for the second predictor table
    unsigned int index3 = 0;  // Index for the third predictor table
    unsigned int index4 = 0;  // Index for the fourth predictor table
    unsigned int index5 = 0;  // Index for the fifth predictor table
    unsigned int index6 = 0;  // Index for the sixth predictor table
    unsigned int index7 = 0;  // Index for the seventh predictor table
    unsigned int index8 = 0;  // Index for the eighth predictor table
    unsigned int index9 = 0;  // Index for the ninth predictor table
    unsigned int index10 = 0; // Index for the tenth predictor table
    unsigned int index11 = 0; // Index for the eleventh predictor table
    unsigned int pc = 0;      // Store PC for better indexing
};

class my_predictor : public branch_predictor {
public:
    // Keep the same history lengths matching table sizes
    #define HISTORY_LENGTH 22
    #define HISTORY2_LENGTH 20
    #define HISTORY3_LENGTH 18
    #define HISTORY4_LENGTH 16
    #define HISTORY5_LENGTH 14
    #define HISTORY6_LENGTH 12
    #define HISTORY7_LENGTH 10
    #define HISTORY8_LENGTH 8
    #define HISTORY9_LENGTH 6
    #define HISTORY10_LENGTH 4
    #define HISTORY11_LENGTH 2  // New history length for the eleventh table

    #define TABLE_BITS 22
    #define TABLE2_BITS 20
    #define TABLE3_BITS 18
    #define TABLE4_BITS 16
    #define TABLE5_BITS 14
    #define TABLE6_BITS 12
    #define TABLE7_BITS 10
    #define TABLE8_BITS 8
    #define TABLE9_BITS 6
    #define TABLE10_BITS 4
    #define TABLE11_BITS 2  // New table bits for the eleventh table

    my_update u;
    branch_info bi;
    unsigned int history = 0;
    unsigned int history2 = 0;
    unsigned int history3 = 0;
    unsigned int history4 = 0;
    unsigned int history5 = 0;
    unsigned int history6 = 0;
    unsigned int history7 = 0;
    unsigned int history8 = 0;
    unsigned int history9 = 0;
    unsigned int history10 = 0;
    unsigned int history11 = 0;  // New history for the eleventh table
    
    unsigned int last_pc = 0;

    unsigned char tab[1 << TABLE_BITS];
    unsigned char tab2[1 << TABLE2_BITS];
    unsigned char tab3[1 << TABLE3_BITS];
    unsigned char tab4[1 << TABLE4_BITS];
    unsigned char tab5[1 << TABLE5_BITS];
    unsigned char tab6[1 << TABLE6_BITS];
    unsigned char tab7[1 << TABLE7_BITS];
    unsigned char tab8[1 << TABLE8_BITS];
    unsigned char tab9[1 << TABLE9_BITS];
    unsigned char tab10[1 << TABLE10_BITS];
    unsigned char tab11[1 << TABLE11_BITS];  // New table for the eleventh predictor

    // Helper function for better index mixing
    unsigned int hash_index(unsigned int pc, unsigned int history, unsigned int bits) {
        unsigned int index = pc ^ (pc >> 2);
        index = index ^ (history << (bits / 2));
        index = index ^ (history >> (bits / 3));
        return index & ((1 << bits) - 1);
    }

    my_predictor() {
        // Initialize to weakly taken for better startup behavior
        memset(tab, 2, sizeof(tab));
        memset(tab2, 2, sizeof(tab2));
        memset(tab3, 2, sizeof(tab3));
        memset(tab4, 2, sizeof(tab4));
        memset(tab5, 2, sizeof(tab5));
        memset(tab6, 2, sizeof(tab6));
        memset(tab7, 2, sizeof(tab7));
        memset(tab8, 2, sizeof(tab8));
        memset(tab9, 2, sizeof(tab9));
        memset(tab10, 2, sizeof(tab10));
        memset(tab11, 2, sizeof(tab11));  // Initialize the new table
    }

    branch_update* predict(branch_info& b) {
        bi = b;
        if (b.br_flags & BR_CONDITIONAL) {
            u.pc = b.address;

            // Better index calculation using both PC and history
            u.index = hash_index(b.address, history, TABLE_BITS);
            u.index2 = hash_index(b.address, history2, TABLE2_BITS);
            u.index3 = hash_index(b.address, history3, TABLE3_BITS);
            u.index4 = hash_index(b.address, history4, TABLE4_BITS);
            u.index5 = hash_index(b.address, history5, TABLE5_BITS);
            u.index6 = hash_index(b.address, history6, TABLE6_BITS);
            u.index7 = hash_index(b.address, history7, TABLE7_BITS);
            u.index8 = hash_index(b.address, history8, TABLE8_BITS);
            u.index9 = hash_index(b.address, history9, TABLE9_BITS);
            u.index10 = hash_index(b.address, history10, TABLE10_BITS);
            u.index11 = hash_index(b.address, history11, TABLE11_BITS);  // New index calculation for the eleventh table

            // Get counter values
            unsigned char cnt1 = tab[u.index];
            unsigned char cnt2 = tab2[u.index2];
            unsigned char cnt3 = tab3[u.index3];
            unsigned char cnt4 = tab4[u.index4];
            unsigned char cnt5 = tab5[u.index5];
            unsigned char cnt6 = tab6[u.index6];
            unsigned char cnt7 = tab7[u.index7];
            unsigned char cnt8 = tab8[u.index8];
            unsigned char cnt9 = tab9[u.index9];
            unsigned char cnt10 = tab10[u.index10];
            unsigned char cnt11 = tab11[u.index11];  // Get counter for the eleventh table

            // Adaptive weights based on counter confidence
            int weight1 = (cnt1 == 0 || cnt1 == 3) ? 7 : 5;
            int weight2 = (cnt2 == 0 || cnt2 == 3) ? 6 : 4;
            int weight3 = (cnt3 == 0 || cnt3 == 3) ? 5 : 3;
            int weight4 = (cnt4 == 0 || cnt4 == 3) ? 4 : 2;
            int weight5 = (cnt5 == 0 || cnt5 == 3) ? 3 : 1;
            int weight6 = (cnt6 == 0 || cnt6 == 3) ? 3 : 1;
            int weight7 = (cnt7 == 0 || cnt7 == 3) ? 2 : 1;
            int weight8 = (cnt8 == 0 || cnt8 == 3) ? 2 : 1;
            int weight9 = (cnt9 == 0 || cnt9 == 3) ? 1 : 1;
            int weight10 = (cnt10 == 0 || cnt10 == 3) ? 1 : 1;
            int weight11 = (cnt11 == 0 || cnt11 == 3) ? 1 : 1;  // New weight for the eleventh table

            // Enhanced voting system
            int weighted_vote = 0;
            
            // Add votes based on counter values
            weighted_vote += (cnt1 >= 2) ? weight1 : -weight1;
            weighted_vote += (cnt2 >= 2) ? weight2 : -weight2;
            weighted_vote += (cnt3 >= 2) ? weight3 : -weight3;
            weighted_vote += (cnt4 >= 2) ? weight4 : -weight4;
            weighted_vote += (cnt5 >= 2) ? weight5 : -weight5;
            weighted_vote += (cnt6 >= 2) ? weight6 : -weight6;
            weighted_vote += (cnt7 >= 2) ? weight7 : -weight7;
            weighted_vote += (cnt8 >= 2) ? weight8 : -weight8;
            weighted_vote += (cnt9 >= 2) ? weight9 : -weight9;
            weighted_vote += (cnt10 >= 2) ? weight10 : -weight10;
            weighted_vote += (cnt11 >= 2) ? weight11 : -weight11;  // Add vote for the eleventh table

            // Strong bias to maintain prediction if highly confident
            if (last_pc == b.address) {
                if (cnt1 == 3 || cnt1 == 0) {
                    weighted_vote += (cnt1 >= 2) ? 2 : -2;
                }
            }

            bool final_pred = (weighted_vote >= 0);
            u.direction_prediction(final_pred);
        } else {
            u.direction_prediction(true);
        }
        u.target_prediction(0);
        last_pc = b.address;
        return &u;
    }

    void update(branch_update* u, bool taken, unsigned int target) {
        if (bi.br_flags & BR_CONDITIONAL) {
            // Update the first table's entry
            unsigned char* f = &tab[((my_update*)u)->index];
            if (taken) {
                if (*f < 3) (*f)++;
            } else {
                if (*f > 0) (*f)--;
            }

            // Update the second table's entry
            unsigned char* c = &tab2[((my_update*)u)->index2];
            if (taken) {
                if (*c < 3) (*c)++;
            } else {
                if (*c > 0) (*c)--;
            }

            // Update the third table's entry
            unsigned char* d = &tab3[((my_update*)u)->index3];
            if (taken) {
                if (*d < 3) (*d)++;
            } else {
                if (*d > 0) (*d)--;
            }

            // Update the fourth table's entry
            unsigned char* e = &tab4[((my_update*)u)->index4];
            if (taken) {
                if (*e < 3) (*e)++;
            } else {
                if (*e > 0) (*e)--;
            }

            // Update the fifth table's entry
            unsigned char* g = &tab5[((my_update*)u)->index5];
            if (taken) {
                if (*g < 3) (*g)++;
            } else {
                if (*g > 0) (*g)--;
            }

            // Update the sixth table's entry
            unsigned char* h = &tab6[((my_update*)u)->index6];
            if (taken) {
                if (*h < 3) (*h)++;
            } else {
                if (*h > 0) (*h)--;
            }

            // Update the seventh table's entry
            unsigned char* i = &tab7[((my_update*)u)->index7];
            if (taken) {
                if (*i < 3) (*i)++;
            } else {
                if (*i > 0) (*i)--;
            }

            // Update the eighth table's entry
            unsigned char* j = &tab8[((my_update*)u)->index8];
            if (taken) {
                if (*j < 3) (*j)++;
            } else {
                if (*j > 0) (*j)--;
            }

            // Update the ninth table's entry
            unsigned char* k = &tab9[((my_update*)u)->index9];
            if (taken) {
                if (*k < 3) (*k)++;
            } else {
                if (*k > 0) (*k)--;
            }

            // Update the tenth table's entry
            unsigned char* l = &tab10[((my_update*)u)->index10];
            if (taken) {
                if (*l < 3) (*l)++;
            } else {
                if (*l > 0) (*l)--;
            }

            // Update the eleventh table's entry
            unsigned char* m = &tab11[((my_update*)u)->index11];
            if (taken) {
                if (*m < 3) (*m)++;
            } else {
                if (*m > 0) (*m)--;
            }

            // Shift histories and add the outcome
            history = ((history << 1) | taken) & ((1 << HISTORY_LENGTH) - 1);
            history2 = ((history2 << 1) | taken) & ((1 << HISTORY2_LENGTH) - 1);
            history3 = ((history3 << 1) | taken) & ((1 << HISTORY3_LENGTH) - 1);
            history4 = ((history4 << 1) | taken) & ((1 << HISTORY4_LENGTH) - 1);
            history5 = ((history5 << 1) | taken) & ((1 << HISTORY5_LENGTH) - 1);
            history6 = ((history6 << 1) | taken) & ((1 << HISTORY6_LENGTH) - 1);
            history7 = ((history7 << 1) | taken) & ((1 << HISTORY7_LENGTH) - 1);
            history8 = ((history8 << 1) | taken) & ((1 << HISTORY8_LENGTH) - 1);
            history9 = ((history9 << 1) | taken) & ((1 << HISTORY9_LENGTH) - 1);
            history10 = ((history10 << 1) | taken) & ((1 << HISTORY10_LENGTH) - 1);
            history11 = ((history11 << 1) | taken) & ((1 << HISTORY11_LENGTH) - 1);  // Shift history for the eleventh table
        }
    }
};
