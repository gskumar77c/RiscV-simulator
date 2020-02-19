#include <iostream>
#include <bitset>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>
#include <string>
#include <cctype>
#include <algorithm>
#define N 177
using namespace std;
long int LINE_ADDRESS = 0;
long int MOMERY_ADDRESS = 0;
long int LWI_ADDRESS = 0;
class  instruction{
  public:
    std::string s1;
    std::string s2;
    std::bitset<7>opcode;
    std::bitset<7>func7;
    std::bitset<3>func3;
  instruction(){
      opcode = std::bitset<7>(string("0"));
      func3 = std::bitset<3>(string("0"));
      func7 = std::bitset<7>(string("0"));
    }
};

class dataclass{
  public :
  std::string label;
  std::string type;
  std::vector<string> value;
  int memoryaddress;
};

class labelclass{
  public :
  std::string label;
  std::string type;
  int address;
};

class branchTargetBuffer{
    public:
        int brnAddr;
        int tgtAddr;
};

vector<branchTargetBuffer> BTBufferVec;                 // branch target buffer vector

void addToBTBuffer(branchTargetBuffer obj)
{
    BTBufferVec.push_back(obj);

}

void writeBTBfr()                   // for printing the data of BTBuffer to a file in case needed.
{
  fstream f ;
  f.open("BTBuffer.txt",std::fstream::out);
  f.close();

    FILE *fp;
    fp=fopen("BTBuffer.txt","a");
    for(int i=0;i<BTBufferVec.size();i++)
    {
        int a=BTBufferVec[i].brnAddr;
        int b=BTBufferVec[i].tgtAddr;
        fprintf(fp,"0x%x    ",a);
        fprintf(fp,"0x%.8x\n",b);
    }
    fclose(fp);
}

int hash_key(string s){
  int n = s.length();
  int k=7;
  int i,j;
  long int m=0;
  for(i=0;i<n;i++){
    j = (int)s[i];
    m = m+((int)pow(k,i))*j;
  }

  j = (m%N);
  return j;

}
bool S_type(instruction* ,string );

void fill (instruction ** I,int N1){
  fstream myfile;
  int i,kjh;
  string j;
  string line,str,s;
  myfile.open("instruction_set.txt");
  if (myfile.is_open())
  {
    while ( getline (myfile,line) )
    { std::stringstream s(line);
      s>>str;
      int n = hash_key(str);
      for(i=0; ;i++){
        if(I[(n+i)%N1]==NULL) {instruction * ist = new instruction;
                   ist->s1 = str;s>>str;
                   ist->s2 = str;s>>str;
                   ist->opcode = std::bitset<7>(string(str));s>>j;
                   j = j.substr(2,j.length());
                   stringstream gsk ;
                   gsk<<std::hex<<j;
                   gsk>>kjh;
                  ist->func3 = std::bitset<3>(kjh);s>>j;
                  j = j.substr(2,j.length());
                   stringstream gsk2 ;
                   gsk2<<std::hex<<j;
                   gsk2>>kjh;
                   ist->func7 = std::bitset<7>(kjh);
                   I[(n+i)%N1] = ist;
                   break;
                    }
      }

    }
    myfile.close();
  }

  else cout << "Unable to open file"; 

}

void print_file(bitset<32>code){
  FILE *fp;
  fp=fopen("test.mc","a");
  int hex = code.to_ulong();
  int hex2 = LINE_ADDRESS;
  fprintf(fp,"0x%x    ",hex2);
  fprintf(fp,"0x%.8x\n",hex);
  fclose(fp);
  /*ofstream outfile;
  outfile.open("test.txt", std::ios_base::app);
    outfile << hex << code.to_ulong() << endl;
    outfile.close();*/
}



////////////-----------R_type function----------------------/////////////////////////

void R_type(instruction* i,string s){ 
   //  cout << "funct7 is: "  << i->func7  << endl;
   //  cout << "oppcode is: "  << i->opcode << endl; 
    // cout << "funct3 is: "  << i->func3 << endl;
    int a1,a2,a3,j;
    for(j=0;j< s.length() ; j++){
      if(s[j] == 'x'){
                j++;
        a1 = s[j]-'0';
        j++;
        if(s[j] != ' ' && s[j] != 'x' && s[j] != ','){  a1 = a1*10 + (s[j]-'0');}
        break;
      }
    }
    for(;j< s.length() ; j++){
      if(s[j] == 'x'){
                j++;
        a2 = s[j]-'0';
        j++;
        if(s[j] != ' ' && s[j] != 'x' && s[j] != ','){
          a2 = a2*10 + (s[j]-'0');
        }
        break;
      }
    }
    for(;j< s.length() ; j++){
      if(s[j] == 'x'){
                j++;
        a3 = s[j]-'0';
        j++;
        if(s[j] != ' ' && s[j] != 'x' && s[j] != ',' && s[j] != '\0'){
          a3 = a3*10 + (s[j]-'0');
        }
        break;
      }
    }
    bitset<32> rd(a1*128);
    bitset<32> rs1(a2*32768);
    bitset<32> rs2(a3*1048576);
    bitset<32> f1 = rd|rs1 ;
    f1 = f1|rs2 ;
    int inter =i->opcode.to_ulong();
    bitset<32> opp(inter);
    f1 = f1|opp;
    int flag7 =i->func7.to_ulong();
    bitset<32> f7(flag7*33554432);
    
    //cout << "flag7 is: "  << f7 << endl;
    f1 = f1|f7;
        int flag3 =i->func3.to_ulong();
    bitset<32> f3(flag3*4096);
    f1 = f1|f3;

    //cout << "f1 is :"<<f1[31]<<f1[30]<<f1[29]<<f1[28]<<"  "<<f1[27]<<f1[26]<<f1[25]<<f1[24]<<"  "<<f1[23]<<f1[22]<<f1[21]<<f1[20]<<"  "<<f1[19]<<f1[18]<<f1[17]<<f1[16]<<"  "<<f1[15]<<f1[14]<<f1[13]<<f1[12]<<"  "<<f1[11]<<f1[10]<<f1[9]<<f1[8]<<"  "<<f1[7]<<f1[6]<<f1[5]<<f1[4]<<"  "<<f1[3]<<f1[2]<<f1[1]<<f1[0] << endl;
    
    
    print_file(f1);   
}
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

///////////////////--------U_type----------------//////////////////


void U_type(instruction* i,string s){

  int j=0;
  bitset<20>bin[2];   //bin[1] == rd, bin[0] ==imm
  int k = 1;
  while(s[j] != '\0'){
    if(s[j] == 'x' || s[j] == ',' || s[j] == ' ' || s[j] == '(' || s[j] == ')' || isdigit(s[j])){
      int num=0;
      int flag=0;
      while(isdigit(s[j])){
        num = num*10 + ((int)s[j] - (int)'0');
        flag = 1;
        j++;
      }
      if(flag == 1){
          bin[k] = bitset<20>(num);
          k--;
      }
        
      j++;
    }
    else  j++;
  }
  bitset<32>mCode;
  for(j=31;j>=0;j--){
    if(j>11)
      mCode[j] = bin[0][j-12];
    else if(j>6)
      mCode[j] = bin[1][j-7];
    else
      mCode[j] = (i->opcode)[j];
  }
    
  print_file(mCode);
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$


/////////////////------------I_type function--------------///////////////////////////

void I_type(instruction* i,vector<dataclass>D,string s){

  int j=0;
  bitset<12>bin[3];    //bin[2] == rd, bin[1] == rs1, bin[0] == imm or rs2
  int k = 2;
  int count = 0;
  int minus=0;
  int bracket = 0;
  while(s[j] != '\0'){
    if(s[j] == 'x' || s[j] == ',' || s[j] == ' ' || s[j] == '(' || s[j] == ')' || s[j] == '-' || isdigit(s[j]) || bracket == 1){
      int num=0;
      int flag=0;
      if(s[j] == '-'){
        minus = 1;
        j++;
        continue;
      }
      else if(s[j] == '('){
        bracket = 1;
        j++;
        continue;
      }
      else if(s[j] == ')'){
        bracket = 0;
        j++;
        continue;
      }
      while(isdigit(s[j])){
        num = num*10 + ((int)s[j] - (int)'0');
        flag = 1;
        j++;
      }
      if(bracket == 1 && s[j] == 's' && s[j+1] == 'p'){
        num = 2;
        j += 2;
      }
      if (minus == 1){
        num = -num;
        minus = 0;
      }
      if(s[j] == 'x') count++;
      if(flag == 1){
        if(count == 1 && k == 1)
          bin[0] = bitset<12>(num);
        else{
          bin[k] = bitset<12>(num);
          k--;
        }
      }
        
      j++;
    }
    else if(count == 1){
      string ILabel = s.substr(j);
      ILabel.erase(std::remove_if(ILabel.begin(), ILabel.end(), ::isspace), ILabel.end());
      
      int ILoop = 0;
      while(s[j] != ' ' && s[j] != '\0'  && s[j] != '\n')
         j++;
      for(ILoop=0;ILoop<=D.size();ILoop++){
        if(ILabel == D[ILoop].label){
          instruction *ist = new instruction;
          ist->opcode = bitset<7>(string("0010111"));
          size_t pos = s.find(ILabel);
          string srn7 = s.substr (0,pos);
          srn7 += "65536";
          U_type(ist,srn7);
          LINE_ADDRESS += 4;
          LWI_ADDRESS = LWI_ADDRESS + 4;
          bin[1] = bin[2];
          long long int imm = -((LINE_ADDRESS - 4) - D[ILoop].memoryaddress);
          bin[0] = bitset<12>(imm);
          //bin[0] = bitset<12>(string("0"));
          break;
        }
      }
    }
    else  j++;
  }
  bitset<32>mCode;
  for(j=31;j>=0;j--){
    if(j>24 && i->func7 != bitset<7>(string("0")))
      mCode[j] = (i->func7)[j-25];
    else if(j>19)
      mCode[j] = bin[0][j-20];
    else if(j>14)
      mCode[j] = bin[1][j-15];
    else if(j>11)
      mCode[j] = (i->func3)[j-12];
    else if(j>6)
      mCode[j] = bin[2][j-7];
    else
      mCode[j] = (i->opcode)[j];
  }
  print_file(mCode);
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$


void SB_type(instruction * i,vector<labelclass> D,string s)
{
    //'in' is the effective immediate value with respect to label S
    //cout <<  "s string is: "<< s   << endl;
      //sample code

    branchTargetBuffer rvn;
    int b1,b2,j;
    for(j=0;j< s.length() ; j++){
      if(s[j] == 'x'){
                j++;
        b1 = s[j]-'0';
        j++;
        if(s[j] != ' ' && s[j] != 'x' && s[j] != ',')
        { 
          b1 = b1*10 + (s[j]-'0');
        }
        break;
      }
    }
    for(;j< s.length() ; j++){
      if(s[j] == 'x'){
                j++;
        b2 = s[j]-'0';
        j++;
        if(s[j] != ' ' && s[j] != 'x' && s[j] != ','){
          b2 = b2*10 + (s[j++]-'0');
        }
        break;
      }
    }
    string s3 = "";
    for( ; j< s.length() ; j++)
    {
              if(s[j] != ' '  && s[j] != ','  && s[j] != '\0')
        {
          s3 = s3 + s[j]; 
        } 
    }
    //cout << "s3 is : "  << s3   <<  " " << s3.length() << endl;


    int in, p=0,temp= -50;
    for( ; p< D.size() ; p++)
    {
      if(D[p].label == s3)
      {
        temp = p;
        break;
      }
    }
    if(temp == -50)
    {
      cout << "Label is not found(ERROR)  " <<s3<< endl;
    }
    else
    {
      
    
    
        //cout << "size of vector:" << D.size()  << "  "  << D[p].label  << endl;
        //cout << "p:" << p  << endl;
        in = D[p].address + LWI_ADDRESS - LINE_ADDRESS ;
        rvn.tgtAddr = in;
        rvn.brnAddr = LINE_ADDRESS;
        addToBTBuffer(rvn);
        //cout<<"effective immediate value wrt to pc is "<<in<<endl;
        
        //cout << "b1 is:"  << b1 << endl;
        //cout << "b2 is:"  << b2 << endl;

        bitset<32> Rs1(b1);
        Rs1 = Rs1 << 15;
        bitset<32> Rs2(b2);
        Rs2 = Rs2<<20;

        bitset<32> fs = Rs1|Rs2 ;

        int inter1 =i->opcode.to_ulong();
        bitset<32> opp1(inter1);
        int inter2 =i->func3.to_ulong();
        bitset<32> f3(inter2*4096);
        in = in/2;
        bitset<12> imm(in);
        
        fs = fs|opp1 ;
        fs = fs|f3;

  

          fs[7] = imm[10];  // 11
          fs[8] = imm[0];    // 1
          fs[9] = imm[1];    // 2
          fs[10] = imm[2];    // 3
          fs[11] = imm[3];    // 4

            fs[25] = imm[4];        // 5
            fs[26] = imm[5];         // 6
            fs[27] = imm[6];         // 7
          fs[28] = imm[7];         // 8
          fs[29] = imm[8];        // 9
          fs[30] = imm[9];        // 10
          fs[31] = imm[11];      //12 

        //cout   << "machine code is: "  << fs  << endl; 
        bitset<32> f1 = fs;
        //cout << "machine code of SB-format is :"<<f1[31]<<f1[30]<<f1[29]<<f1[28]<<"  "<<f1[27]<<f1[26]<<f1[25]<<f1[24]<<"  "<<f1[23]<<f1[22]<<f1[21]<<f1[20]<<"  "<<f1[19]<<f1[18]<<f1[17]<<f1[16]<<"  "<<f1[15]<<f1[14]<<f1[13]<<f1[12]<<"  "<<f1[11]<<f1[10]<<f1[9]<<f1[8]<<"  "<<f1[7]<<f1[6]<<f1[5]<<f1[4]<<"  "<<f1[3]<<f1[2]<<f1[1]<<f1[0] << endl;
        print_file(f1); 
    }
    
}

void UJ_type(instruction * i,vector<labelclass>D,string s){
    //'in' is the effective immediate value with respect to label S
    //sample code
    branchTargetBuffer ram;
    int b1,j;
    for(j=0;j< s.length() ; j++){
      if(s[j] == 'x'){
                j++;
        b1 = s[j]-'0';
        j++;
        if(s[j] != ' ' && s[j] != 'x' && s[j] != ',')
        { 
          b1 = b1*10 + (s[j++]-'0');
        }
        break;
      }
    }

    string s3 = "";
    for( ; j< s.length() ; j++)
    {
              if(s[j] != ' '  && s[j] != ','  && s[j] != '\0')
        {
          s3 = s3 + s[j]; 
        } 
    }
    //cout << "s3 is : "  << s3   <<  " " << s3.length() << endl;


    int in, p=0,temp= -50;
    for( ; p< D.size() ; p++)
    {
      if(D[p].label == s3)
      {
        temp = p;
        break;
      }
    }
    if(temp == -50)
    {
      cout << "Label is not found(ERROR)  "<<s3 << endl;
    }
    else
    {
      
    
    
        //cout << "size of vector:" << D.size()  << "  "  << D[p].label  << endl;
        //cout << "p:" << p  << endl;
        in = D[p].address +LWI_ADDRESS - LINE_ADDRESS ;
        ram.tgtAddr = in;
        ram.brnAddr = LINE_ADDRESS;
        addToBTBuffer(ram);
        //cout<<"effective immediate value wrt to pc is "<<in<<endl;
        
        //cout << "b1 is:"  << b1 << endl;
        
                bitset<32> rd(b1);
        rd = rd << 7;
        
                int inter1 =i->opcode.to_ulong();
        bitset<32> opp1(inter1);
        
        bitset<32> fs = opp1|rd ; 
        in = in/2;
        bitset<20> imm(in);

                
          fs[12] = imm[11];    // 12
                    fs[13] = imm[12];  // 13
          fs[14] = imm[13];    // 14
          fs[15] = imm[14];    // 15
          fs[16] = imm[15];    // 16
          fs[17] = imm[16];  // 17
          fs[18] = imm[17];    // 18
          fs[19] = imm[18];    // 19
          
          fs[20] = imm[10];    // 11
          
          fs[21] = imm[0];        // 1
                    fs[22] = imm[1];        //  2
          fs[23] = imm[2];         // 3
            fs[24] = imm[3];          // 4
            fs[25] = imm[4];        // 5
            fs[26] = imm[5];         // 6
            fs[27] = imm[6];         // 7
          fs[28] = imm[7];         // 8
          fs[29] = imm[8];        // 9
          fs[30] = imm[9];        // 10
          
          fs[31] = imm[19];      //20 

         
        bitset<32> f1 = fs;
        //cout << "jal machine code is :"<<f1[31]<<f1[30]<<f1[29]<<f1[28]<<"  "<<f1[27]<<f1[26]<<f1[25]<<f1[24]<<"  "<<f1[23]<<f1[22]<<f1[21]<<f1[20]<<"  "<<f1[19]<<f1[18]<<f1[17]<<f1[16]<<"  "<<f1[15]<<f1[14]<<f1[13]<<f1[12]<<"  "<<f1[11]<<f1[10]<<f1[9]<<f1[8]<<"  "<<f1[7]<<f1[6]<<f1[5]<<f1[4]<<"  "<<f1[3]<<f1[2]<<f1[1]<<f1[0] << endl;
        print_file(f1); 
    }
    
}





void instruction_fetch(instruction ** I,vector<dataclass> &D,vector<labelclass>&raavan,int N1){
  fstream myfile;
  int i,j;
  string srn7 ;
  int count =0;
  string line,str,s;
  myfile.open("input.asm");
  if (myfile.is_open())
     {
      while ( 1 )
       {  
        myfile>>s;
        if(s==".data"){count = 1;getline (myfile,line);continue;}
        if(myfile.eof()){ /*cout<<s<<endl;*/break;}
        if(count ==1){
          if(s==".text"){count = 0;getline (myfile,line);continue;}
          if(s[0]=='#') {getline (myfile,line);continue;}
          dataclass c ;
          string srn = s.substr(0,s.length()-1);
          c.label = srn;
          myfile>>s;
          if(s[0]=='#') {getline(myfile,line);myfile>>s;}
          c.type = s;
          getline(myfile,line);
          int x = 0;
          while(line[x] != '\0' && line[x] != '#'){
          string sr;
          while(line[x] == ' ')
            x++;
          if(line[x] == '#')
            break;
          while(line[x] != ' ' && line[x] != '\0' && line[x] != '#'){
            sr.push_back(line[x]);
            x++;
          }
          c.value.push_back(sr);
          if(line[x] == '#')
            break;
          x++;
          }
          c.memoryaddress = MOMERY_ADDRESS;
          MOMERY_ADDRESS = MOMERY_ADDRESS + 4;
          D.push_back(c);
        }
        else{
        
        if(s[s.length()-1]==':') {continue;}
        if(!getline (myfile,line)) break; 
        if(s[0]=='#') continue;         
          int n = hash_key(s);
          instruction * ist = NULL;
          for(i=0; ;i++){
            if(I[(n+i)%N1]!=NULL){ if(I[(n+i)%N1]->s1 == s) {ist = I[(n+i)%N1];break;} 
                          cout<<I[(n+i)%N1]->s1<<endl;
                        }
            else break;
          }

          if(ist!=NULL){
            stringstream gsk7(line);
            getline(gsk7,srn7,'#');
            //cout<<"\ninstruction found---"<<s<<" "<<srn7<<endl;
            
            if(ist->s2=="R") {R_type(ist,srn7);LINE_ADDRESS = LINE_ADDRESS+4;}
            else if(ist->s2=="I") {I_type(ist,D,srn7);LINE_ADDRESS = LINE_ADDRESS + 4;}
            else if(ist->s2=="S"){ bool bs = S_type(ist,srn7);LINE_ADDRESS = LINE_ADDRESS + 4;}
            else if(ist->s2=="SB") {SB_type(ist,raavan,srn7);LINE_ADDRESS = LINE_ADDRESS + 4;}
            else if(ist->s2=="UJ"){ UJ_type(ist,raavan,srn7);LINE_ADDRESS = LINE_ADDRESS + 4;}
            else if(ist->s2=="U") {U_type(ist,srn7);LINE_ADDRESS = LINE_ADDRESS + 4;}
            
          }
          else {/*cout<<"\ninstruction not found---"<<s<<" "<<srn7<<endl;LINE_ADDRESS = LINE_ADDRESS+4;*/}

       }}
       myfile.close();
     }

    else cout << "Unable to open file"; 

}


void find_lineaddress(instruction ** I,vector<labelclass> &raavan,int N1){
  fstream myfile;
  int i,j;
  string srn7 ;
  int count =0;
  string line,str,s;
  myfile.open("input.asm");
  if (myfile.is_open())
     {
      while ( 1 )
       {  
        myfile>>s;
        if(s==".data"){count = 1;getline (myfile,line);continue;}
        if(myfile.eof()){ /*cout<<s<<endl;*/
                    if(s[s.length()-1]==':') {
                      labelclass l;
                      l.label = s.substr(0,s.length()-1);
                      l.address = LINE_ADDRESS;
                      raavan.push_back(l);
                      }
                      break;}
        if(count ==1){
          if(s==".text"){count = 0;getline (myfile,line);continue;}
          if(s[0]=='#') {getline (myfile,line);continue;}
          string srn = s.substr(0,s.length()-1);
          myfile>>s;
          if(s[0]=='#') {getline(myfile,line);myfile>>s;}
          myfile>>s;
        }
        else{
        
        if(s[s.length()-1]==':') {
                      labelclass l;
                      l.label = s.substr(0,s.length()-1);
                      l.address = LINE_ADDRESS;
                      raavan.push_back(l);
                      continue;}
        if(!getline (myfile,line)) break; 
        if(s[0]=='#') continue;         
          int n = hash_key(s);
          instruction * ist = NULL;
          for(i=0; ;i++){
            if(I[(n+i)%N1]!=NULL){ if(I[(n+i)%N1]->s1 == s) {ist = I[(n+i)%N1];break;} 
                          
                        }
            else break;
          }

          if(ist!=NULL){
            stringstream gsk7(line);
            getline(gsk7,srn7,'#');
            //cout<<"\ninstruction found---"<<s<<" "<<srn7<<endl;
            LINE_ADDRESS = LINE_ADDRESS+4;
          }
          else {cout<<"\ninstruction not found---"<<s<<" "<<srn7<<endl;LINE_ADDRESS = LINE_ADDRESS+4;}

       }}
       myfile.close();
     }

    else cout << "Unable to open file"; 

}

void print_data(std::vector<dataclass> &D){
  FILE *fp;
  fp=fopen("test.mc","a");
  string str = "0x10000000";
  stringstream ss;
  ss<<std::hex<<str;
  unsigned int l;
  ss>>l;

  int i;
  for(i=0;i<D.size();i++){
  int v;
    for(v = 0; v < D[i].value.size(); v++){
      fprintf(fp,"0x%x ",l);
      unsigned int m;
      if(D[i].value[v][0] == '0' && D[i].value[v][1]=='x'){
          stringstream ss1;
          ss1<<std::hex<<D[i].value[v];
          ss1>>m;
      }
      else{
          stringstream sp(D[i].value[v]);
          sp>>m;
      }
      fprintf(fp,"0x%.8x\n",m);
      l = l+4;
    }
  }
  fclose(fp);
}

int main()
{
  fstream f ;
  f.open("test.mc",std::fstream::out);
  f.close();

  int i,j=0;
  instruction * I[N] = {NULL};
  std::vector<dataclass> D;
  fill (I,N);
  vector<labelclass> raavan;
  find_lineaddress(I,raavan,N);
  LINE_ADDRESS = 0;
  instruction_fetch(I,D,raavan,N);
  if(D.size()!=0){
  FILE *fp;
  fp=fopen("test.mc","a");
  char strs[] = "$$_MEMORY_DATA_$$";
  fprintf(fp, "\n%s\n\n", strs);
  fclose(fp);
  print_data(D);}

  int gsk3 = hash_key("sub");
  writeBTBfr();
  //cout<<I[gsk3]->opcode<<endl;
  /*
  stringstream ss;
  ss<<std::hex<<LINE_ADDRESS;
  string s = ss.str();
  cout<<"0x"<< s <<endl;
  */

  return 0;
}



















/////////////////------------lengthy_S_type function--------------///////////////////////////

bitset <32> binAdd(bitset <32> bin1 , bitset <32> bin2)
{    
    int i, carry=0 , b1, b2, sum=0;
    bitset <32> binSum;
    for(i=0; i<32; i++)
    {
        b1 = bin1[i];
        b2 = bin2[i];
        sum = b1+b2+carry;
        if(sum == 2 || sum == 3)
        {
            carry = 1;
            if(sum == 2)
            {
                binSum[i] = 0;
            }
            else
            {
                binSum[i] = 1;
            }
        }
        
        else
        {
            binSum[i] = sum;    
            carry = 0;
        }
    }                
    
    return binSum; 
}



bitset <32> dec2bin(unsigned int dec)
{
    int i ,j=0;
     bitset<32> bin;
                
    for(i=dec ; i>0; i=i/2)
    {
        bin[j] = i%2;
        j++;
    }
    
    return bin;    
}




bool S_type(instruction* obj,string s)
{
    bool res=true;
    int rs2,rs1,imm=0,i,negFlag=0,spFlag=0;
    string st="";
    //-------------------------------------------------------
    //RS2
    for(i=0;i<s.length();i++)
    {
        if(s[i]=='x')
            break;
        else if( (s[i]=='s') & ((i+1)<s.length()) & (s[i+1]=='p') )
        {
            spFlag=1;
            break;
        }
    }
    if(spFlag==1)
    {
        rs2=2;
        i+=2;
    }
    else
    {
        i++;
        if(s[i] >=48 & s[i] <=57  )
        {
            rs2=s[i]-48;
            if(s[i+1] >=48 & s[i+1] <=57 )
            {
                rs2= (rs2*10) ;
                rs2= rs2 + (s[i+1]-48); 
                i++;
            }
            i++;
        }
        else
        {
            return false;           // wrong syntax
        }
    }
    if(rs2 >31 || rs2<0)
        return false;
    
    bitset <32> bin_rs2;
    bin_rs2 = dec2bin(rs2);
    //-------------------------------------------------------
    //IMM
    for(;i<s.length();i++)
    {
        if(s[i] ==' ' || s[i]==','  )
        {
            continue;
        }
        else
        {
            break;
        }
    }
    
    for(;i<s.length();i++)
    {
        if(s[i]==' ' || s[i]==',' || s[i]=='(' || s[i]=='x' )
        {
            break;
        }
        else
            st=st+s[i];
    }

    bitset <32> bin_imm;
    // converting st to imm
    int j=0;
    if(st[0]=='-')
    {
        negFlag=1;
        j=1;
    }
    for(; j<st.length();j++)
    {
        imm=imm*10+(st[j]-48);
    }

    if(negFlag==1)
    {
        if(imm>2048)
            return false;

        bitset <32> bin2;
        bin2 = dec2bin(imm);
        //2S


        bitset <32> bin2_1s;
        for(int l=0; l<32; l++)
        {
            if(bin2[l] == 0)
            {
                bin2_1s[l] = 1;
            }
            else
            {
                bin2_1s[l] = 0;
            }
        }

        
        bin_imm = binAdd(bin2_1s , ((bitset <32>) 1));
        imm*=-1;
    }

    else
    {
        if(imm>4095)
            return false;
        else if(imm>2047)
        {
            imm=imm-4096;
            bitset <32> bin2;
            bin2 = dec2bin(imm);
            //2S


            bitset <32> bin2_1s;
            for(int l=0; l<32; l++)
            {
                if(bin2[l] == 0)
                {
                    bin2_1s[l] = 1;
                }
                else
                {
                    bin2_1s[l] = 0;
                }
            }

            
            bin_imm = binAdd(bin2_1s , ((bitset <32>) 1));
        }
        else
            bin_imm = dec2bin(imm);
    }
    //-------------------------------------------------------
    // RS1 
    spFlag=0;
    for(;i<s.length();i++)
    {
        if(s[i]=='x')
            break;
        else if( (s[i]=='s') & ((i+1)<s.length()) & (s[i+1]=='p') )
        {
            spFlag=1;
            break;
        }
    }
    if(spFlag==1)
    {
        rs1=2;
        i+=2;
    }
    else
    {
        i++;
        if(s[i] >=48 & s[i] <=57  )
        {
            rs1=s[i]-48;
            if(s[i+1] >=48 & s[i+1] <=57 )
            {
                rs1= (rs1*10) ;
                rs1= rs1 + (s[i+1]-48); 
                i++;
            }
            i++;
        }
        else
        {
            return false;           // wrong syntax
        }
    }
    if(rs1 >31 || rs1<0)
        return false;

    bitset <32> bin_rs1(rs1);
    //bin_rs1 = dec2bin(rs1);
    int l;
    bitset <32> inst;
    for( l=31;l>=25;l--)
    {
        inst[l]=bin_imm[l-20];      // first 7 bits of imm
    }
    for(l=24 ; l>=20;l--)
    {
        inst[l]=bin_rs2[l-20];      // for rs2
    }
    for(l=19 ; l>=15;l--)
    {
        inst[l]=bin_rs1[l-15];      // for rs1
    }
    for(l=14;l>=12;l--)
    {
        inst[l]=(obj->func3)[l-12];              // func3 from instruction class object;
    }
    for(l=11;l>=7;l--)
    {
        inst[l]=bin_imm[l-7];       // last 5 bits of imm
    }
    for(l=6;l>=0;l--)
    {
        inst[l]=(obj->opcode)[l];              // opcode from instruction class object;
    }
    print_file(inst); 
    return true;
}