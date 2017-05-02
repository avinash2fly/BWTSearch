//
// Created by avina on 4/28/2017.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <string>
#include <bits/ios_base.h>
#include <ios>
#include <map>

#include <ctime>
#include <cstring>
//#include <chrono>

using namespace std;
//using namespace std::chrono;

typedef struct searchResult {
    unsigned int first;
    unsigned int last;
}searchRes;

typedef struct CharacterS {
    char s;
    unsigned int i;
}CharS;

typedef struct array {
    unsigned int a[95];
}LocalArray;

const unsigned int  BLOCKSIZE = 25000;
const unsigned int  LINESIZE = 380;
bool  WRITEINFILE = 1;
const unsigned short  FILESIZELIMIT =5200;
std::vector<LocalArray> localIndex;
//const unsigned int a[][];



void createIndexFile(const char *filename, const char *indexFile);


void readFileAndCreateIndex(const char *filename, const char *indexFile);
void writeIntoFile(ofstream& file, unsigned int a[]);

void printIndexFile(const char *file);

void fillCountArray(const char *indexFile);

searchResult searchPattern(const char *pattern1, const char *indexFile, const char *filename);

searchResult getFirstAndLast(unsigned char c);

signed int  getOccurences(unsigned char c, unsigned int location, const char *indexFile, const char *filename);

unsigned int getRank(ifstream &file, unsigned char c, unsigned int ref);

unsigned int getNewRankForC(unsigned char ch,ifstream &file, unsigned int location, unsigned int checkPoint, bool reverse);

unsigned int getOccurencestillIndex(unsigned char c, unsigned int location, const char *indexFile, const char *filename);

unsigned int getFILESIZE(const char *filename);

void interpretResult(searchRes result, const char *filename, const char *indexFile);

signed long iterateTillStart(unsigned int i, const char *indexFile, const char *filename);

char getCharAtPosition(unsigned int i, const char *filename);

string forwardSearch(unsigned int i, const char *indexFile, const char *filename);

CharS lookForwardForNextIndex(unsigned int index, const char *indexFile, const char *filename);

unsigned int getIndexInBwtFileByCharAndCount(char c, unsigned int pos, const char *filename,const char *indexFile);

unsigned int getStartIndexOfBlock(char c, unsigned int *count, unsigned int pos, const char *indexFile);

string strMakeBybackwardSearch(unsigned int i, const char *indexFile, const char *filename);

CharS makeStringByBackwardSearch(unsigned int i, const char *indexFile, const char *filename, string *str);

unsigned int *countArray;



int main(int argc, char** argv){
    if (argc < 4 || argc >6){
        //cerr << "Error, wrong number of arguments" << endl;
        return 0;
    }
    const char* filename = argv[1];
    const char* indexFile = argv[2];
    //creates index in file of bwtfile
    createIndexFile(filename,indexFile);
    //iterate each  pattern and find the result
    for(unsigned short i=3;i<argc;i++ ){
        const char* pattern = argv[i];
        searchRes res= searchPattern(pattern,indexFile,filename);
        interpretResult(res,filename,indexFile);
    }
    delete [] countArray;
    return 0;
}

/*
interpretResult will search backward and forward for each index then will concat both and print
*/
void interpretResult(searchRes result, const char *filename, const char *indexFile) {
    std::vector<unsigned int> resultsStartVector;
    string s1="",s2="";
    for(unsigned i = result.first;i<=result.last;i++){
        s1 = strMakeBybackwardSearch(i,indexFile,filename);
        s2 = forwardSearch(i,indexFile,filename);
        cout << s1+s2 << endl;
    }
}


//function used to search backward from a string
string strMakeBybackwardSearch(unsigned int i, const char *indexFile, const char *filename) {
    string str="";
    char c;
    do{
        CharS pair =  makeStringByBackwardSearch(i,indexFile,filename,&str);
        c=pair.s;
        i=pair.i;
        str.append(1,c);
    } while(c!='[');
    reverse(str.begin(),str.end());
    return str;
}

//this function will return the next index string for making the backward search complete
CharS makeStringByBackwardSearch(unsigned int i, const char *indexFile, const char *filename, string *str) {
    CharS pair;
    pair.s=getCharAtPosition(i,filename);//get char in F array
    unsigned int pos = getOccurences(pair.s,i,indexFile,filename);
    searchRes result = getFirstAndLast(pair.s);
    pair.i=result.first+pos;

    return pair;
}

//function used for forwardSearch
string forwardSearch(unsigned int index, const char *indexFile, const char *filename) {
    string str="";
    while(1){
        CharS pair = lookForwardForNextIndex(index,indexFile,filename);
        if(pair.s=='['){
            break;
        }
        str.append(1,pair.s);
        index=pair.i;
    }
    return str;
}

//function used to find the next index for forward search
CharS lookForwardForNextIndex(unsigned int index, const char *indexFile, const char *filename) {
    CharS pair;
    signed long count=0;
    unsigned int pos=0;
    char c;
    for(unsigned short i =32; i<127;i++){
        c=i;
        unsigned temp = count+countArray[i];
        if(!(temp>index)){
            count+=countArray[i];
            continue;
        }
        pos=index-count+1;
        break;
    }
    pair.s=c;
    pair.i = getIndexInBwtFileByCharAndCount(c,pos,filename,indexFile);
    return pair;
}

//function used to get index in bwt file by char and it's count
unsigned int getIndexInBwtFileByCharAndCount(char c, unsigned int pos, const char *filename,const char *indexFile) {
    ifstream bwtFile(filename,ios::binary);
    char ch;
    unsigned int count=0;
    unsigned int index = getStartIndexOfBlock(c,&count,pos,indexFile);
    index=index*BLOCKSIZE;
    bwtFile.seekg(index);
    bwtFile.get(ch);
    while(bwtFile.get(ch)){
        if(ch==c)
            count++;
        if(count==pos)
            break;
    }
    unsigned x =bwtFile.tellg();
    bwtFile.close();
    return (x-1);

}

//function used to calculate start index of the corresponding block, where data character probaby gonna be; by its position
unsigned int getStartIndexOfBlock(char c, unsigned int *count, unsigned int pos, const char *indexFile) {
    ifstream iFile;
    if(WRITEINFILE)
        iFile.open(indexFile, ios::binary);
    unsigned int blockNum=0,l_blockNum;
    unsigned int c_count=0,l_count=0;
    do{
        l_count=c_count;
        if(WRITEINFILE){
            unsigned int index = (blockNum*LINESIZE)+((unsigned int)c - 32)*4;
            iFile.seekg(index);
            iFile.read((char*)&c_count, sizeof(unsigned int));
        }
        else{
            LocalArray A;
            A=localIndex[blockNum];
            c_count = A.a[(unsigned int)c-32];
            // cout <<"check count " << c_count <<" " <<A.a[(unsigned int)c-32];
        }
        blockNum++;
    }while (!(c_count>=pos));
    *count=l_count;
    if(blockNum==1)
        return 0;
    return blockNum-2;
}

//function used to get index of character in bwt file
char getCharAtPosition(unsigned int i, const char *filename) {
    //ifstream in (filename,ios::binary);
    FILE *file;
    file=fopen(filename,"rb");
    //in.seekg(i);
    fseek(file,i,SEEK_SET);
    char c;
    fread(&c,1,1,file);
    //in.get(c);
    //in.close();
    fclose(file);
    return c;
}

//function used to search pattern and return the start and last index of first character in serach pattern
searchResult searchPattern(const char *pattern1, const char *indexFile, const char *filename) {
    unsigned int i = strlen(pattern1)-1;
    unsigned char c  = pattern1[i];
    searchResult result;
    result = getFirstAndLast(c);
    while((result.first <= result.last) && i){
        searchResult lastResult=result;
        c=pattern1[--i];
        result=getFirstAndLast(c);
        unsigned int previous = getOccurencestillIndex(c,lastResult.first-1,indexFile,filename);
        unsigned int total = getOccurencestillIndex(c,lastResult.last,indexFile,filename);
        unsigned int currentCount = total-previous;
        unsigned int first = result.first;
        if(!currentCount)
        {
            result.first=0;
            result.last=0;
        }
        result.first=first+previous;
        result.last=first+total-1;
    }
    return result;
}

//function used to get occurance of character till the index from the bwt file, searching from start of block
unsigned int getOccurencestillIndex(unsigned char c, unsigned int location, const char *indexFile, const char *filename) {
    ifstream indexF (indexFile,ios::binary);
    ifstream origF (filename,ios::binary | ios::ate);
    unsigned int ogFileSize = origF.tellg();//check whether to -1 or not for zero index
    unsigned  int blockNum = location/BLOCKSIZE;
    unsigned  int checkPoint1 = BLOCKSIZE*blockNum;
    unsigned int nearestCheckP = checkPoint1;
    unsigned int nearestRef = 0;
    nearestRef = blockNum*LINESIZE;
    unsigned int RankAtReference=getRank(indexF,c,nearestRef);
    unsigned int count = getNewRankForC(c, origF,location,nearestCheckP,0);
    signed long newRank = RankAtReference+count;
    return newRank;
}

//function used to get occurance of character till the index from the bwt file, searching from nearest block
//e.g block1 start at 0 and block2 start at 5, if index is 4, then it will start from block2
signed int getOccurences(unsigned char c, unsigned int location, const char *indexFile, const char *filename) {
    ifstream indexF (indexFile,ios::binary);
    ifstream origF (filename,ios::binary | ios::ate);
    unsigned int ogFileSize = origF.tellg();//check whether to -1 or not for zero index
    unsigned  int blockNum = location/BLOCKSIZE;
    unsigned  int checkPoint1 = BLOCKSIZE*blockNum;
    unsigned  int checkPoint2 = BLOCKSIZE*(blockNum+1);
    if (checkPoint2>ogFileSize)
        checkPoint2=ogFileSize-1;
    unsigned int nearestRef = 0;
    unsigned int nearestCheckP = checkPoint1;
    bool reverse=0;
    if((checkPoint2-location)<(location-checkPoint1)){
        nearestRef = (blockNum+1)*LINESIZE;
        reverse=1;
        nearestCheckP=checkPoint2;
    }
    else{
        nearestRef = blockNum*LINESIZE;
    }
    unsigned int RankAtReference=getRank(indexF,c,nearestRef);
    unsigned int count = getNewRankForC(c, origF,location,nearestCheckP,reverse);

    signed long newRank = (reverse)?RankAtReference-count-1:RankAtReference+count-1;
    if(newRank<0)
        return -1;
    return newRank;
}


//function used to get count of the character in bwt at that index
//e.g block1 start at 0 and block2 start at 5, if index is 4, then it will start from block2
unsigned int getNewRankForC(unsigned char ch, ifstream &file, unsigned int location, unsigned int checkPoint, bool reverse) {
    unsigned int track = checkPoint;
    unsigned int count=0;
    if(reverse) {
        file.seekg(track);
        if (track < location+1)
            return count;
        //track--;
    }
    else {
        file.seekg(++track);
        if (track > location)
            return count;
    }
    char c;
    //unsigned int count=0;
    while(file.get(c)){
        if(ch==c){
            count++;
        }
        if(reverse){
            track--;
            if(track<location+1)
                break;
            file.seekg(track);
        }
        else{
            track++;
            if(track>location)
                break;
            file.seekg(track);
        }

    }
    return count;
}

//gives rank from the indexfile, based on the index i.e block
unsigned int getRank(ifstream &file, unsigned char c, unsigned int ref) {
    if(!WRITEINFILE){
        unsigned int index = ref/LINESIZE;
        LocalArray A;
        A=localIndex[index];
        unsigned x = A.a[(unsigned int)c-32];
        return x;
    }
    unsigned  int c_loc = ref+(((unsigned int)c)-32)*4;
    file.seekg(c_loc);
    unsigned int z;
    file.read((char*)&z, sizeof(unsigned int));
    return z;

}

//returns first and last index of character from the index file
searchResult getFirstAndLast(unsigned char c) {
    searchResult result;
    unsigned int c_count = countArray[(unsigned int)c];
    unsigned int prev_count = 0;
    for(unsigned int i=32;i<(unsigned int)c;i++){
        if(!countArray[i])
            continue;
        prev_count += countArray[i];
    }
    result.first=prev_count;
    result.last=prev_count+c_count-1;
    return result;
}

//tempary function used for printing index file, but not used in program
void printIndexFile(const char *file) {
    ifstream in_file(file,ios::binary);
    unsigned int z;
    int count1=0;
    while(in_file.read((char*)&z, sizeof(unsigned int))){
        count1++;
        cout << z;
        if(count1==95){
            cout << endl;
            count1=0;
        }
    }
    cout << endl;
    for (unsigned int i=32 ; i<127;i++){
        cout << countArray[i];
    }
    in_file.close();
}

//functioon used to create index file
void createIndexFile(const char *filename, const char *indexFile) {
    unsigned int size = getFILESIZE(filename);
    if(size<FILESIZELIMIT)
        WRITEINFILE = 0;
    readFileAndCreateIndex(filename,indexFile);
    if(WRITEINFILE)
        fillCountArray(indexFile);
    else{
        LocalArray A = localIndex[localIndex.size()-1];
        countArray = new unsigned int[127];
        for(unsigned int index=32;index<127;index++){
            countArray[index]= A.a[index-32];
        }

    }
    // cout << "done" << endl;

}

//functiomn returns the filesize
unsigned int getFILESIZE(const char *filename) {
    ifstream in(filename,ios::binary | ios::ate);
    unsigned int size = in.tellg();
    in.close();
    return size;
}

//function used to fill count array
void fillCountArray(const char *indexFile) {
    ifstream in(indexFile,ios::binary);
    //as only last 380 bytes in index contain total count. 95(char)*4(bytes)=380
    in.seekg(-380,ios::end);
    //cout << in.tellg();
    unsigned int z;
    countArray = new unsigned int[127];
    unsigned int index=32;
    while(in.read((char*)&z, sizeof(unsigned int))){
        countArray[index]= z;
        index++;
    }
    in.close();
}

//function used to create index file form bwtfile
void readFileAndCreateIndex(const char *filename, const char *indexFile) {
    if(WRITEINFILE){
        fstream in(indexFile , ios::binary);
        if(in.good()){
            return ;
            in.close();
        }
    }
    unsigned int a[127]={0};
    ifstream input_file (filename,ios::binary);
    if(input_file.is_open()){
        char c;
        ofstream out_file;
        if(WRITEINFILE)
            out_file.open(indexFile , ios::binary);
        unsigned  int count=0;
        bool started=0;
        while (input_file.get(c)) {
            // cout << line;
            //for(auto c : line){
            if (a[(unsigned int) c] == 0) {
                a[(unsigned int) c] = 1;
            } else
                a[(unsigned int) c] = a[(unsigned int) c] + 1;
            //}
            if(count==BLOCKSIZE || !started){
                writeIntoFile(out_file,a);
                count=0;
            }
            if(!started){
                started=1;
            }
            count++;
        }
        if(count>0)
            writeIntoFile(out_file,a);
        if(WRITEINFILE)
            out_file.close();
    }
    input_file.close();

}

//function used to write in file; used basically to write in index file
void writeIntoFile(ofstream &file, unsigned int *a) {
    if(WRITEINFILE){
        for( unsigned int i = 32; i < 127; i = i + 1 ){
            file.write((char*)&a[i], sizeof(unsigned int));
        }
    }
    else{
        LocalArray localArray;
        for( unsigned int i = 32; i < 127; i = i + 1 ){
            localArray.a[i-32]=a[i];
        }
        localIndex.push_back(localArray);

    }

}
