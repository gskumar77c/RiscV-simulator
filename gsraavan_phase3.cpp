#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <bitset>
#include <cmath>
#define N 7778777
#define KR 13
using namespace std;


int X[32] ={0};	//register file
int PC,RA,RB,RZ,RY,RM,IM,PC_TEMP,PC_TGT; //intermediate buffers
bitset<32>IR;  //Instruction registor
int apc,tgtcount=0;
bool btake ,actual_branch;
int debug = 0;
int gsrav =0;
int TC,TI,CPI,ALU,CNTRL,DS,CS,MP,DT,DH,CH;
int var=0; 
int c_knob=0;

int srn=8;
bool TAKEN = true;
bool NOT_TAKEN = false;

class mux{
public:
	int muxb;
	int muxy;
	int muxpc;
	int muxinc;
	int finalrd;
	string alu;
	string mem;

};

class mem{
	public:
		unsigned int address;
		std::bitset<8>b;
		mem * next;
	mem(int ad,bitset<8>bg){
		address = ad;
		b = bg;
		next = NULL;
		}

};

class interphase{
public:
	int r1,r2,r3;
	int rg1,rg2,im,rm,rvns;
	bitset<32>ir;
	mux m;
};

class brnget{
public:
	int c_branch;
	int t_branch;
};

std::vector<brnget> BTB;


interphase IPB1,IPB2,IPB3,IPB4;

mem * MEMORY[N] = {NULL};

unsigned int hash_key(string s){
	int n = s.length();
	int k=KR;
	unsigned int i,j;
	long int m=0;
	for(i=0;i<n;i++){
		j = (int)s[i];
		m = (m+((int)pow(k,i))*j)%N;
	}

	j = (m%N);
	return j;

}

mem * access_memory(unsigned int addr){
	int nb = addr%4;
	int i,j;
	addr = addr - nb;
	stringstream ss;
	ss<<std::hex<<addr;
	string s(ss.str());
	s = "0x" + s;
	unsigned int index = hash_key(s);
	mem *  p = MEMORY[index];	
	if(MEMORY[index]==NULL) return p;
	else {
		for(i=0; ;i++){
			if(p->address == (addr+nb)) break;
			else if(p->next == NULL){p = p->next;break;}
			else p = p->next;
		}
	}
	return p;
}

int signed_value(unsigned int n,int k){
	int m = 0;
	if(k==12){
		bitset<12>b1(n);
		if(b1[11]==1){
			bitset<12>b2(-1);
			b2 = b2 ^ b1;
			m = -1*(b2.to_ulong() +1);
		}
		else m = b1.to_ulong();
	}

	if(k==13){
		bitset<13>b1(n);
		//cout<<b1<<endl;
		if(b1[12]==1){
			bitset<13>b2(-1);
			b2 = b2 ^ b1;
			//cout<<b2<<endl;
			m = -1*(b2.to_ulong() +1);
		}
		else m = b1.to_ulong();
	}

	else if(k==32){
		bitset<32>b1(n);
		if(b1[31]==1){
			bitset<32>b2(-1);
			b2 = b2 ^ b1;
			m = -1*(b2.to_ulong() +1);
		}
		else m = b1.to_ulong();
	}

	else if(k==20){
		bitset<20>b1(n);
		if(b1[19]==1){
			bitset<20>b2(-1);
			b2 = b2 ^ b1;
			m = -1*(b2.to_ulong() +1);
		}
		else m = b1.to_ulong();
	}

	else if(k==8){
		bitset<8>b1(n);
		if(b1[7]==1){
			bitset<8>b2(-1);
			b2 = b2 ^ b1;
			m = -1*(b2.to_ulong() +1);
		}
		else m = b1.to_ulong();
	}



	return m ;
}


///////////ASDFG/////////////////////////////////////////////////////////////////////////////
bool GetPrediction(){
  if(srn==1 || srn ==2 || srn==3 || srn==4){
    return NOT_TAKEN; 
  }
  else{
    return TAKEN; 
  }
}

void UpdatePredictor( bool resolveDir){

  

if(srn ==1){
    if(resolveDir == TAKEN) srn = 2;
    else  srn = 1;}

else if(srn ==2){
    if(resolveDir == TAKEN) srn = 3;
    else  srn = 1;}
else if(srn ==3){
    if(resolveDir == TAKEN) srn = 4;
    else  srn = 2;}
else if(srn ==4){
    if(resolveDir == TAKEN) srn = 4;
    else  srn = 3;}
    
    
else if(srn ==5){
    if(resolveDir == TAKEN) srn = 6;
	else  srn = 4;}

else if(srn ==6){
    if(resolveDir == TAKEN) srn = 7;
    else  srn = 5;}
else if(srn ==7){
    if(resolveDir == TAKEN) srn = 8;
    else  srn = 6;}
else if(srn ==8){
    if(resolveDir == TAKEN) srn = 8;
    else  srn = 7;}
  
  }


int load_memory(string str){
	fstream myfile;
	string s1,s2,line;
	int i,j,count =0;
	int dcount = 0;
	int ic=0;
	unsigned int addr,index,instruction;
	myfile.open(str.c_str());
	if(myfile.is_open()){
		while(getline(myfile,line)){
			if(line=="") continue;
			if(line[0]=='$') {dcount = 1;continue;}
			stringstream ss(line);
			ss>>s1>>s2;
				stringstream ss1;
				ss1<<std::hex<<s1;
				ss1>>addr;
				stringstream ss2;
				ss2<<std::hex<<s2;
				ss2>>instruction;
				bitset<32>bg(instruction);
				bitset<32>gs1(255);
				bitset<32>csp(0);
				if(dcount==0)ic++;
				index = hash_key(s1);
					for(i=0;i<4;i++){
						if(i!=0) gs1 = gs1 << 8;
						csp = bg & gs1;
						if(i!=0) csp= csp >> (i*8);
						bitset<8>bg1(csp.to_ulong());
						mem * p = new mem((addr+i),bg1);
						if(MEMORY[index]==NULL) MEMORY[index] = p;
						else {
							mem * q = MEMORY[index];
							for(j=0; ;j++){
								if(q->next==NULL) break;
								else q = q->next;
								}
							q->next = p;
							}
					}
		}
	}

	 else cout<<"Unable to open "<<str<<" file"<<endl;

	 fstream myrvn;
	myrvn.open("BTBuffer.txt");
	if(myrvn.is_open()){
		int tgt;
		while(getline(myrvn,line)){
			if(line=="") continue;
			stringstream ss(line);
			ss>>s1>>s2;
				stringstream ss1;
				ss1<<std::hex<<s1;
				ss1>>addr;
				stringstream ss2;
				ss2<<std::hex<<s2;
				ss2>>instruction;
				//bitset<32>b(instruction);
				tgt = signed_value(instruction,32);
				brnget b;
				b.c_branch = addr;
				b.t_branch = tgt;
				BTB.push_back(b);
				//cout<<"current addr "<<addr<<"final addr "<<tgt<<endl;

			}
	}

	else cout<<"unable to open btbfile "<<endl;

	 return ic;
}



void store_data(unsigned int addr,int data,int nb){
		int i,j;
		cout<<"address store_data "<<addr<<"  data  "<<data<<endl;
		unsigned int ad2,count;
			bitset<32>bg(data);
			bitset<32>gs1(255);
			bitset<32>csp(0);
		if(nb == 4) {
			for(i=0;i<4;i++){
				count = 0;
				ad2 = (addr+i)%4;
				ad2 = (addr+i) - ad2;
			//cout<<ad2<<endl;
				stringstream ss;
				ss<<std::hex<<ad2;
				string s(ss.str());
				s = "0x"  + s;
			
				unsigned int index = hash_key(s);
				mem * p = new mem(0,0);
				//cout<<"index is  "<<index<<endl;
				if(MEMORY[index]!=NULL) p = MEMORY[index];
				//cout<<s<<endl;
				for(j=0; ;j++){
					if(MEMORY[index]==NULL) {count = 1;break;}
					else if(p->address == (addr+i)){count = 3;break;}
					else if(p->next==NULL){count=2;break;}
					else p = p->next;
				}


				if(i!=0) gs1 = gs1 << 8;
				csp = bg & gs1;
				if(i!=0) csp= csp >> (i*8);
				bitset<8>bg1(csp.to_ulong());
				if(count==1) MEMORY[index] = new mem((addr+i),bg1);
				else if(count ==2) p->next = new mem((addr+i),bg1);
				else if(count==3)  p->b = bg1;
			}
		}

		else if(nb==1){
			bitset<8>bgbit(data);
				ad2 = (addr)%4;
				ad2 = (addr) - ad2;
				count  = 0;
			//cout<<ad2<<endl;
				stringstream ss;
				ss<<std::hex<<ad2;
				string s(ss.str());
				s = "0x"  + s;
				unsigned int index = hash_key(s);
				mem * p = new mem(0,0);
				if(MEMORY[index]!=NULL) p = MEMORY[index];
				for(j=0; ;j++){
					if(MEMORY[index]==NULL) {count = 1;break;}
					else if(p->address == (addr)){count = 3;break;}
					else if(p->next==NULL){count=2;break;}
					else p = p->next;
				}

				if(count==1)MEMORY[index] = new mem(addr,bgbit);
				else if(count==2) p->next = new mem(addr,bgbit);
				else if(count==3) p->b = bgbit;
		}


}

void fetch_instruction(unsigned int pc){
	int i;
	IR.reset();
	bitset<32>gs(0);
	bitset<8>srn7(0);
		for(i=0;i<4;i++){
			mem * p = access_memory(pc+i);
			if(p!=NULL)srn7 = p->b;
			else srn7.reset();
			bitset<32>temp(srn7.to_ulong());
			if(i!=0) temp = temp<< (i*8);
			gs = gs | temp; 
				}
	IR = gs;
	bitset<32>bseven(127);
	bitset<32>temp;
	temp = bseven & IR;
	bitset<7>b1(temp.to_ulong());
	string opcode = b1.to_string();//opcode
	debug =0;
	if(opcode=="1101111" || opcode == "1100011"){
		int ig;
		for(ig=0;ig<BTB.size();ig++){
			if(BTB[ig].c_branch==PC){PC_TGT = PC + BTB[ig].t_branch;
					//cout<<"jal fetched "<<PC<<endl;
				if(PC_TGT<0) cout<<"\n\n$$$$$$$$$$$$$$$$$$$$  at pC "<<PC<<" of jal && branch  "<<PC_TGT<<endl;
					if(opcode=="1101111") {tgtcount = 1;}
					else { tgtcount = 2;debug=1;
						   btake=GetPrediction();} 
				break;}
		}
	}
	else if(opcode=="1100111"){
		bitset<32>imd(-1);
		bitset<32>bfive(31);
		imd = imd<<20;
		imd = imd & IR;
		imd = imd>>20;
			temp = bfive<<15;
			temp = temp & IR;
			temp = temp >> 15;
			int rs1 = temp.to_ulong();
		IM = signed_value(imd.to_ulong(),12);
		PC_TGT = X[rs1] + IM;
		if(PC_TGT<0) cout<<"\n\n$$$$$$$$$$$$$$$$$$$$ at pC "<<PC<<" of jalr  "<<PC_TGT<<endl;
		tgtcount = 3;

	}
	else tgtcount =0;
	//cout<<IR<<endl;
	IPB1.ir = IR;
	//if(debug==1)cout<<"EXITING FETCH PC "<<PC<<"   target "<<PC_TGT<<endl;
}



void instruction_decode(){
	//cout<<"enteredd decode"<<endl;
	//cout<<IPB1.ir<<endl;
	bitset<32>bseven(127);
	bitset<32>bfive(31);
	bitset<32>bthree(7);
	bitset<32>temp;
	temp = bseven & IPB1.ir;
	bitset<7>b1(temp.to_ulong());
	string opcode = b1.to_string();//opcode
	temp = bseven<<25;
	temp = temp & IPB1.ir;
	temp = temp>>25;
	bitset<7>b2(temp.to_ulong());
	string func7 = b2.to_string();//func7
	temp = bthree << 12;
	temp = temp & IPB1.ir;
	temp = temp>>12;
	int func3 = temp.to_ulong();//func3
	temp = bfive<<7;
	temp = temp & IPB1.ir;
	temp = temp >> 7;
	int rd = temp.to_ulong();
	temp = bfive<<15;
	temp = temp & IPB1.ir;
	temp = temp >> 15;
	int rs1 = temp.to_ulong();
	temp = bfive<<20;
	temp = temp & IPB1.ir;
	temp = temp>>20;
	int rs2 = temp.to_ulong();
	mux m1;
	PC_TEMP = PC;
	//cout<<PC_TEMP<<"  PC_TEMP"<<endl;
	//cout<<opcode<<endl;
	if(opcode == "0110011"){    ///normal add
			m1.mem = "no";
			ALU++;
			m1.finalrd = rd;
			RA = X[rs1];
			RB = X[rs2];
			//cout<<rd<<endl;
			//cout<<RA<<"  "<<RB<<endl;
			m1.muxb=0;m1.muxy=0;m1.muxpc=0;m1.muxinc=0;
		if(func3==0){
			if(func7=="0000000") m1.alu = "add";
			else if(func7=="0000001") m1.alu = "mul";
			else   m1.alu = "sub";
			}
		else if(func3==1)	m1.alu = "sll";
		else if(func3==2)   m1.alu = "slt";
		else if(func3==3)	m1.alu = "sltu";
		else if(func3==4)	{	if(func7=="0000001") m1.alu = "div";
								else m1.alu = "xor";}
		else if(func3==5)	{
			if(func7=="0000000") m1.alu = "srl";
			else m1.alu="sra";
			}
		else if(func3==6)	m1.alu = "or";
		else if(func3==7)	m1.alu = "and";

		//if(rs1==IPB2.r3 || rs2 == IPB2.r3) IPB2.stall = 2;
		//else if(rs2==IPB3.r3 || rs2 == IPB3.r3) IPB2.stall = 1;
		
	}

		else if(opcode == "0010011"){       ///addi im

		bitset<32>imd(4095);
		imd = imd<<20;
		imd = imd & IPB1.ir;
		imd = imd>>20;
		//cout<<imd<<endl;
		IM = signed_value(imd.to_ulong(),12);
		//cout<<IM<<endl;
		m1.mem = "no";
		m1.muxb = 1;
		m1.muxy = 0;
		m1.muxpc = 0;
		m1.muxinc = 0;
		RA = X[rs1];
		ALU++;
		//cout<<"RS1 --> "<<rs1<<"  x[rs1] --> "<<X[rs1]<<"  RA --> "<<RA<<endl;
		m1.finalrd = rd;
		//if(rd==5) cout<<"]\n\n"<<RA<<"\n\n"<<rs1<<"x0 "<<X[0]<<endl;
		if(func3==0) {m1.alu = "add"; } 
		else if(func3==1) m1.alu =  "sll";
		else if(func3==2) m1.alu = "slt";
		else if(func3==3) m1.alu = "sltu";
		else if(func3==4) m1.alu = "xor";
		else if(func3==5){ 	if(func7=="0000000") m1.alu = "srl";
							else m1.alu="sra";}
		else if(func3==6) m1.alu = "or";	
		else if(func3==7) m1.alu = "and";	


		
	}

		else if(opcode == "0000011"){     ///load instructiosn 
		bitset<32>imd(4095);
		imd = imd<<20;
		imd = imd & IPB1.ir;
		imd = imd>>20;
		IM = signed_value(imd.to_ulong(),12);
		m1.finalrd = rd;
		RA = X[rs1];
		//cout<<"RA me  "<<RA<<endl;
		m1.muxb = 1;
		m1.muxy = 1;
		m1.muxpc = 0;
		m1.muxinc = 0;
		m1.alu = "add";
		if(func3==0) m1.mem = "lb";
		else if(func3==1) m1.mem = "lh";
		else if(func3==2) m1.mem = "lw";
		else if(func3==3) m1.mem = "ld";
		else if(func3==4) m1.mem = "lbu";
		else if(func3==5) m1.mem = "lhu";
		else if(func3==6) m1.mem = "lwu";
		
	}
	else if(opcode == "0010111"){		// auipc
		bitset<32>gsk1(-1);
		gsk1 = gsk1 <<12;
		gsk1 = gsk1 & IPB1.ir;
		m1.muxb=0;m1.muxy=0;m1.muxpc=77;m1.muxinc=0;
		IM = PC + signed_value(gsk1.to_ulong(),32) + apc;
		m1.finalrd = rd;
		//if(PC == 4) cout<<"1st auipc "<<IM<<" rd -->" <<m1.finalrd<<endl;
		//if(PC == 8) cout<<"2nd auipc "<<IM<<" rd -->" <<m1.finalrd<<endl;
		
		m1.alu = "no";
		m1.mem = "no";

	}

		else if(opcode == "0110111"){		// lui
		bitset<32>gsk1(-1);
		gsk1 = gsk1 <<12;
		gsk1 = gsk1 & IPB1.ir;
		m1.muxb=0;m1.muxy=0;m1.muxpc=77;m1.muxinc=0;
		IM =  signed_value(gsk1.to_ulong(),32);
		m1.finalrd = rd;
		//if(PC == 0) cout<<"1st auipc "<<IM<<" rd -->" <<m1.finalrd<<endl;
		//if(PC == 8) cout<<"2nd auipc "<<IM<<" rd -->" <<m1.finalrd<<endl;
		
		m1.alu = "no";
		m1.mem = "no";

	}

		else if(opcode=="0100011"){		//store instructions
		bitset<32>imd(4095);
		imd = imd<<20;
		imd = imd & IPB1.ir;
		imd = imd>>20;
		imd[0] = IPB1.ir[7];imd[1]=IPB1.ir[8];imd[2]=IPB1.ir[9];imd[3]=IPB1.ir[10];
		imd[4] = IPB1.ir[11];
		IM = signed_value (imd.to_ulong(),12);
		m1.finalrd = -1;
		RA = X[rs1];
		RB = X[rs2];
		//cout<<"decode rm "<<RM<<endl;
		rd = -1;
		m1.muxb = 1;
		m1.muxy = 0;
		m1.muxpc = 0;
		m1.muxinc = 0;
		m1.alu = "add";
		if(func3==0) m1.mem = "sb";
		else if(func3==1) m1.mem = "sh";
		else if(func3==2) m1.mem = "sw";
		else if(func3==3) m1.mem = "sd";
	}

		else if(opcode=="1101111"){		//jal
			CNTRL++;
		bitset<32>imd1(-1);
		bitset<32>gs(0);
		imd1 = imd1 >>22;
		imd1 = imd1 << 21;
		//cout<<"10:1--> "<<imd1<<endl;
		gs = imd1 & IPB1.ir;
		imd1 = imd1 >>23;
		//cout<<"8bits 1s "<<imd1<<endl;
		gs = gs >> 20;
		imd1 = imd1<<12;
		//cout<<"after shifting 8 bits "<<imd1<<endl;
		imd1 = imd1 & IPB1.ir;
		gs = gs | imd1;
		gs[20] =  IPB1.ir[31];
		gs[11] = IPB1.ir[20];
		IM = signed_value(gs.to_ulong(),20);
		IPB2.rvns = PC;
		m1.muxpc = 0;
		m1.muxinc = 1;
		m1.finalrd = rd;
		//cout<<m1.finalrd<<"  finalrd "<<PC<<endl;
		m1.alu = "jal";
		m1.mem = "no";
		m1.muxy = 3;
		//cout<<"hi raaavana   " <<PC_TEMP<<"   original pc  "<<PC<<endl;
	}

	else if(opcode == "1100111"){		//jalr
		CNTRL++;
		bitset<32>imd(-1);
		imd = imd<<20;
		imd = imd & IPB1.ir;
		imd = imd>>20;
		IM = signed_value(imd.to_ulong(),12);
		m1.finalrd = rd;
		IPB2.rvns = PC;
		RA = X[rs1];
		m1.muxpc = 1;
		m1.muxinc = 1;
		m1.muxb = 1;
		m1.muxy = 3;
		m1.alu = "jalr";
		m1.mem = "no";
	}

		else if(opcode=="1100011"){		//branch instructions
		CNTRL++;
		bitset<32>gsk(0);
		bitset<32>srn(-1);
		srn = srn >> 26;
		srn = srn << 25;
		gsk = IPB1.ir & srn;
		gsk = gsk >>20;
		srn = srn>>27;
		srn = srn << 8;
		srn = srn & IPB1.ir;
		srn = srn>>7;
		gsk = gsk | srn;
		gsk[11] = IPB1.ir[7];
		gsk[12] = IPB1.ir[31];
		IM = signed_value(gsk.to_ulong(),13);
		m1.finalrd = -1;
		RA = X[rs1];
		RB = X[rs2];
		m1.muxb = 0;
		m1.muxy = 0;
		m1.muxpc = 0;
		m1.muxinc = -7;
		m1.mem = "no";
		if(func3==0) {m1.alu = "beq";IPB2.rvns = 0;}
		else if(func3==1) {m1.alu = "bne";IPB2.rvns = 1;}
		else if(func3==4) {m1.alu = "blt"; IPB2.rvns = 2;}
		else if(func3==5) {m1.alu = "bge"; IPB2.rvns = 3;}
		else if(func3==6) {m1.alu = "bltu"; IPB2.rvns = 4;}
		else if(func3==7) {m1.alu = "bgeu"; IPB2.rvns = 5;}

	}
	else if(opcode=="0000000"){
		m1.finalrd = -1;
		m1.muxb = 0;
		m1.muxy = 0;
		m1.muxpc = 0;
		m1.muxinc = 0;
		m1.alu = "no";
		m1.mem = "no";
	}


			IPB2.r1 = rs1;
			IPB2.r2 = rs2;
			IPB2.r3 = m1.finalrd;
			IPB2.rg1 = RA;
			IPB2.rg2 = RB;
			IPB2.im = IM; 
			IPB2.m =  m1;
			IPB2.rm = RM;

			//if(debug==1)cout<<"Exiting DECODE PC "<<PC<<endl;
}

void execute(){

	//if(debug==1)cout<<"ENTERING EXECUTE PC "<<PC<<endl;
	mux m = IPB2.m;
	bitset<32>gsk1(IPB2.rg1);
	bitset<32>gsk2(IPB2.rg2);
	bitset<32>gsk3(IPB2.im);
	if(m.alu=="add"){
		if(m.muxb==0)    RZ = IPB2.rg1 + IPB2.rg2;
		else if(m.muxb==1) {RZ = IPB2.rg1 + IPB2.im ;}
	}
	else if(m.alu=="mul") RZ = IPB2.rg1 * IPB2.rg2;
	else if(m.alu=="sub") RZ = IPB2.rg1 - IPB2.rg2;
	else if(m.alu=="or") { 
					if(m.muxb==0) gsk1 = gsk1 | gsk2;
					else if(m.muxb==1) gsk1 = gsk1 | gsk3 ;
					RZ = signed_value(gsk1.to_ulong(),32);
					}
	else if(m.alu=="and") {
							if(m.muxb==0) gsk1 = gsk1 & gsk2 ;
							else if(m.muxb==1) gsk1 = gsk1 & gsk3;
							RZ = signed_value(gsk1.to_ulong(),32);
							}
	else if(m.alu=="xor"){
							if(m.muxb==0)  gsk1 = gsk1 ^ gsk2 ;
							else if(m.muxb==1) gsk1 = gsk1 ^ gsk3;
							RZ = signed_value(gsk1.to_ulong(),32);
							}
	else if(m.alu=="sll"){ 
							if(m.muxb==0) gsk1 = gsk1 << (gsk2.to_ulong());
							else if(m.muxb==1) gsk1 = gsk1 << (gsk3.to_ulong());
							RZ = signed_value(gsk1.to_ulong(),32);
							}
	else if(m.alu=="srl"){ 
							if(m.muxb==0) gsk1 = gsk1 >> gsk2.to_ulong();
							else if(m.muxb==1) gsk1 = gsk1 >> gsk3.to_ulong();
							RZ = signed_value(gsk1.to_ulong(),32);
							}
	else if(m.alu=="slt"){
							if(m.muxb==0) m.finalrd = (IPB2.rg1<IPB2.rg2)?1:0;
							else if(m.muxb==1) m.finalrd = (IPB2.rg1<IPB2.im)?1:0;
							}	
	else if(m.alu=="sltu"){
							if(m.muxb==0) {bitset<32>ba(IPB2.rg1);bitset<32>bb(IPB2.rg2);
										 m.finalrd = (ba.to_ulong()<bb.to_ulong())?1:0;}
							else if(m.muxb==1){bitset<32>ba(IPB2.rg1);bitset<32>bb(IPB2.im);
										 m.finalrd = (ba.to_ulong()<bb.to_ulong())?1:0;}
							}
	else if(m.alu=="beq") RZ =  (IPB2.rg1==IPB2.rg2)?1:0;
	else if(m.alu=="bne") RZ =  (IPB2.rg1!=IPB2.rg2)?1:0;
	else if(m.alu=="blt") RZ =  (IPB2.rg1<IPB2.rg2)?1:0;
	else if(m.alu=="bge") RZ =  (IPB2.rg1>=IPB2.rg2)?1:0;
	else if(m.alu=="bltu"){bitset<32>ba(IPB2.rg1);bitset<32>bb(IPB2.rg2);
							RZ = (ba.to_ulong()<bb.to_ulong())?1:0;}
	else if(m.alu=="bgeu"){bitset<32>ba(IPB2.rg1);bitset<32>bb(IPB2.rg2);
							RZ = (ba.to_ulong()>=bb.to_ulong())?1:0;}

	if(m.muxinc==-7 && RZ == 0) m.muxinc = 0;
	else if(m.muxinc==-7 && RZ ==1) m.muxinc=1; 

	if(m.muxpc==0){
			if(m.muxinc==0) PC+=4;
			else if(m.muxinc==1) {
					if(IPB2.im%4!=0) IPB2.im = IPB2.im - IPB2.im%4;
						PC = PC + IPB2.im ;
						 }
		}
	else if(m.muxpc==1) {
			if(m.muxinc==0) PC = IPB2.rg1 + 4;
			else if(m.muxinc==1) {PC = IPB2.rg1 + IPB2.im; if(m.alu=="jalr" && gsrav==0)PC = PC+4;}//cout<<"\n\n\n\n\nPC to which jalr jump "<<PC<<endl<<endl; }
		} 

	else if(m.muxpc==77){ RZ = IPB2.im;PC = PC + 4;}
	
	RM = IPB2.rg2;
	IPB3.m = m;
	IPB3.rg1 = RZ;
	IPB3.r3 = IPB2.r3;
	IPB3.rm = IPB2.rg2;
	IPB3.rvns = IPB2.rvns;


	//if(debug==1)cout<<"EXITING EXECUTE PC "<<PC<<endl;
}


int loading_data(int x,unsigned int addr){
	int value,i,j;
	if(x==1 || x==7){
			bitset<32>gs(0);
			bitset<8>srn7(0);
		for(i=0;i<4;i++){
			mem * p = access_memory(addr+i);
			if(p!=NULL)srn7 = p->b;
			else srn7.reset();
			bitset<32>temp(srn7.to_ulong());
			if(i!=0) temp = temp<< (i*8);
			gs = gs | temp; 
				}
			if(x==1)value = signed_value(gs.to_ulong(),32);
			else value = gs.to_ulong();
	}

	else if(x==3){
		bitset<8>srn(0);
		mem *p = access_memory(addr);
		if(p!=NULL) srn = p->b;
		value = signed_value(srn.to_ulong(),8);
	}
	//cout<<"loading_data address  "<<addr<<" data "<<value<<endl;
	return value;
}


void memory(){

	//if(debug==1)cout<<"ENTERING MEMORY PC "<<PC<<endl;
	int MEMDATA;
	mux m = IPB3.m;
	if(m.mem=="lw") MEMDATA = loading_data(1,IPB3.rg1);
	else if(m.mem=="ld") MEMDATA = loading_data(2,IPB3.rg1);
	else if(m.mem=="lb") MEMDATA =  loading_data(3,IPB3.rg1);
	else if(m.mem=="lh") MEMDATA = loading_data(4,IPB3.rg1);
	else if(m.mem=="lbu") MEMDATA = loading_data(5,IPB3.rg1);
	else if(m.mem=="lhu") MEMDATA = loading_data(6,IPB3.rg1);
	else if(m.mem=="lwu") MEMDATA = loading_data(7,IPB3.rg1);
	else if(m.mem=="sw") store_data(IPB3.rg1,IPB3.rm,4);
	else if(m.mem=="sd") store_data(IPB3.rg1,IPB3.rm,8);
	else if(m.mem=="sb") store_data(IPB3.rg1,IPB3.rm,1);
    
    if(m.muxy==1) RY = MEMDATA;
    else if(m.muxy==0) RY = IPB3.rg1;
    else if(m.muxy==2) RY = PC_TEMP;
    else if(m.muxy==3) {RY = IPB3.rvns + gsrav;}


    IPB4.m = m;
    IPB4.r3 = IPB3.r3;
    IPB4.rg1 = RY;

    //if(debug==1)cout<<"EXITING MEMORY PC "<<PC<<endl;
}
int cycles_count=0;
void write_back(){
		
//if(debug==1)cout<<"ENTERING WRITEBACK PC "<<PC<<endl;
		mux m = IPB4.m;
		if(m.finalrd>0) X[m.finalrd] = IPB4.rg1;
		 //cout<<PC<<" -->pc where "<<m.finalrd<<" is changed "<<X[m.finalrd]<<endl;
 

		 //if(debug==1)cout<<"EXITING WRITEBACK PC "<<PC<<endl;
	if(cycles_count == var && c_knob==1){
		cout<<"indsidez_write_back:\n";
		for(int i=0;i<32;i++){
		cout<<"\tX["<<i<<"]  -->  " <<X[i]<<endl;
		//cout<<"X["<<i<<"]\t-->\t "<<X[i]<<"\n\n"<<endl;
		}
		
	}
	cycles_count++;


}


void run_simulator_with_stalling(int IC){
	TC = CNTRL = DS = CS = DH = CH = MP = 0 ;
	PC = 0;
	apc = -4;
	gsrav =0;
	IR.set(0);
	IC=IC*4;
	int cycles=0;
	int stall[5] = {0};
	int A[5];
	int i,j,k,stld,count;j=0;k=0;
	int raavan_control = 0;
	int flush=0;
	for(i=1;i<6;i++) A[i-1]=-1*i;
	while(PC<IC || raavan_control<2){
		if(PC>IC) raavan_control++;
		if(PC<IC) raavan_control=0;
		if(PC==IC+16) break; 
		if(cycles==1) PC=PC+4;
		count = 0;
		j = k;
		for(i=0;i<5;i++){
			if(stall[(i+j)%5]>0){stall[(i+j)%5]--;
							
							if(stall[(i+j)%5]!=0) break;
							else count = 1;}
			if(A[(i+j)%5]!=5) A[(i+j)%5]++;
			//if(A[(i+j)%5]==5 && i>0){if(A[(i+j)%5-1]==0)A[(i+j)%5]=0;}
			if(A[(i+j)%5]==0){
				//if(cycles==11) cout<<"fetched"<<endl;
				if(tgtcount==1) fetch_instruction(PC_TGT);
				else if(tgtcount==2 && actual_branch==true) fetch_instruction(PC_TGT);
				else if(tgtcount==2 && actual_branch==false) fetch_instruction(PC);
				else if(tgtcount==3) fetch_instruction(PC_TGT);
				else fetch_instruction(PC);
			}
			else if(A[(i+j)%5]==1){
				//if(cycles==11) cout<<"instruction_decode"<<endl;
				int tempct=0;
				instruction_decode();
				
				if(count==1){count=0;stld=1;A[(i+j+1)%5] = -1;continue;}
				if((IPB2.r1==IPB3.r3 || IPB2.r2 == IPB3.r3) && cycles!=1){
					stall[(i+j)%5] = 2;

					DS = DS + 2;
					DH++;
					A[(i+j)%5] = 0;tempct=1;//break;
					//cout<<IPB2.r1<<"--checking dependency--"<<cycles<<endl;
				}
				else if((IPB2.r1==IPB4.r3 || IPB2.r2 == IPB4.r3) && cycles!=1 && stld!=1 && cycles!=2 && tempct!=1){
					stall[(i+j)%5] = 1;

					DS++;

					A[(i+j)%5] = 0;tempct=1;//break;
				}
				stld =0;
				 if(IPB2.r1==IPB4.r3 || IPB2.r2 == IPB4.r3) DH++;

				if(tgtcount==3){
						if(IPB2.r1==IPB4.r3){PC_TGT = IPB2.im + IPB4.rg1;}
						else if(IPB2.r1==IPB3.r3){PC_TGT= IPB2.im + IPB3.rg1;}
						else {PC_TGT = IPB2.rg1 + IPB2.im;}
				}
    
				if(tempct!=1)A[(i+j+1)%5] = -1;
				if(tgtcount==2){
					int gsk3;
					 if(IPB2.rvns==0) gsk3 =  (IPB2.rg1==IPB2.rg2)?1:0;
						else if(IPB2.rvns==1) gsk3 =  (IPB2.rg1!=IPB2.rg2)?1:0;
						else if(IPB2.rvns==2) gsk3 =  (IPB2.rg1<IPB2.rg2)?1:0;
						else if(IPB2.rvns==3) {gsk3 =  (IPB2.rg1>=IPB2.rg2)?1:0;}
						else if(IPB2.rvns==4){bitset<32>ba(IPB2.rg1);bitset<32>bb(IPB2.rg2);
							gsk3 = (ba.to_ulong()<bb.to_ulong())?1:0;}
						else if(IPB2.rvns==5){bitset<32>ba(IPB2.rg1);bitset<32>bb(IPB2.rg2);
							gsk3 = (ba.to_ulong()>=bb.to_ulong())?1:0;}

					if((btake==true && gsk3==0)||(btake==false && gsk3==1)){
								A[(i+j+1)%5] = -2;
								//tgtcount = 0;
								flush++;
								CS++;
								CH++;
								MP++;
								if(gsk3==0) actual_branch = false;
								else actual_branch = true;
								UpdatePredictor(actual_branch);
							}
					else if(btake==false && gsk3==0){
						actual_branch = false;
						UpdatePredictor(actual_branch);
						CH++;
					}
					else if(btake==true && gsk3==1){
						actual_branch = true;
						UpdatePredictor(actual_branch);
						CH++;
					}
					

					}
					if(tempct==1) break;


			}

			else if(A[(i+j)%5]==2){
				//if(cycles==11) cout<<"execute"<<endl;
				execute();
				//if(PC == PC_TGT) PC = PC + 4;
			}
			else if(A[(i+j)%5]==3){
				//if(cycles==11) cout<<"memory"<<endl;
				memory();
			}
			else if(A[(i+j)%5]==4){
				//if(cycles==11) cout<<"write_back"<<endl;
				write_back();
				k = (i+j)%5+1;
			}

			//if(i==4 && A[i]==0) A[0] = 0;
			//else if(A[i]==5 && A[i-1]==0)
		}


		cycles++;
		
		//cout<<"cycle-->" <<cycles<<"  pc "<<PC<<" Ic "<<IC<<" PC_TGT"<<PC_TGT<<endl;
		if(cycles==180) break;
	}

	TC = cycles;

}

int ary[31];

void run_simulator_with_dataforwading(int IC){
	PC = 0;
	TC = CNTRL = DS = CS = DH = CH = MP = 0 ;
	apc = -4;
	int flush=0;
	gsrav =0;
	IR.set(0);
	IC=IC*4;
	int cycles=0;
	int stall[5] = {0};
	int A[5];
	int i,j,k,count,stld;j=0;k=0;
	int raavan_control = 0;
	for(i=1;i<6;i++) A[i-1]=-1*i;
	while(PC<IC || raavan_control<3){
		if(PC>=IC) raavan_control++;
		if(PC<IC) raavan_control=0;
		if(PC==IC+16) break; 


		if(cycles==1) PC=PC+4;
		count = 0;
		j = k;
		for(i=0;i<5;i++){
			if(stall[(i+j)%5]>0){stall[(i+j)%5]--;
							//break;
							if(stall[(i+j)%5]!=0) break;
							else count = 1;
						}
			if(A[(i+j)%5]!=5) A[(i+j)%5]++;
			//if(A[(i+j)%5]==5 && i>0){if(A[(i+j)%5-1]==0)A[(i+j)%5]=0;}
			if(A[(i+j)%5]==0){
				//if(cycles==11) cout<<"fetched"<<endl;
				if(tgtcount==1) {fetch_instruction(PC_TGT);}
				else if(tgtcount==2 && actual_branch==true) fetch_instruction(PC_TGT);
				else if(tgtcount==2 && actual_branch==false) fetch_instruction(PC);
				else if(tgtcount==3){ fetch_instruction(PC_TGT);}
				else fetch_instruction(PC);
			}
			else if(A[(i+j)%5]==1){
				instruction_decode();
				int subtc=0;
				
				if(count==1){
					//load after add instruction stalling and forwarding
					count=0;
					if(IPB4.m.mem[0]=='l'){
						if(IPB2.r1==IPB4.r3) IPB2.rg1 = IPB4.rg1;
						else IPB2.rg2 = IPB4.rg1;
						DH++;}
					A[(i+j+1)%5] = -1;continue;
					}

				if((IPB2.r1==IPB4.r3 || IPB2.r2 == IPB4.r3) && cycles!=1 && cycles!=2){
					//M-->E data forwarding
					//cout<<IPB2.m.mem<<IPB2.m.alu<<endl;
					DH++;
					if(IPB2.r1==IPB4.r3) {IPB2.rg1 = IPB4.rg1;}
					else IPB2.rg2 = IPB4.rg1;
					//if(IPB2.r3==5) cout<<"data forwardietg 1 &&&&&&&&&&&&&&&"<<endl;
					//stall[(i+j)%5] = 1;
					//A[(i+j)%5] = 0;break;
				}
				if((IPB2.r1==IPB3.r3 || IPB2.r2 == IPB3.r3) && cycles!=1){
					//E-->E data forwarding
					
					if(IPB3.m.mem[0]!='l'){
						//cout<<IPB2.r2<<"   forewardd"<<endl;
						if(IPB2.r1==IPB3.r3) IPB2.rg1 = IPB3.rg1;
						else IPB2.rg2 = IPB3.rg1;
						DH++;
						}
					else {
						stall[(i+j)%5] = 1;
						DS++;
						DH++;
						A[(i+j)%5] = 0;subtc=1;//break;
						}
					//if(IPB2.r3==5) cout<<"data forwardietg 2 &&&&&&&&&&&&&&&"<<endl;
					//cout<<IPB2.r1<<"--checking dependency--"<<cycles<<endl;
				}

				
				if(tgtcount==3){
						
						if(IPB2.r1==IPB4.r3){PC_TGT = IPB2.im + IPB4.rg1;}
						else if(IPB2.r1==IPB3.r3){PC_TGT= IPB2.im + IPB3.rg1;}
						else {PC_TGT = IPB2.rg1 + IPB2.im;}
				}

				if(subtc!=1)A[(i+j+1)%5] = -1;

			if(tgtcount==2){

					int gsk3;

					 if(IPB2.rvns==0) gsk3 =  (IPB2.rg1==IPB2.rg2)?1:0;
						else if(IPB2.rvns==1) gsk3 =  (IPB2.rg1!=IPB2.rg2)?1:0;
						else if(IPB2.rvns==2) gsk3 =  (IPB2.rg1<IPB2.rg2)?1:0;
						else if(IPB2.rvns==3) gsk3 =  (IPB2.rg1>=IPB2.rg2)?1:0;
						else if(IPB2.rvns==4){bitset<32>ba(IPB2.rg1);bitset<32>bb(IPB2.rg2);
							gsk3 = (ba.to_ulong()<bb.to_ulong())?1:0;}
						else if(IPB2.rvns==5){bitset<32>ba(IPB2.rg1);bitset<32>bb(IPB2.rg2);
							gsk3 = (ba.to_ulong()>=bb.to_ulong())?1:0;}
					if((btake==true && gsk3==0)||(btake==false && gsk3==1)){
								A[(i+j+1)%5] = -2;
								//tgtcount = 0;
								CH++;
								flush++;
								MP++;
								if(gsk3==0) actual_branch = false;
								else actual_branch = true;
								UpdatePredictor(actual_branch);
							}

					else if(btake==false && gsk3==0){
						cout<<"predicted correct$$$$$$$$$$$$$$$"<<endl;
						actual_branch = false;
						UpdatePredictor(actual_branch);
					}
					else if(btake==true && gsk3==1){
						cout<<"predicted coreect $$$$$$$$$$$$$$$"<<endl;
						actual_branch = true;
						UpdatePredictor(actual_branch);
					}
					

					}

					if(subtc==1) break;
			}

			else if(A[(i+j)%5]==2){
				execute();
			}
			else if(A[(i+j)%5]==3){
				memory();
			}
			else if(A[(i+j)%5]==4){
				write_back();
				k = (i+j)%5+1;
			}
		}

                
		cycles++;
		//cout<<"cycle-->" <<cycles<<"  pc "<<PC<<" Ic "<<IC<<" PC_TGT"<<PC_TGT<<endl;
		if(cycles==170) break;
		
	}

}

void run_simulator(int IC){
	PC = 0;
	apc =0;
	IR.set(0);
	gsrav = 4;
	IC=IC*4;
	int counter = 0;
	while(PC<IC){
		fetch_instruction(PC);
		instruction_decode();
		execute();
		memory();
		write_back();
		TI = (IC - 1)/4;
		TC = TI*5;

		//cout<<PC<<endl;
		//if(PC>100 || PC<-100){break;}
		//if(PC==16)
		counter++;
		if(counter>80) break;
	}

}



int main(int argc, char const *argv[])
{	


	
	int ic = load_memory("test.mc");
	int i;
	X[2] = 2147483632;
	X[3] = 268435456;
	mem * p = access_memory(64);
	PC = 68;
	TI = ic;
	int count_knob=0;
	cout<<"Enter 1 For Enabling of register file printing\nEnter 2 For Disabeling of register file printing \n";
	cin>>count_knob;
	if(count_knob==1){
		c_knob=1;
		cout<<"\tEnter the cycle number for which you want to see register values:  ";
		cin>>var;

	}
	
	int p_knob=0;
	cout<<"Enter 1 For Enabling pipelining \nEnter 2 For Disabeling pipelining\n";
	cin>>p_knob;

	if(p_knob==1){
		int switchh=0;
		cout<<"\tEnter 1 for stallling\n\tEnter 2 for data forwarding\n\t";
		cin>>switchh;
		if(switchh==1)	run_simulator_with_stalling(ic);
		if (switchh==2) run_simulator_with_dataforwading(ic);
	
	}
	else if(p_knob==2){
			run_simulator(ic);
	}




	for(i=0;i<32;i++){
	cout<<"X["<<i<<"]  -->  " <<X[i]<<endl;
	//cout<<"X["<<i<<"]\t-->\t "<<X[i]<<"\n\n"<<endl;
	}
	//int mu = loading_data(1,268435460);
	//cout<<mu<<"  gsk"<<endl;
	cout<<"\n___________________________________________________\n";
	cout<<"1.Total instruction\t\t\t\t"<<TI<<endl;
	cout<<"2.Total number of cycles\t\t\t"<<TC<<endl;
	cout<<"3.CPI\t\t\t\t\t\t"<<float(TC)/float(TI)<<endl;
	cout<<"4.Number of Data-transfer instructions\t\t"<<TI-DH-CH<<endl;
	cout<<"5.Number of ALU instructions\t\t\t"<<ALU<<endl;
	cout<<"6.Total CNTRL\t\t\t\t\t"<<CNTRL<<endl;
	cout<<"7.Number of stalls/bubble in the pipeline\t"<<DS+CS<<endl;
	cout<<"8.Number of Data_hazards\t\t\t"<<DH<<endl;
	cout<<"9.Number of Control_hazards\t\t\t"<<CH<<endl;
	cout<<"10.Number of stalls due to data hazards\t\t"<<DS<<endl;
	cout<<"11.Number of stalls due to control hazards\t"<<CS<<endl;
	cout<<"12.Number of branch misspredicted\t\t"<<MP<<endl;



	return 0;
}