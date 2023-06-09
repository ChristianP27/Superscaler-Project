#include <iomanip>
#include <unordered_map>
#include <bitset>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <iomanip>
using namespace std;

struct instruction{
	int v,rs,rd,rt,shamt, func, opcode, imm, target, intVal, addr, jTarget,
	    dest=-1, src1=-1, src2=-1;
	string out, bitstr, istr;
	bool isBreak;
	instruction(){}
	instruction( int i, int addr, bool doneBreak): isBreak( false )
	{ 	
		unsigned int ui = (unsigned int)  i;
		this->addr = addr;
		intVal = i;
		v =  ui >> 31;
		opcode = ui >> 26;
		rs = ui << 6 >> 27;
		rt = ui <<11 >> 27;
		rd = ui << 16 >> 27;
		imm = i << 16 >> 16;
		target = (imm << 2);
		jTarget = ui <<6 >> 4;
		shamt = ui << 21 >> 27;
		func = ui <<26 >> 26;
		if( doneBreak ){
			stringstream ss;
			ss << "\t"<<addr << "\t" << i; 
			out = bitset<32>( i ).to_string() + ss.str();
		} else {
			bitstr = bitset<32>(i).to_string();
			bitstr = bitstr.substr(0,1) + " " + bitstr.substr(1,5) + " " + bitstr.substr(6,5) + " "
				+ bitstr.substr(11, 5 ) + " " + bitstr.substr(16, 5) + " " 
				+ bitstr.substr( 21, 5 ) + " " + bitstr.substr(26, 6);
			{
				stringstream ss;
				ss << addr;
				out = bitstr + "\t" + ss.str() + "\t";
			}

			if( v == 0 ){
				out = out + "Invalid Instruction";
				istr = "Invalid Instruction";
			}
			else if( ui == 2147483648 ){
                                stringstream ss;
                                ss << "NOP";
				out = out + ss.str();
				istr = ss.str();
                        }
			else if( opcode == 40 ){
				stringstream ss;
				ss << "ADDI\tR" << rt << ", R" << rs << ", #" << imm; 
				out = out + ss.str();
				istr = ss.str();
				dest = rt;
				src1 = rs;
				src2 = rs;
			}
			else if( opcode == 43 ){
				stringstream ss;
				ss << "SW\tR" << rt << ", " <<imm << "(R" << rs << ")"; 
				out = out + ss.str();
				istr = ss.str();
				
				src1 = rs;
				src2 = rt;
			}
			else if( opcode == 35 ){
				stringstream ss;
				ss << "LW\tR" << rt << ", " <<imm << "(R" << rs << ")"; 
				out = out + ss.str();
				istr = ss.str();
				dest = rt;
				src1 = rs;
				src2 = rs;
			}
			else if( opcode == 33 ){
				stringstream ss;
				ss << "BLTZ\tR" << rs << ", #" <<target; 
				out = out + ss.str();
				istr = ss.str();
				//dest = rt;
				src1 = rs;
				src2 = rs;
			}
			else if( opcode == 32 && func == 0 ){
				stringstream ss;
				ss << "SLL\tR" << rd << ", R" << rt << ", #" << shamt; 
				out = out + ss.str();
				istr = ss.str();
			}
			else if( opcode == 32 && func == 2 ){
				stringstream ss;
				ss << "SRL\tR" << rd << ", R" << rt << ", #" << shamt; 
				out = out + ss.str();
				istr = ss.str();
			}
			else if( opcode == 32 && func == 34 ){
				stringstream ss;
				ss << "SUB\tR" << rd << ", R" << rs << ", R" << rt; 
				out = out + ss.str();
				istr = ss.str();
			}
			else if( opcode == 32 && func == 32 ){
				stringstream ss;
				ss << "ADD\tR" << rd << ", R" << rs << ", R" << rt; 
				out = out + ss.str();
				istr = ss.str();
			}
			else if( opcode == 32 && func == 10 ){
				stringstream ss;
				ss << "MOVZ\tR" << rd << ", R" << rs << ", R" << rt; 
				out = out + ss.str();
				istr = ss.str();
			}
			else if( opcode == 60 && func == 2 ){
				stringstream ss;
				ss << "MUL\tR" << rd << ", R" << rs << ", R" << rt; 
				out = out + ss.str();
				istr = ss.str();
			}
			else if( opcode == 32 && func == 8 ){
				stringstream ss;
				ss << "JR\tR" << rs; 
				out = out + ss.str();
				istr = ss.str();
			}
			else if( opcode == 34){
				stringstream ss;
				ss << "J\t#" << jTarget; 
				out = out + ss.str();
				istr = ss.str();

			}
			else if( opcode == 32 && func == 13 ){
				stringstream ss;
				ss << "BREAK"; 
				istr = ss.str();
				out = out + ss.str();
				isBreak = true;
			}
			else {
				printf ("Opcode %i func %i not implemented\n", opcode, func);
				print();
				exit(0);
			}
		}

		
	}

	void print(){
		printf( "%s\n%s v %i; rs %i, rt %i, rd %i, shamt %i, func %i, opcode %i, func %i, imm %i, target %i, intVal %i, addr %i\n",
				out.c_str(),bitstr.c_str(), v, rs, rt, rd, shamt, func, opcode, func,imm, target, intVal, addr);
	}
};

string printState( const int R[], const int PC, const int cycle, unordered_map< int, instruction> & MEM,
	      const int breakAddr, const int lastAddr ){
	std::ios oldState(nullptr);
	stringstream ss1;
	oldState.copyfmt(ss1);	
	instruction I = MEM[PC];
	ss1<< "====================\ncycle:"<<cycle<<" "<<PC<<"\t"<< I.istr<<"\n\nregisters:";
	for( int i = 0; i < 32; i++ ) {
		if(i%8 == 0){
			ss1 << "\nr"<<std::setfill('0') << std::setw(2)<< i;
			std::cout.copyfmt( oldState );
		}
		ss1 << "\t" <<R[i];
	}
	ss1 << "\n\ndata:";
	for( int i = breakAddr +4; i < lastAddr; i+=4 ) {
		if( ((i-breakAddr -4)/4) % 8 == 0 )
			ss1<< "\n"<< i <<":";
		ss1<< "\t" <<  MEM[i].intVal;
	}
	ss1<<"\n\n";
	return ss1.str();

}	

int main( )
{
	unordered_map< int, instruction> MEM;
	bool doneBreak = false;
	int breakAddr = 0;
	int lastAddr = 0;
        char buffer[4];
        int i;
        char * iPtr;
        iPtr = (char*)(void*) &i;
	int addr = 96;
        int FD = open("t2.bin" , O_RDONLY);
        // printf( "filename: %s", argv[2]);
	int amt = 4;
        while( amt != 0 )
        {
                amt = read(FD, buffer, 4);
                if( amt == 4)
                {
                        iPtr[0] = buffer[3];
                        iPtr[1] = buffer[2];
                        iPtr[2] = buffer[1];
                        iPtr[3] = buffer[0];
                        //cout << "i = " <<hex<< i << endl;
                }
        	instruction I( i, addr, doneBreak );
		MEM[addr] = I;
		if( I.isBreak ){
		       	doneBreak = true;
			breakAddr = addr;
		}
		addr +=4;
	}
	lastAddr = addr-4;

	// for( int i = 96; i < lastAddr; i+=4 )
	// 	cout << MEM[i].out << endl;

	// make a register file and processor state elements
	struct processorState{
		int R[32] ={0};
		int PC = 96;
		int cycle = 1;
		unordered_map< int, instruction> MEM;
		int preIssue[4] = {0};
		int preALU[2] = {0};
		int preMEM[2] = {0};
		struct postThings{
			int instr=0, value=0;
		};
		postThings postALU, postMEM;
		
		bool XBWcheck( int reg, int issuePos ){
			if( reg <0 ) return false;
			for( int i = issuePos-1; i >=0; i-- ){
				if( reg == MEM[preIssue[i]].dest ) return true;
			}
			for( int i = 0; i < 2; i++ ){
				if( reg == MEM[preALU[i]].dest ) return true;
				if( reg == MEM[preMEM[i]].dest ) return true;
			}
			if( reg == MEM[postALU.instr].dest ) return true;
			if( reg == MEM[postMEM.instr].dest ) return true;
			return false;
		}
		// void WB(){
		// 	if( postMEM.instr !=0 ) {
		// 		R[ MEM[postMEM[instr]].dest ] = postMEM.value;
		// 		postMEM = postThings();
		// 	}
		// 	if( postALU.instr !=0 ) {
		// 		R[ MEM[postALU[instr]].dest ] = postALU.value;
		// 		postALU = postThings();
		// 	}
		// }

void IF(){
            int add_holder = PC;
            int add_holder1 = PC + 4;
            int temp_counter;
            int L = 0;
            int sec_Counter = 0;

            for (int i = 0; i < 4 ; i++ ) { // checks the contents in the array counts how many NON-zeros
                    if(preIssue[i] != 0){
                        temp_counter ++;
                        continue;
                    
                    }
            }
                    if (temp_counter == 4){ // array is full do nothing 
                        nullptr;
                    } else {

                    // which arrray slot has a zero
                    for ( int i = 0; i < 4; i++ ){ // places the first address into the array slot
                        if( (preIssue[i] == 0) && (L != 1)){
                            preIssue[i] = add_holder;
                            add_holder = 0;
                            L ++;
                        }
                        else if ( (preIssue[i] == 0) && ( sec_Counter != 1 )){ // do the second instruction
                            preIssue[i] = add_holder1;
                            sec_Counter ++;
                            add_holder1 = 0; 

                        }
                    }
                    }
                    


                    // else if ( temp_counter == 3 ){
                    //     preIssue[3] = ;
                    // }
                    // else if ( temp_counter = 2 ){
                    //     preIssue[2] = PC;
                    // }
                    // else if ( temp_counter = 1 ){
                    //     preIssue[1] = PC;
                    // }
            // preIssue[i] = PC;
        
}

void ISSUE(){ // move the instructions to the given areas needed / Job divider
        int temp_counter = 0;
        int counter = 0;

        // temp storage for the values
        

            for (int i = 0; i < 4 ; i++ ) { // checks the contents in the array is it empty is the job list empty
                    if(preIssue[i] != 0){
                        temp_counter ++;
                        continue;
                    }
            }

            // test to see if empty
            if ( temp_counter != 0 ) { // not empty now do the given job
                if (((MEM[preIssue[1]].rs == MEM[preIssue[0]].rt) || (MEM[preIssue[1]].imm == MEM[preIssue[0]].rt)) || 
				((MEM[preIssue[0]].opcode == 40) && ((MEM[preIssue[0]].rs == MEM[preALU[0]].rt) || (MEM[preIssue[0]].rs == MEM[preMEM[0]].rt))) || 
				((MEM[preIssue[0]].opcode == 43) && ((MEM[preIssue[0]].rt == MEM[preALU[0]].rt) || (MEM[preIssue[0]].rt == MEM[preMEM[0]].rt))) ||
				((MEM[preIssue[0]].opcode == 32) && ((MEM[preIssue[0]].rs == MEM[preALU[0]].rt) || (MEM[preIssue[0]].rs == MEM[preMEM[0]].rt))) ){ // can i move both items or just one is there a dependency???

                    if ( (MEM[preIssue[0]].opcode == 35) || (MEM[preIssue[0]].opcode == 43) ) { // check if LW or SW
                        preMEM[0] = preIssue[0]; // moves the one element
                        preIssue[0] = preIssue[1];
                        preIssue[1] = 0;
                    } else if (MEM[preIssue[0]].opcode == 40 || MEM[preIssue[0]].opcode == 40) {
						preALU[0] = preIssue[0];
						preIssue[0] = preIssue[1];
						preIssue[1] = 0;
					}
                } else { // able to move both
                     if ( (MEM[preIssue[0]].opcode == 35) || (MEM[preIssue[0]].opcode == 43) || (MEM[preIssue[1]].opcode == 35) || (MEM[preIssue[1]].opcode == 43)) { // check if LW or SW
                        preMEM[0] = preIssue[0];
                        preMEM[1] = preIssue[1];
                        preIssue[0] = 0;
                        preIssue[1] = 0;
                    } else if ((MEM[preIssue[0]].opcode == 40) || (MEM[preIssue[0]].opcode == 32) || (MEM[preIssue[1]].opcode == 40) || (MEM[preIssue[1]].opcode == 32)) {
						preALU[0] = preIssue[0];
						preIssue[0] = preIssue[1];
						preIssue[1] = 0;
                	}
            	} 
			
			} else { // empty dont do anything
            	nullptr; 
		}
}

        // void MEM(){

        // }

        void ALU(){

        }
	};
	processorState state;
	state.MEM = MEM;
    int counter = 0;
	while( counter != 3 ){
		
		// state.WB();
		// state.ALU();
		// state.MEM();
		
		state.IF();

        // printing the data

        // Pre-Issue Section
        cout << "--------------------\nCycle: " << state.cycle << endl;
        cout << endl << "Pre-Issue Buffer:\n";
        for ( int i = 0; i < 4; i ++){
            cout << "\tEntry " << i << ": ";
            if ( state.preIssue[i] == 0 ) {
                nullptr;
            }
            else {
                cout << "\t[" << state.MEM[state.preIssue[i]].istr << "]";
            }
            cout << endl;
        }

        // Pre_ALU
        cout << "Pre_ALU Queue:" << endl;
        for ( int i = 0; i < 2; i++ ){
            cout << "\tEntry " << i << ": ";
            if ( state.preALU[i] == 0 ){
                nullptr;
            } else {
                cout << "\t[" << state.MEM[state.preALU[i]].istr << "]";
            }
            cout << endl;
        }


        // Post_ALU

        cout << "Post_ALU Queue:" << endl;
        for ( int i = 0; i < 1; i++ ){
            cout << "\tEntry " << i << ": ";
            if ( state.postALU.value != 0 ){
                cout << "\t[" << state.MEM[state.postALU.value].istr << "]";
            }
            cout << endl;
        }


        // Pre_MEM
        cout << "Pre_MEM Queue:" << endl;
        for ( int i = 0; i < 2; i++ ){
            cout << "\tEntry " << i << ": ";
            if ( state.preMEM[i] == 0 ){
                nullptr;
            } else {
                cout << "\t[" << state.MEM[state.preMEM[i]].istr << "]";
            }
            cout << endl;
        }


        // Post MEM

        cout << "Post_MEM Queue:" << endl;
        for ( int i = 0; i < 1; i++ ){
            cout << "\tEntry " << i << ": ";
            if ( state.postMEM.value == 0 ){
                nullptr;
            } else {
                cout << "\t[" << state.MEM[state.postMEM.value].istr << "]";
            }
            cout << endl;
        }




        // END OF PRINT DATA//////////

        state.ISSUE(); // moves the data to needed areas
        // state.MEM(); // activates the memory access and grabs/ stores the data

    counter ++; // temp counter to stp the whille loop
    state.PC += 8;
	state.cycle++;












		/*
		instruction I = MEM[ PC ];
		while( I.v == 0 ){
			PC += 4;
			I = MEM[ PC ];
		}
		PC += 4;
		nextpc = PC;
		if( (unsigned int) I.intVal == 2147483648 ){ //NOP
		}
		else if( I.opcode == 40 ){ //ADDI
			R[ I.rt ] = R[ I.rs] + I.imm;
		}
		else if( I.opcode == 43 ){ //SW
			//ss << "SW\tR" << rt << ", " <<imm << "(R" << rs << ")"; 
			MEM[ R[I.rs] + I.imm ].intVal = R[ I.rt ];
		}
		else if( I.opcode == 35 ){
			//ss << "LW\tR" << rt << ", " <<imm << "(R" << rs << ")"; 
			R[I.rt] = MEM[ I.imm + R[I.rs]].intVal;
		}
		else if( I.opcode == 33 ){
			//ss << "BLTZ\tR" << rs << ", #" <<target;
			if (R[I.rs] < 0	)
				nextpc = PC + I.target;
		}
		else if( I.opcode == 32 && I.func == 0 ){
			//ss << "SLL\tR" << rd << ", R" << rt << ", #" << shamt; 
			R[I.rd] = R[I.rt] << I.shamt;
		}
		else if( I.opcode == 32 && I.func == 2 ){
			//ss << "SRL\tR" << rd << ", R" << rt << ", #" << shamt; 
		}
		else if( I.opcode == 32 && I.func == 34 ){
			//1ss << "SUB\tR" << rd << ", R" << rs << ", R" << rt; 
			R[I.rd] = R[I.rs] - R[I.rt];
		}
		else if( I.opcode == 32 && I.func == 32 ){
			//ss << "ADD\tR" << rd << ", R" << rs << ", R" << rt; 
			R[I.rd] = R[I.rs] + R[I.rt];
		}
		else if( I.opcode == 32 && I.func == 10 ){
			//ss << "MOVZ\tR" << rd << ", R" << rs << ", R" << rt; 
		}
		else if( I.opcode == 60 && I.func == 2 ){
			//ss << "MUL\tR" << rd << ", R" << rs << ", R" << rt; 
		}
		else if( I.opcode == 32 && I.func == 8 ){
			//ss << "JR\tR" << rs; 
		}
		else if( I.opcode == 34){
			//ss << "J\t#" << jTarget; 
			nextpc = I.jTarget;

		}
		else if( I.opcode == 32 && I.func == 13 ){
			//ss << "BREAK"; 
		//	isBreak = true;
		}
		else {
			printf ("Opcode %i func %i not implemented\n", I.opcode, I.func);
			I.print();
			exit(0);
		}

		cout << printState(  R, PC-4,  cycle, MEM,  breakAddr,  lastAddr ) << endl;
		cycle ++;
		if (PC == breakAddr + 4) break;
		PC = nextpc;
		//if( cycle >= 145) break;
		*/
	}

}