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
//#include <chrono>

using namespace std;
//using namespace std::chrono;
const unsigned int  ARRAY_SIZE = 128;

typedef struct searchResult {
    unsigned int first;
    unsigned int last;
}searchRes;

typedef struct CharacterS {
    char s;
    unsigned int i;
}CharS;

typedef struct array {
    unsigned int a[ARRAY_SIZE];
}LocalArray;

const unsigned int  START_INDEX = 0;
//const unsigned int  BLOCKSIZE = 25600;
//const unsigned int  BLOCKSIZE = 1024;
const unsigned int  BLOCKSIZE = 5;
const unsigned int  LINESIZE = 512;
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

unsigned int getNewRankForC(unsigned char ch,const char *filename, unsigned int location, unsigned int checkPoint, bool reverse);

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

signed int getStartingStringIndex(unsigned int i, const char *indexFile, const char *filename);
void interpretResultNew(unsigned int i, const char *filename, const char *indexFile);

unsigned int *countArray;



int main(int argc, char** argv){
    if (argc < 4 || argc >6){
        cerr << "Error, wrong number of arguments" << endl;
        return 0;
    }
    int main_Result[500];
    int temp_Result[500];
    memset(main_Result,-1, sizeof(main_Result));
    memset(main_Result,-1, sizeof(main_Result));
    const char* filename = argv[1];
    const char* indexFile = argv[2];
    createIndexFile(filename,indexFile);
    searchRes res  = searchPattern(argv[3],indexFile,filename);
    unsigned short j =0;
    vector<unsigned int> result_set;
    //int j=0;
    for(unsigned int i=res.first;i<=res.last;i++){
        result_set.push_back(getStartingStringIndex(i,indexFile,filename));
    }

    for(unsigned short i=4;i<argc;i++ ){
        sort(result_set.begin(),result_set.end());
        vector<unsigned int> temp_set;
        const char* pattern = argv[i];
        res  = searchPattern(pattern,indexFile,filename);
        for(unsigned int j=res.first;j<=res.last;j++){
            unsigned int index = getStartingStringIndex(j,indexFile,filename);
            if(binary_search(result_set.begin(),result_set.end(),index))
                temp_set.push_back(index);
        }
        result_set=temp_set;
    }
    for(unsigned int i=0;i<result_set.size();i++)
        interpretResultNew(result_set[i],filename,indexFile);
    delete [] countArray;
    return 0;
}

void interpretResult(searchRes result, const char *filename, const char *indexFile) {
    string s1="",s2="";
    for(unsigned i = result.first;i<=result.last;i++){
        s1 = strMakeBybackwardSearch(i,indexFile,filename);
        s2 = forwardSearch(i,indexFile,filename);
        string s = s1+s2;
        if(s=="")
            continue;
        cout << s << endl;
    }
}

void interpretResultNew(unsigned int i, const char *filename, const char *indexFile) {
    string s1="",s2="";
        //s1 = strMakeBybackwardSearch(i,indexFile,filename);
        s2 = forwardSearch(i,indexFile,filename);
        string s = s1+s2;
        if(s!="")
            cout << s << endl;
}



string strMakeBybackwardSearch(unsigned int i, const char *indexFile, const char *filename) {
    string str="";
    char c;
    int count=0;
    bool bracesFound= false;
    do{
        CharS pair =  makeStringByBackwardSearch(i,indexFile,filename,&str);
        c=pair.s;
        i=pair.i;
        str.append(1,c);
        //count++;
        if(pair.s==']'){
            bracesFound=true;
        }
    } while(c!='[');
    if(!bracesFound)
        return "";
    reverse(str.begin(),str.end());
    return str;
}



CharS makeStringByBackwardSearch(unsigned int i, const char *indexFile, const char *filename, string *str) {
    CharS pair;

    //auto finish = std::chrono::high_resolution_clock::now();

    pair.s=getCharAtPosition(i,filename);//get char in F array

    //auto start = std::chrono::high_resolution_clock::now();
    //std::chrono::duration<double> elapsed = start-finish;
    //cout<<"getCharAtPosition i : "<<elapsed.count()<<endl;

    unsigned int pos = getOccurencestillIndex(pair.s,i,indexFile,filename);

    //finish = std::chrono::high_resolution_clock::now();
    //elapsed = finish-start;
    //cout<<"getOccurences i : "<<elapsed.count()<<endl;

    searchRes result = getFirstAndLast(pair.s);

    //start = std::chrono::high_resolution_clock::now();
    //elapsed = start-finish;
    //cout<<"getFirstAndLast i : "<<elapsed.count()<<endl;

    pair.i=result.first+pos-1;

    return pair;
}

string forwardSearch(unsigned int index, const char *indexFile, const char *filename) {
    string str="[";
    int count =0;
    CharS pair = lookForwardForNextIndex(index,indexFile,filename);
    index=pair.i;
    while(1){
        //count++;
        CharS pair = lookForwardForNextIndex(index,indexFile,filename);
        if(pair.s=='['){
            break;
        }
//        if(pair.s==']'){
//            return "";
//        }
        str.append(1,pair.s);
        index=pair.i;
    }
    return str;
}

CharS lookForwardForNextIndex(unsigned int index, const char *indexFile, const char *filename) {
    CharS pair;
    signed long count=0;
    unsigned int pos=0;
    char c;
    for(unsigned short i =0; i<ARRAY_SIZE;i++){
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

unsigned int getIndexInBwtFileByCharAndCount(char c, unsigned int pos, const char *filename,const char *indexFile) {
    ifstream bwtFile(filename,ios::binary);
    //FILE* bwtFile;
    //fopen(filename,"r");
    char ch;
    unsigned int count=0;
    unsigned int index = getStartIndexOfBlock(c,&count,pos,indexFile);
    index=index*BLOCKSIZE;
    bwtFile.seekg(index);
    bwtFile.read(&ch,1);
    //fseek(bwtFile,index,SEEK_SET);
    //std::vector<char> buffer (BLOCKSIZE,0);

    //bwtFile.read(buffer.data(),buffer.size());
    //for(unsigned int i =0;i<BLOCKSIZE;i++){
    while(bwtFile.read(&ch,1)){
        if(ch==c)
            count++;
        if(count==pos)
            break;
    }
    unsigned x =bwtFile.tellg();
    // unsigned x =ftell(bwtFile);
    bwtFile.close();
    // fclose(bwtFile);
    return (x-1);

}

/*unsigned int getStartIndexOfBlock(char c, unsigned int *count, unsigned int pos, const char *indexFile) {
    ifstream iFile;
    if(WRITEINFILE)
        iFile.open(indexFile, ios::binary);
    unsigned int blockNum=0,l_blockNum;
    unsigned int c_count=0,l_count=0;
    do{
        l_count=c_count;
        if(WRITEINFILE){
            unsigned int index = (blockNum*LINESIZE)+((unsigned int)c - START_INDEX)*4;
            iFile.seekg(index);
            iFile.read((char*)&c_count, sizeof(unsigned int));
        }
        else{
            LocalArray A;
            A=localIndex[blockNum];
            c_count = A.a[(unsigned int)c-START_INDEX];
            // cout <<"check count " << c_count <<" " <<A.a[(unsigned int)c-START_INDEX];
        }
        blockNum++;
    }while (!(c_count>=pos));
    *count=l_count;
    if(blockNum==1)
        return 0;
    return blockNum-2;
}*/

unsigned int getStartIndexOfBlock(char c, unsigned int *count, unsigned int pos, const char *indexFile) {
    ifstream iFile;
    signed int end=0;
    signed int start=0;
    if(WRITEINFILE){
        iFile.open(indexFile, ios::binary | ios::ate);
        end=(iFile.tellg())/LINESIZE;
    }
    else{
        end= localIndex.size();
    }

    unsigned int blockNum=0,l_blockNum;
    unsigned int c_count=0,n_count=0;

    while (start <= end) {
        int blockNum = (start + end) / 2;
        if(WRITEINFILE){
            unsigned int index = (blockNum*LINESIZE)+((unsigned int)c - START_INDEX)*4;
            iFile.seekg(index);
            iFile.read((char*)&c_count, sizeof(unsigned int));
            index = ((blockNum+1)*LINESIZE)+((unsigned int)c - START_INDEX)*4;
            iFile.seekg(index);
            iFile.read((char*)&n_count, sizeof(unsigned int));
        }
        else{
            LocalArray A;
            A=localIndex[blockNum];
            c_count = A.a[(unsigned int)c-START_INDEX];
            A=localIndex[blockNum+1];
            n_count = A.a[(unsigned int)c-START_INDEX];
            // cout <<"check count " << c_count <<" " <<A.a[(unsigned int)c-START_INDEX];
        }

        if ( (n_count>= pos && c_count<pos) || (end==start)){
            *count=c_count;
            return blockNum;
        }
        else if (c_count >= pos)
            end = blockNum;
        else
            start = blockNum +1;
    }
}

signed long iterateTillStart(unsigned int i, const char *indexFile, const char *filename) {
    char start='[';
    char current;
    bool isCloseFound=0;
    unsigned int last;
    do{
        current = getCharAtPosition(i,filename);
        if(current==']')
            isCloseFound=1;
        unsigned int count  = getOccurencestillIndex(current,i,indexFile,filename);
        searchRes result=getFirstAndLast(current);
        last=i;
        i = result.first+(count-1);
    }while(current!=start);
    if(isCloseFound)
        return last;
    return -1;
}

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

searchResult searchPattern(const char *pattern1, const char *indexFile, const char *filename) {
    unsigned int i = strlen(pattern1)-1;
    unsigned char c  = pattern1[i];
    searchResult result;
    result = getFirstAndLast(c);
    //cout <<endl << result.first <<" " << result.last;
    while((result.first <= result.last) && i){
        searchResult lastResult=result;
        c=pattern1[--i];
        result=getFirstAndLast(c);
        unsigned int previous = getOccurencestillIndex(c,lastResult.first-1,indexFile,filename);
        unsigned int total = getOccurencestillIndex(c,lastResult.last,indexFile,filename);
        unsigned int currentCount = total-previous;
        unsigned int first = result.first;
        if(currentCount)
        {
            result.first=first+previous;
            result.last=first+total-1;
        }
        else{
            result.first=0;
            result.last=0;
        }
    }
    return result;
}

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
    unsigned int count = getNewRankForC(c, filename,location,nearestCheckP,0);
    signed long newRank = RankAtReference+count;
    return newRank;
}

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
    // cout << endl << RankAtReference;
    unsigned int count = getNewRankForC(c, filename,location,nearestCheckP,reverse);

    signed long newRank = (reverse)?RankAtReference-count:RankAtReference+count-1;
    if(newRank<0)
        return -1;
    return newRank;
}

unsigned int getNewRankForC(unsigned char ch, const char *filename, unsigned int location, unsigned int checkPoint, bool reverse) {
    unsigned int track = checkPoint;
    //FILE* file;
    ifstream file(filename , ios::binary);
    //fopen(file,"rb");
    unsigned int count=0;
    if(reverse) {
        //fseek(file,track,SEEK_SET);
        file.seekg(track);
        if (track < location+1)
            return count;
        //track--;
    }
    else {
        //fseek(file,++track,SEEK_SET);
        file.seekg(++track);
        if (track > location)
            return count;
    }
    char c;
    //unsigned int count=0;
    std::vector<char> buffer (BLOCKSIZE,0);

    file.read(buffer.data(),buffer.size());
    file.close();
    for(unsigned int i =0;i<BLOCKSIZE;i++){
        c=buffer[i];
        if(ch==c){
            count++;
        }
        if(reverse){
            track--;
            if(track<location+1)
                break;
            //file.seekg(track);
        }
        else{
            track++;
            if(track>location)
                break;
            //file.seekg(track);
        }

    }
    return count;
}

unsigned int getRank(ifstream &file, unsigned char c, unsigned int ref) {
    if(!WRITEINFILE){
        unsigned int index = ref/LINESIZE;
        LocalArray A;
        A=localIndex[index];
        unsigned x = A.a[(unsigned int)c-START_INDEX];
        return x;
    }
    unsigned  int c_loc = ref+(((unsigned int)c)-START_INDEX)*4;
    file.seekg(c_loc);
    unsigned int z;
    file.read((char*)&z, sizeof(unsigned int));
    return z;

}

searchResult getFirstAndLast(unsigned char c) {
    searchResult result;
    unsigned int c_count = countArray[(unsigned int)c];
    unsigned int prev_count = 0;
    for(unsigned int i=START_INDEX;i<(unsigned int)c;i++){
        if(!countArray[i])
            continue;
        prev_count += countArray[i];
    }
    result.first=prev_count;
    result.last=prev_count+c_count-1;
    return result;
}

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
    for (unsigned int i=START_INDEX ; i<ARRAY_SIZE;i++){
        cout << countArray[i];
    }
    in_file.close();
}

void createIndexFile(const char *filename, const char *indexFile) {
    unsigned int size = getFILESIZE(filename);
    if(size<FILESIZELIMIT)
        WRITEINFILE = 0;
    readFileAndCreateIndex(filename,indexFile);
    if(WRITEINFILE)
        fillCountArray(indexFile);
    else{
        LocalArray A = localIndex[localIndex.size()-1];
        countArray = new unsigned int[ARRAY_SIZE];
        for(unsigned int index=START_INDEX;index<ARRAY_SIZE;index++){
            countArray[index]= A.a[index-START_INDEX];
        }

    }
    // cout << "done" << endl;

}

unsigned int getFILESIZE(const char *filename) {
    ifstream in(filename,ios::binary | ios::ate);
    unsigned int size = in.tellg();
    in.close();
    return size;
}

void fillCountArray(const char *indexFile) {
    ifstream in(indexFile,ios::binary);
    in.seekg(-512,ios::end);
    //cout << in.tellg();
    unsigned int z;
    countArray = new unsigned int[ARRAY_SIZE];
    unsigned int index=START_INDEX;
    while(in.read((char*)&z, sizeof(unsigned int))){
        countArray[index]= z;
        index++;
    }
    //cout << in.tellg();
    in.close();
}

void readFileAndCreateIndex(const char *filename, const char *indexFile) {
    if(WRITEINFILE){
        ifstream in(indexFile , ios::binary);
        if(in.good()){
            //   cout << "file found";
            return ;
            in.close();
        }
    }
    unsigned int a[ARRAY_SIZE]={0};
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

void writeIntoFile(ofstream &file, unsigned int *a) {
    if(WRITEINFILE){
        for( unsigned int i = START_INDEX; i < ARRAY_SIZE; i = i + 1 ){
            file.write((char*)&a[i], sizeof(unsigned int));
        }
    }
    else{
        LocalArray localArray;
        for( unsigned int i = START_INDEX; i < ARRAY_SIZE; i = i + 1 ){
            localArray.a[i-START_INDEX]=a[i];
            /*if(a[i]>0){
                cout <<  (char)i << "->" << a[i] << ":";
            }*/
        }
        //cout << endl;
        localIndex.push_back(localArray);

    }

}

signed int getStartingStringIndex(unsigned int i, const char *indexFile, const char *filename) {
    string str="";
    char c;
    int count=0;
    bool bracesFound= false;
    do{
        CharS pair =  makeStringByBackwardSearch(i,indexFile,filename,&str);
        c=pair.s;
        i=pair.i;
        // str.append(1,c);
        //count++;
        if(pair.s==']'){
            bracesFound=true;
        }
    } while(c!='[');
    if(!bracesFound)
        return -1;
    //reverse(str.begin(),str.end());
    return i;
}
