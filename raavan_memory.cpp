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
int TC,TI,CPI,ALU,CNTRL,DS,CS,MP,DT,DH,CH,RAV,GSK,RAVB,GSKB;
int ICH,ICM,DCH,DCM;

int IMREQ,DMREQ;
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

int GRM;

class cach{
public:
	bitset<32> tag;
	bitset<8>data;
	bool valid;
	cach(){
		tag.reset();
		data.reset();
		valid = false;
	}
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
cach * Instruction_cache ;
cach * Data_cache;

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
	IMREQ++;
	bitset<32>cah(addr);
	int bt = (int)(log(RAV)/log(2));
	bitset<32>ctag;
	ctag = cah>>bt;
	bitset<32>cin;
	cin = cah<<(32-bt);
	cin = cin>>(32-bt);

	//if(Instruction_cache[cin.to_ulong()].valid==false) cout<<"asdf;jsdfj asfdasfj afjadsf;asfjasd;fj"<<endl;
	if(Instruction_cache[cin.to_ulong()].valid==true && Instruction_cache[cin.to_ulong()].tag == ctag){
				mem * p = new mem(addr,Instruction_cache[cin.to_ulong()].data);
				if(PC<GRM)ICH++;
				return p;

	}


	int nb = addr%4;
	int i,j;
	addr = addr - nb;
	stringstream ss;
	ss<<std::hex<<addr;
	string s(ss.str());
	s = "0x" + s;
	unsigned int index = hash_key(s);
	mem *  p = MEMORY[index];	
	if(MEMORY[index]==NULL) {
		bitset<8>gsd(0);
		Instruction_cache[cin.to_ulong()].data = gsd;
		Instruction_cache[cin.to_ulong()].tag = ctag;
		Instruction_cache[cin.to_ulong()].valid = true;
		if(PC<GRM)ICM++;
		return p;}
	else {
		for(i=0; ;i++){
			if(p->address == (addr+nb)) break;
			else if(p->next == NULL){p = p->next;break;}
			else p = p->next;
		}
	}
	

	if(p!=NULL){
		Instruction_cache[cin.to_ulong()].data = p->b;
		Instruction_cache[cin.to_ulong()].tag = ctag;
		Instruction_cache[cin.to_ulong()].valid = true;
		//cout<<cin.to_ulong()<<"\t\t\t"<<bin.to_ulong()<<endl;
		if(PC<GRM)ICM++;
		
	}
	return p;
}

mem * access_datamemory(unsigned int addr){
	DMREQ++;
	bitset<32>cah(addr);
	int bt = (int)(log(GSK)/log(2));//RAV
	bitset<32>ctag;
	ctag = cah>>bt;
	bitset<32>cin;
	cin = cah<<(32-bt);
	cin = cin>>(32-bt);

	if(Data_cache[cin.to_ulong()].valid==true && Data_cache[cin.to_ulong()].tag == ctag){
				mem * p = new mem(addr,Data_cache[cin.to_ulong()].data);
				DCH++;
					return p;

	}


	int nb = addr%4;
	int i,j;
	addr = addr - nb;
	stringstream ss;
	ss<<std::hex<<addr;
	string s(ss.str());
	s = "0x" + s;
	unsigned int index = hash_key(s);
	mem *  p = MEMORY[index];	
	if(MEMORY[index]==NULL) {		
		bitset<8>bre(0);
		Data_cache[cin.to_ulong()].data = bre;
		Data_cache[cin.to_ulong()].tag = ctag;
		Data_cache[cin.to_ulong()].valid = true;
		DCM++;
		return p;
		}

	else {
		for(i=0; ;i++){
			if(p->address == (addr+nb)) break;
			else if(p->next == NULL){p = p->next;break;}
			else p = p->next;
		}
	}


	if(p!=NULL){
		Data_cache[cin.to_ulong()].data = p->b;
		Data_cache[cin.to_ulong()].tag = ctag;
		Data_cache[cin.to_ulong()].valid = true;
		DCM++;
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
		if(b1[12]==1){
			bitset<13>b2(-1);
			b2 = b2 ^ b1;
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
				tgt = signed_value(instruction,32);
				brnget b;
				b.c_branch = addr;
				b.t_branch = tgt;
				BTB.push_back(b);
			}
	}

	else cout<<"unable to open btbfile "<<endl;

	 return ic;
}



void store_data(unsigned int addr,int data,int nb,int cachac = 0){
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
				stringstream ss;
				ss<<std::hex<<ad2;
				string s(ss.str());
				s = "0x"  + s;
			
				unsigned int index = hash_key(s);
				mem * p = new mem(0,0);
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
				if(cachac==1){
					bitset<32>cah(addr+i);
					int bt = (int)(log(GSK)/log(2));//RAV
					bitset<32>ctag;
					ctag = cah>>bt;
					bitset<32>cin;
					cin = cah<<(32-bt);
					cin = cin>>(32-bt);
					Data_cache[cin.to_ulong()].tag = ctag;
					Data_cache[cin.to_ulong()].data = bg1;
					Data_cache[cin.to_ulong()].valid = true;}

			}
		}

		else if(nb==1){
			bitset<8>bgbit(data);
				ad2 = (addr)%4;
				ad2 = (addr) - ad2;
				count  = 0;
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
				if(cachac==1){
					bitset<32>cah(addr+i);
					int bt = (int)(log(GSK)/log(2));//RAV
					bitset<32>ctag;
					ctag = cah>>bt;
					bitset<32>cin;
					cin = cah<<(32-bt);
					cin = cin>>(32-bt);
					Data_cache[cin.to_ulong()].tag = ctag;
					Data_cache[cin.to_ulong()].data = bgbit;
					Data_cache[cin.to_ulong()].valid = true;}
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
					if(opcode=="1101111") {tgtcount = 1;}
					else { tgtcount = 2;debug=1;
						   btake=false;} 
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
		tgtcount = 3;

	}
	else tgtcount =0;
	IPB1.ir = IR;
}



void instruction_decode(){

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
	if(opcode == "0110011"){    ///normal add
			m1.mem = "no";
			m1.finalrd = rd;
			RA = X[rs1];
			RB = X[rs2];
		
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
		m1.finalrd = rd;
		if(func3==0) m1.alu = "add";
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
		m1.alu = "jal";
		m1.mem = "no";
		m1.muxy = 3;
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
}

void execute(){

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

}


int loading_data(int x,unsigned int addr){
	int value,i,j;
	if(x==1 || x==7){
			bitset<32>gs(0);
			bitset<8>srn7(0);
		for(i=0;i<4;i++){
			mem * p = access_datamemory(addr+i);
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
		mem *p = access_datamemory(addr);
		if(p!=NULL) srn = p->b;
		value = signed_value(srn.to_ulong(),8);
	}

	return value;
}


void memory(){

	int MEMDATA;
	mux m = IPB3.m;
	if(m.mem=="lw") MEMDATA = loading_data(1,IPB3.rg1);
	else if(m.mem=="ld") MEMDATA = loading_data(2,IPB3.rg1);
	else if(m.mem=="lb") MEMDATA =  loading_data(3,IPB3.rg1);
	else if(m.mem=="lh") MEMDATA = loading_data(4,IPB3.rg1);
	else if(m.mem=="lbu") MEMDATA = loading_data(5,IPB3.rg1);
	else if(m.mem=="lhu") MEMDATA = loading_data(6,IPB3.rg1);
	else if(m.mem=="lwu") MEMDATA = loading_data(7,IPB3.rg1);
	else if(m.mem=="sw") store_data(IPB3.rg1,IPB3.rm,4,1);
	else if(m.mem=="sd") store_data(IPB3.rg1,IPB3.rm,8,1);
	else if(m.mem=="sb") store_data(IPB3.rg1,IPB3.rm,1,1);
    
    if(m.muxy==1) RY = MEMDATA;
    else if(m.muxy==0) RY = IPB3.rg1;
    else if(m.muxy==2) RY = PC_TEMP;
    else if(m.muxy==3) {RY = IPB3.rvns + gsrav;}


    IPB4.m = m;
    IPB4.r3 = IPB3.r3;
    IPB4.rg1 = RY;

}

void write_back(){
	
		mux m = IPB4.m;
		if(m.finalrd>0) X[m.finalrd] = IPB4.rg1;
		 //cout<<PC<<" -->pc where "<<m.finalrd<<" is changed "<<X[m.finalrd]<<endl;
 
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
			if(A[(i+j)%5]==0){
				if(tgtcount==1) fetch_instruction(PC_TGT);
				else if(tgtcount==2 && actual_branch==true) fetch_instruction(PC_TGT);
				else if(tgtcount==2 && actual_branch==false){ fetch_instruction(PC); }
				else if(tgtcount==3){ fetch_instruction(PC_TGT);}
				else {fetch_instruction(PC);}
			}
			else if(A[(i+j)%5]==1){
				int tempct=0;
				instruction_decode();
				
				if(count==1){count=0;stld=1;A[(i+j+1)%5] = -1;continue;}
				if((IPB2.r1==IPB3.r3 || IPB2.r2 == IPB3.r3) && cycles!=1){
					stall[(i+j)%5] = 2;

					DS = DS + 2;
					DH++;
					A[(i+j)%5] = 0;tempct=1;//break;
				}
				else if((IPB2.r1==IPB4.r3 || IPB2.r2 == IPB4.r3) && cycles!=1 && stld!=1 && cycles!=2 && tempct!=1){
					stall[(i+j)%5] = 1;

					DS++;

					A[(i+j)%5] = 0;tempct=1;//break;
				}
				stld =0;
				 if(IPB2.r1==IPB4.r3 || IPB2.r2 == IPB4.r3) DH++;

				if(tgtcount==3){
						if(IPB2.r1==IPB4.r3){PC_TGT = IPB2.im + IPB4.rg1; }
						else if(IPB2.r1==IPB3.r3){PC_TGT= IPB2.im + IPB3.rg1; }
						else {PC_TGT = IPB2.rg1 + IPB2.im; }
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
								flush++;
								CS++;
								CH++;
								MP++;
								if(gsk3==0) actual_branch = false;
								else actual_branch = true;
							}
					else if(btake==false && gsk3==0){
						
						actual_branch = false;
						CH++;
					}
					else if(btake==true && gsk3==1){
						
						actual_branch = true;
						CH++;
					}
					

					}
					if(tempct==1) break;


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
		
		cout<<"cycle-->" <<cycles<<"  pc "<<PC<<" Ic "<<IC<<" PC_TGT"<<PC_TGT<<endl;
		if(cycles==180) break;
	}

	TC = cycles;

}

void run_simulator_with_dataforwading(int IC){
	PC = 0;
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
							
							if(stall[(i+j)%5]!=0) break;
							else count = 1;
						}
			if(A[(i+j)%5]!=5) A[(i+j)%5]++;

			if(A[(i+j)%5]==0){
	
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
						else IPB2.rg2 = IPB4.rg1;}
					A[(i+j+1)%5] = -1;continue;
					}

				if((IPB2.r1==IPB4.r3 || IPB2.r2 == IPB4.r3) && cycles!=1 && cycles!=2){
					//M-->E data forwarding
					//cout<<IPB2.m.mem<<IPB2.m.alu<<endl;
					if(IPB2.r1==IPB4.r3) {IPB2.rg1 = IPB4.rg1;cout<<IPB2.r1<<"   "<<IPB4.r3<<endl;}
					else IPB2.rg2 = IPB4.rg1;
				}
				if((IPB2.r1==IPB3.r3 || IPB2.r2 == IPB3.r3) && cycles!=1){
					//E-->E data forwarding

					if(IPB3.m.mem[0]!='l'){
						
						if(IPB2.r1==IPB3.r3) IPB2.rg1 = IPB3.rg1;
						else IPB2.rg2 = IPB3.rg1;
						}
					else {
						stall[(i+j)%5] = 1;
						A[(i+j)%5] = 0;subtc=1;//break;
						}
					
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
								flush++;
								if(gsk3==0) actual_branch = false;
								else actual_branch = true;
							}

					else if(btake==false && gsk3==0){
						
						actual_branch = false;
					}
					else if(btake==true && gsk3==1){
						
						actual_branch = true;
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
		cout<<"cycle-->" <<cycles<<"  pc "<<PC<<" Ic "<<IC<<" PC_TGT"<<PC_TGT<<endl;
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
	
		counter++;
		if(counter>80) break;
	}

}



int main(int argc, char const *argv[])
{
	IMREQ = DMREQ = 0;
	int ic = load_memory("test.mc");
	int i;
	GRM = ic*4;
	ICM = ICH =DCH = DCM = 0;
	cout<<"instruction cache size : "<<endl;
	cin>>RAV;
	cout<<"data cache size : "<<endl;
	cin>>GSK;
	Instruction_cache = new cach[RAV];
	Data_cache = new cach[GSK];
	cach  obj;
	for(i=0;i<RAV;i++)Instruction_cache[i]=obj;
	for(i=0;i<GSK;i++)Data_cache[i]=obj;
	/*stringstream ss ;
	ss<<std::hex<<ic;
	string sg;
	ss>>sg;
	cout<<sg<<endl;*/
	
	//store_data(268435452,268439552,4,1);
	X[2] = 2147483632;
	X[3] = 268435456;
	mem * p = access_datamemory(64);
	PC = 68;
	//cout<<"working properly   "<<endl;
	TI = ic;
	run_simulator_with_stalling(ic);
	for(i=0;i<32;i++){
	cout<<"X["<<i<<"]  -->  " <<X[i]<<endl;
	//cout<<"X["<<i<<"]\t-->\t "<<X[i]<<"\n\n"<<endl;
	}
	//int mu = loading_data(1,268435460);
	//cout<<mu<<"  gsk"<<endl;
	cout<<"no of instruction_cache hits  --> "<<ICH<<endl;
	cout<<"no of instruction_cache misses --> "<<ICM<<endl;
	cout<<"no of data_cache hits --> "<<DCH<<endl;
	cout<<"no of data_cache misses --> "<<DCM<<endl;
	cout<<"raavan memory"<<IMREQ<<endl;
	int IM_access = ICH + ICM;
	int DM_access = DCH + DCM;
	cout<<"Instruction_cache hit rate --> "<<double(ICH)/double(IM_access)<<endl;
	cout<<"Instruction_cache miss rate --> "<<double(ICM)/double(IM_access)<<endl;
	cout<<"Data_cache hit rate --> "<<double(DCH)/double(DM_access)<<endl;
	cout<<"Data_cache miss rate --> "<<double(DCM)/double(DM_access)<<endl;
	
	


	return 0;
}
