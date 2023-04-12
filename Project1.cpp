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

int main( ) // add arguments later
{
	int counter = 0;
	unordered_map< int, instruction> MEM;
	bool doneBreak = false;
	int breakAddr = 0;
	int lastAddr = 0;
        char buffer[4];
        int i;
        char * iPtr;
        iPtr = (char*)(void*) &i;
	int addr = 96;
        int FD = open("t1.bin" , O_RDONLY);
        printf( "filename: %s", "t1.bin\n");
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
	lastAddr = addr-8;

	for( int i = 96; i < lastAddr+4; i+=4 )
		cout << MEM[i].out << endl;

	// make a register file and processor state elements
	struct processorState{
		int R[32] ={0};
		int PC = 96;
		int cycle = 1;
		unordered_map< int, instruction> MEM;
		// unordered_map<int, preALU> instr;
        int preIssue[4] = {0};
		int preALU[2] = {0};
		int preMEM[2] = {0};
		// int preIssue[4] = {0};
		// int preALU[2] = {0};
		// int preMEM[2] = {0};
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
		void WB(){
			if( postMEM.instr !=0 ) {
				R[ MEM[postMEM.instr].dest ] = postMEM.value;
				postMEM = postThings();
			}
			if( postALU.instr !=0 ) {
				R[ MEM[postALU.instr].dest ] = postALU.value;
				postALU = postThings();
			}
		}

        void IF(){
			int temp_counter;
			int L =1;
            // grabs the first two instructions if there is nothing in the queue also 
			// check to see if queue is full.
			// if(cycle_counter == 1){

			// }
			for(int i = 0; i < sizeof(preIssue); i++){
				if(preIssue[i] != 0){
					temp_counter ++;
					continue;
				}
				if((cycle == 1) && (preIssue[i] == 0) && (L<=2)){
					preIssue[i] = PC;
					PC += 4;
					L ++;
				}

		// 	}
			if( temp_counter == 4){
				nullptr;
			}
		// 	else if ( temp_counter == 3){
		// 	preIssue[3] = 1; 	
		// 	}
		// 	else if ( temp_counter == 2){
		// 	preIssue[2] = 1; 
		// 	}
		// 	else if ( temp_counter == 1 ){
		// 	preIssue[1] = 1; 
		// 	}
		// 	else if ( temp_counter == 0){
		// 	preIssue[0] = 1;
        }
		}
	};
	processorState state;
	state.MEM = MEM;
	int cycle_counter = state.cycle;
	while( counter != 3){ // state.PC != lastAddr
		 cycle_counter = state.cycle;
		 cout << "--------------------\nCycle: " << cycle_counter << endl;
		// state.WB();
	// 	// state.ALU();
	// 	// state.MEM();
	// 	// state.ISSUE();
		state.IF();




		// printing the steps
		stringstream ssl;
		cout << endl << "Pre-Issue Buffer:\n";
		// for(int r = state.PC; r <= state.PC + 4; r+=4 ){
			for ( int r = 0; r <= 4; r++){
			cout << "\tEntry " << r << ": ";
			if((state.preIssue[r] != 0)) {
				cout << "\t[" << state.MEM[state.preIssue[r]].istr << "]";
				// if (state.MEM[state.preIssue[r]].rt == state.MEM[state.preIssue[r+1]].rs){
				// 	for(int m = 0,  i=0; m <= sizeof(state.preMEM); m++){
				// 		if((state.preMEM[m] == 0) && (i<2) ){
				// 			state.preMEM[m] = state.preIssue[r];
				// 			state.preIssue[r] = state.preIssue[r+1];
				// 			state.preIssue[r+1] = 0;
				// 	}
				// 	}
				// }
	
			}
			
			cout <<  ssl.str() << endl;
		// }
			ssl.clear();
		}
		cout << "Pre_ALU Queue:" << endl;
		for( int r = 0; r <= 1; r++){
			cout << "\tEntry " << r << ": ";
			if (state.preALU[r] == 1){
				// for later stage
			}
			cout << ssl.str() << endl;
			ssl.clear();
		}
		cout << "Post_ALU Queue:" << endl;
		for( int r = 0; r <= 0; r++){
			cout << "\tEntry " << r << ": ";
			if (state.preALU[r] == 1){
				// for later stage
			}
			cout << ssl.str() << endl;
			ssl.clear();
		}
		cout << "Pre_MEM Queue:" << endl;
		for( int r = 0; r <= 1; r++){
			cout << "\tEntry " << r << ": ";
			if (state.preALU[r] == 1){
				// for later stage
			}
			cout << ssl.str() << endl;
			ssl.clear();
		}
		cout << "Post_MEM Queue:" << endl;
		for( int r = 0; r <= 0; r++){
			cout << "\tEntry " << r << ": ";
			if (state.preALU[r] == 1){
				// for later stage
			}
			cout << ssl.str() << endl;
			ssl.clear();
		}
		
		// displays the register
        ssl << "\nregisters:";
        for( int i = 0; i < 32; i++ ){
            if( i %8 == 0 )
                ssl << "\nr"<<i<<":  ";
            ssl << "\t" << state.R[i];
            }
	    cout  << ssl.str() << endl;
		ssl.clear();

		//displays the data
		cout << "\nData";
		for ( int i = breakAddr + 4, r = 0; r < (lastAddr-breakAddr)/4; i+=4, r++){
			if( r % 8 == 0){
				cout << endl << i << ":\t";
			}
			cout << MEM[i].intVal << "\t";
		}
		cout << endl;
		for ( int r = 0; r <= 4; r++){
		if (state.MEM[state.preIssue[r]].rt == state.MEM[state.preIssue[r+1]].rs){
			for(int m = 0,  i=0; m <= sizeof(state.preMEM); m++){
				if((state.preMEM[m] == 0) && (i<2) ){
					state.preMEM[m] = state.preIssue[r];
					state.preIssue[r] = state.preIssue[r+1];
					state.preIssue[r+1] = 0;
				}
			}
		}
		}



		// state.PC += 4;
		state.cycle += 1;
		counter ++;

    }










					// Prints the data section
        // stringstream ss1;
        // ss1 << "====================\ncycle:" << state.cycle << "\t"
        //     << "\t" << "\n\nregisters:";
        // for( int i = 0; i < 32; i++ ){
        //     if( i %8 == 0 )
        //         ss1 << "\nr"<<i<<":  ";
        //     ss1 << "\t" << state.R[i];
        //         //ss1 << I.addr;// = address it is currently on
        //     }
	    // cout  << ss1.str() << endl;
        // state.cycle ++;




		/*      OLD CODE
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