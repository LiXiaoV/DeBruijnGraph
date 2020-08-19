#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <string>
#include <map>
#include <unordered_map>
#include <list>

#define N_KMER 30
#define N_KM1MER (N_KMER-1)

#define N_READS 5     //读取reads的次数
#define READ_LENGTH (rand()%41+60)     //每次读取read的长度，60-100的随机数
//#define READ_LENGTH 20   //for test
using namespace std;
list<string> tour;
map<string,list<string>> tour_map;
typedef list<string> STRINGLIST;


class Node{
public:
    string km1mer;
    int nin;
    int nout;
    Node(string km1mer){
        this->km1mer = km1mer;
        this->nin = 0;
        this->nout = 0;
    }

    void show(){
        cout<< "km1mer:"<<km1mer<<"\tnin:"<<nin<<"\tnout:"<<nout<<endl;
    }

    bool isSemiBalanced(){
        return abs(nin-nout) == 1;
    }

    bool isBalanced(){
        return nin == nout;
    }


};

//从前向后显示list队列的全部元素
void put_list(STRINGLIST list, string name)
{
    STRINGLIST::iterator plist;

    cout << "The contents of " << name << " : ";
//    cout<<list.back();
    for(plist = list.begin(); plist != list.end(); plist++)
        cout << *plist << " ";
    cout<<endl;
}



class DeBruijnGraph{
public:
    map<string,list<string>> graph;
    unordered_map<string,Node> nodes;
    int nSemiBalanced;
    int nBalanced;
    int nNeither;
    Node *head,*tail;

    DeBruijnGraph(string str){
        int str_len = str.length();
        int n_kmer=str_len-N_KMER+1;
        string kmer_str,km1merL_str,km1merR_str;
        for (int i = 0; i < n_kmer; ++i) {
            kmer_str = str.substr(i,N_KMER);
            km1merL_str = kmer_str.substr(0,N_KM1MER);
            cout<<"nodeL:"<<km1merL_str<<endl;
            km1merR_str = kmer_str.substr(1);
            cout<<"nodeR:"<<km1merR_str<<endl;

//        Node *km1mer_nodeL,*km1mer_nodeR;

            if(nodes.count(km1merL_str) > 0){

                cout<<"nodeL:"<<km1merL_str<< "已存在"<<endl;
//            km1mer_nodeL= &nodes.at(km1merL_str);
//            km1mer_nodeL->show();
                nodes.at(km1merL_str).nout++;
//            km1mer_nodeL->show();
            }
            else{
                Node *km1mer_nodeL = new Node(km1merL_str);
                km1mer_nodeL->nout++;
                nodes.insert(pair<string,Node>(km1merL_str,*km1mer_nodeL));
                cout<<"添加"<<km1merL_str<<endl;
            }

            if(nodes.count(km1merR_str) > 0){

                cout<<"nodeR:"<<km1merR_str<< "已存在"<<endl;
//            km1mer_nodeR = &nodes.at(km1merR_str);
//            km1mer_nodeR->show();
                nodes.at(km1merR_str).nin++;
//            km1mer_nodeR->show();
            }
            else{
                Node *km1mer_nodeR = new Node(km1merR_str);
                km1mer_nodeR->nin++;
                nodes.insert(pair<string,Node>(km1merR_str,*km1mer_nodeR));
                cout<<"添加"<<km1merR_str<<endl;
            }

//        iter = nodes.find(km1merL_str);
//        if(iter != nodes.end()){
//            iter->second.nout++;
//            iter->second.show();
//        } else{
//            Node nodeL(km1merL_str);
//            nodes.insert(pair<string,Node>(km1merL_str,nodeL));
//        }
//
//        iter = nodes.find(km1merR_str);
//        if(iter != nodes.end()){
//            iter->second.nin++;
//            iter->second.show();
//        } else{
//            Node nodeR(km1merR_str);
//            nodes.insert(pair<string,Node>(km1merR_str,nodeR));
//        }
//问题：指针++赋值无法成功
//        km1mer_nodeL->nout++;
//        km1mer_nodeR->nin++;
//        km1mer_nodeL->show();
//        km1mer_nodeR->show();

//        cout<<"graph["<<km1merL_str<<"]:"<<graph.count(km1merL_str)<<endl;
            if(graph.count(km1merL_str) > 0){
                cout<<graph[km1merL_str].size()<<endl;
                graph[km1merL_str].push_back(km1merR_str);
            } else{
                list<string> nextNodeR_strlist;
                nextNodeR_strlist.push_back(km1merR_str);
                graph.insert(pair<string,list<string>>(km1merL_str,nextNodeR_strlist));
                put_list(graph.at(km1merL_str),km1merL_str);
            }


        }

        nSemiBalanced = nBalanced = nNeither = 0;
        unordered_map<string,Node>::iterator iter_hash;
        for(iter_hash = nodes.begin(); iter_hash != nodes.end(); iter_hash++)
        {
//            cout << i++<<":"<<iter_hash->first<<":";
//            iter_hash->second.show();
            if(iter_hash->second.isBalanced())
                nBalanced++;
            else if (iter_hash->second.isSemiBalanced()){
                if(iter_hash->second.nin == iter_hash->second.nout+1)
                    tail = &iter_hash->second;
                if(iter_hash->second.nout == iter_hash->second.nin+1)
                    head = &iter_hash->second;
                nSemiBalanced++;
            } else
                nNeither++;
        }

    }

    void printGraph(){
        int i = 0;
        map<string,list<string>>::iterator iter_map;
        for(iter_map = graph.begin(); iter_map != graph.end(); iter_map++)
        {
            cout << i++<<":";
            put_list(graph[iter_map->first],iter_map->first);
        }
    }

    //打印nodes，这里打印出来的顺序于插入顺序并不相同，确切的说是完全无序的
    void printNodes(){
        int i = 0;
        unordered_map<string,Node>::iterator iter_hash;
        for(iter_hash = nodes.begin(); iter_hash != nodes.end(); iter_hash++)
        {
            cout << i++<<":"<<iter_hash->first<<":";
            iter_hash->second.show();
        }
    }

    bool hasEulerianPath(){
        return (nNeither == 0 && nSemiBalanced == 2);
    }

    bool hasEulerianCycle(){
        return (nNeither == 0 && nSemiBalanced == 0);
    }

    bool isEulerian(){
        return (this->hasEulerianPath() || this->hasEulerianCycle());
    }

    void showAll(){
        this->printNodes();
        this->printGraph();
        cout<<"head:";
        this->head->show();
        cout<<"tail:";
        this->tail->show();
        cout<<"nSemiBalanced:"<<this->nSemiBalanced<<"\t\tnBalanced:"<<this->nBalanced<<"\t\tnNeither:"<<this->nNeither<<endl;
    }

    void visit(string str){
        while(tour_map[str].size()>0){

            string next = tour_map[str].front();
            tour_map[str].pop_front();
            tour.push_back(next);
            visit(next);
        }

    }

    void initEulerianPath(){
        if(this->isEulerian()){
            tour_map = this->graph;
            tour.push_back(head->km1mer);
            visit(head->km1mer);
        }
    }
};



//产生number大小的A~Z的字符串
char* generateString(int number)
{
    char alphabet[26];
    for(int i = 0;i < 26; i++){
        alphabet[i] = 'A'+i;
//        printf("字母：%c\n",alphabet[i]);
    }
    char *str;
    // 动态分配内存空间，如果失败就退出程序
    str = (char *)malloc( number*sizeof(char) );
    if(!str){
        printf("创建字符串数组失败！\n");
        exit(1);
    }
    for (int j = 0; j < number; ++j) {
        str[j] = alphabet[rand()%26];
    }
    return str;
}

//产生n_reads*n_length大小的字符串，并将其写入"TestString.txt"文档
void writeStringToText(){
    printf("写文件开始...\n");
    FILE *fp;
    //判断文件是否打开失败
    if ( (fp = fopen("TestString.txt", "a+")) == NULL ) {
        puts("Fail to open file!");
        exit(0);
    }

    //将文件中的位置指针重新定位到文件开头
    (void)fseek(fp, 0, SEEK_SET);

    for (int i = 0; i < N_READS; ++i) {
        fputs(generateString(READ_LENGTH),fp);
//        printf("%s\n",generateString(n_length));
    }
    fclose(fp);
    printf("写文件结束。\n");
}

//读字符串
string readStringFromText(){
    printf("读文件开始...\n");

    FILE *fp;
    //判断文件是否打开失败
    if ( (fp = fopen("TestString.txt", "r+")) == NULL ) {
        puts("Fail to open file!");
        exit(0);
    }
//    for (int i = 0; i < N_READS; ++i) {
//        char *str;
//        int str_length = READ_LENGTH;
//
//        printf("字符串长度为：%d\n",str_length);
//        // 动态分配内存空间，如果失败就退出程序
//        str = (char *)malloc( (str_length+1)*sizeof(char) );
//        if(!str){
//            printf("创建字符串数组失败！\n");
//            exit(1);
//        }
//        if(fgets(str,str_length,fp)!= NULL)
//            printf("%s\n",str);
//        else
//            break;
//    }
    int str_length = READ_LENGTH;
    char *str;
    printf("字符串长度为：%d\n",str_length);
    // 动态分配内存空间，如果失败就退出程序
    str = (char *)malloc( (str_length+1)*sizeof(char) );
    if(!str){
        printf("创建字符串数组失败！\n");
        exit(1);
    }
    if(fgets(str,str_length+1,fp)!= NULL){
        printf("%s\n",str);
        fseek(fp,str_length,SEEK_CUR);
    }else
        cout<<"读取字符串失败"<<endl;
    fclose(fp);

    printf("读文件结束。\n");
    return str;
}

string tour_to_string(){
    STRINGLIST::iterator plist;
    string str = tour.front();
    str = str.substr(0,str.length()-1);
    for(plist = tour.begin(); plist != tour.end(); plist++)
        str+=(*plist).substr((*plist).length()-1);
    return str;
}


int main() {
    //用系统当前时间设置rand()随机序列种子，保证每次运行随机序列不一样
    srand((unsigned)(time(NULL)));

//    writeStringToText();
    string str = readStringFromText();


    DeBruijnGraph *deBruijnGraph = new DeBruijnGraph(str);
    deBruijnGraph->showAll();
    deBruijnGraph->initEulerianPath();
    put_list(tour,"tour");
    cout<<"tour's size:"<<tour.size()<<endl;

    cout<<"拼接字符串长度："<<tour_to_string().length()<<endl;

    cout<<"原字符串:\t\t"<<str<<endl;
    cout<<"拼接字符串:\t\t"<<tour_to_string()<<endl;
    cout<<"原字符串是否与拼接字符串相等："<<(str == tour_to_string())<<endl;
//    list<string> mylist;
//    string s1 = "aaa";
//    string s2 = "bbb";
//    mylist.push_front(s1);
//    mylist.push_front(s2);
//    map<string,list<string>> mymap;
//
//    mymap.insert(pair<string,list<string>>(s1,mylist));
//
//    put_list(mymap[s1],"mymap[aaa]");
//    cout<< mymap[s1].size()<<endl;
//    cout<<mymap[s1].front()<<endl;









    return 0;
}
