#include <iostream>

#include <unordered_map>
#include "fileName.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
//#include<chrono>
#include <pthread.h>

#define N_KMER 25

#define N_WRITES 100     //写文件次数
#define N_READS 6*10000    //读取reads的次数
#define N_THREAD 5    //线程个数

#define READ_LENGTH (rand()%41+60)     //每次读取read的长度，60-100的随机数
//#define READ_LENGTH 20   //for test


using namespace std;
string newString;

//typedef list<string> STRINGLIST;



////从前向后显示list队列的全部元素
//void put_list(STRINGLIST list, string name)
//{
//    STRINGLIST::iterator plist;
//
//    cout << "The contents of " << name << " : ";
////    cout<<list.back();
//    for(plist = list.begin(); plist != list.end(); plist++)
//        cout << *plist << " ";
//    cout<<endl;
//}

class Node{
public:
    string pre; //kmer前一个节点的第一个字母
    string kmer;
    string suf; //kmer后一个节点的最后一个字母
    Node(string kmer){
        this->kmer = kmer;
    }


    bool isSemiBalanced(){
        return abs((int)(pre.length()-suf.length())) == 1;
    }

    bool isBalanced(){
        return pre.length() == suf.length();
    }

    void show(){
        cout<< "kmer:"<<kmer<<endl;

        cout<<"pre Node:";
        if(pre.length() == 0)
            cout<< " empty"<<endl;
        else{
            for (int i = 0; i < pre.length(); ++i)
                cout << pre[i] + kmer.substr(0, kmer.length() - 1) << " ";
            cout<<endl;
        }

        cout<<"next Node:";
        if(suf.length() == 0)
            cout<<"empty"<<endl;
        else{
            for (int i = 0; i < suf.length(); ++i)
                cout << kmer.substr(1)+suf[i] << " ";
            cout<<endl;
        }
        cout<<endl;

    }
};





class DeBruijnGraph{
public:
    unordered_map<string,Node> nodes;
    int nSemiBalanced;
    int nBalanced;
    int nNeither;
    Node *head,*tail;
//    mutex mtx;  //对nodes操作要同步
//    mutex add_node; //添加节点的时候不能被打扰

    bool containsNode(string kmer_str){
        int count = 0;
//        if (mtx.try_lock()) {   // only excute if currently not locked:
            count = nodes.count(kmer_str);
//            mtx.unlock();
//        }
//        lock_guard<mutex> mtx_locker(mutx);
        return (count > 0);
    }

    string preNode(string kmer_str){
        if(nodes.at(kmer_str).pre.length() == 1)
            return nodes.at(kmer_str).pre + kmer_str.substr(0,kmer_str.length()-1);
        else return NULL;
    }

    string sufNode(string kmer_str){
        if(nodes.at(kmer_str).suf.length() == 1)
            return kmer_str.substr(1)+nodes.at(kmer_str).suf;
        else return NULL;
    }

    //如果它自己并且它前一个，后一个相同，则表示有重叠部分，如果某一个节点没有前一节点或后一节点，则忽略它，只比较其他的
    bool hasOverlap(string kmer_str, string read,int index){
//        lock_guard<mutex> mtx_locker(mutx);
//        cout<<"检测重叠中..."<<endl;

        if(containsNode(kmer_str)){
            int preSame = 0,sufSame = 0;    //都设置为0表示都不相同

//            if (mtx.try_lock()) {   // only excute if currently not locked:
                if(nodes.at(kmer_str).pre.length() ==1 && index > 0){//检验前一个节点是否相同
                    string pre_str = preNode(kmer_str);
                    if(pre_str == read.substr(index-1,N_KMER)){
                        preSame = 1;
//                        cout<<kmer_str<<"的前一个节点相同"<<endl;
                    }
                } else if (nodes.at(kmer_str).pre.length() < 1 || index == 0){
                    preSame = 2;    //2表示不存在
//                    cout<<kmer_str<<"的前一个节点不存在"<<endl;
                }

                if(nodes.at(kmer_str).suf.length() == 1 && index < read.length()-N_KMER){//检验后一个节点是否相同
                    string suf_str = sufNode(kmer_str);
                    if(suf_str == read.substr(index+1,N_KMER)){
                        sufSame = 1;
//                        cout<<kmer_str<<"的后一个节点相同"<<endl;
                    }
                } else if (nodes.at(kmer_str).suf.length() < 1 || index == read.length()-N_KMER){
                    sufSame = 2;
//                    cout<<kmer_str<<"的后一个节点不存在"<<endl;
                }
//                mtx.unlock();
//            }

//            cout<<"preSame:"<<preSame<<"\t\t"<<"sufSame:"<<sufSame<<endl;
            return (preSame != 0 && sufSame !=0);   //只要不为0，就返回true
        } else
            return false;
    }

    void insertNode(string kmer,string pre,string suf){
//        lock_guard<mutex> mtx_locker(mutx);
        Node *kmer_node = new Node(kmer);
        kmer_node->pre = pre;
        kmer_node->suf = suf;
        kmer_node->suf = suf;
//        if (mtx.try_lock()) {   // only excute if currently not locked:
            nodes.insert(pair<string,Node>(kmer,*kmer_node));
//            mtx.unlock();
//        }

    }
    void addRead(string read){
        int str_len = read.length();
        int n_kmer=str_len-N_KMER;
        string kmer_str;
        string pre,suf;
        for (int i = 0; i <= n_kmer; ++i) {
            kmer_str = read.substr(i,N_KMER);
//            cout<<"kmer_str:"<<kmer_str<<endl;

//            if (add_node.try_lock()) {   // only excute if currently not locked:

                //如果已经有这个节点了，要检验是否是重叠部分
                if(containsNode(kmer_str)){
//                cout<<"kmer_str:"<<kmer_str<< "已存在"<<endl;
                    Node *node = &nodes.at(kmer_str);
                    if(i!=0 && node->isSemiBalanced()){
//                    cout<<"修改前：";
//                    node->show();
                        if(node->pre.length() == 0){
                            node->pre = read[i-1];
                        }
//                    cout<<"修改后："<<endl;
//                    node->show();
//                    nodes.at(kmer_str).show();    //证明通过指针是可以操纵hash表的


                    }
                    if(i != n_kmer && node->isSemiBalanced()){
//                    cout<<"修改前：";
//                    node->show();
                        if (node->suf.length() == 0){
                            node->suf = read[i+N_KMER];
                        }
//                    cout<<"修改后："<<endl;
//                    node->show();
                    }
                    if(hasOverlap(kmer_str,read,i)){
//                    cout<<"有重叠"<<endl;
                        //分析：前提：read的长度大于kmer的长度
                        //(1).如果一段read中只有第一个重叠，那么可以更改第一个重叠节点的suf，（更改）之后检测重叠，因为已经是最后一个了，直接continue，不用再检测最后一个了，接着就是插入新节点了
                        //(2).如果一段read中只有前面几个重叠，那么可以可以更改第一个重叠节点的suf，（更改）之后检测重叠，直接循环至最后一个，然后检测最后一个是否需要更改pre，再continue，插入新节点
                        //(3).如果一段read中只有后面几个重叠，那么可以可以更改第一个重叠节点的pre和suf，（更改）之后检测重叠，直接循环至最后一个，然后检测最后一个是否需要更改pre，（更改）再continue结束了
                        //(4).如果一段read中只有最后一个重叠，那么可以可以更改第一个重叠节点的pre，（更改）之后检测重叠，因为已经是最后一个了，直接continue结束
                        //(5).如果一点read中所有都重叠，那么那么可以更改第一个重叠节点的suf，（更改）之后检测重叠，直接循环至最后一个，然后检测最后一个是否需要更改pre，（更改）再continue结束了
                        //(6). 1、2和3、4可以取其一叠加
                        //如果已经是最后一个重叠的kmer了，那么就不能--i了，要不要又包含，又要检测重叠，又要--i，就出现死循环了
                        //
                        if(!containsNode(read.substr(i+1,N_KMER))){
                            continue;
                        }else{
                            while (containsNode(read.substr(i+1,N_KMER))){
                                i++;
                            }
                            i--;//还要检查最后一个是否非平衡，如果是，要修改,因为已经有重叠部分了，所以必定会执行while循坏，程序就不会出现死循环了
                            continue;
                        }

                    }else{
                    cout<<"还有重复节点，无法处理！"<<endl;
                    exit(1);
                        //已经有这个节点但是又没有重叠，这个概率较低，有比较复杂，暂不考虑
                    }
                }else{
                    //构造这个节点
                    if(i == 0) pre = "";
                    else pre = read[i-1];

                    if(i == n_kmer) suf = "";
                    else suf = read[i+N_KMER];
                    insertNode(kmer_str,pre,suf);
//                cout<<"添加"<<kmer_str<<endl;
                }

//                add_node.unlock();
//            }



//        Node *km1mer_nodeL,*km1mer
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
//            if(graph.count(kmerL_str) > 0){
//                cout<<graph[kmerL_str].size()<<endl;
//                graph[kmerL_str].push_back(kmerR_str);
//            } else{
//                list<string> nextNodeR_strlist;
//                nextNodeR_strlist.push_back(kmerR_str);
//                graph.insert(pair<string,list<string>>(kmerL_str,nextNodeR_strlist));
//                put_list(graph.at(kmerL_str),kmerL_str);
//            }


        }


    }

    //计算nSemiBalanced，nBalanced，nNeither，确认head和tail
    void affirmBalancedAndNode(){
        nSemiBalanced = nBalanced = nNeither = 0;
        unordered_map<string,Node>::iterator iter_hash;
        for(iter_hash = nodes.begin(); iter_hash != nodes.end(); iter_hash++)
        {
//            cout << i++<<":"<<iter_hash->first<<":";
//            iter_hash->second.show();
            if(iter_hash->second.isBalanced())
                nBalanced++;
            else if (iter_hash->second.isSemiBalanced()){
                if(iter_hash->second.pre.length() == iter_hash->second.suf.length()+1)
                    tail = &iter_hash->second;
                if(iter_hash->second.suf.length() == iter_hash->second.pre.length()+1)
                    head = &iter_hash->second;
                nSemiBalanced++;
            } else
                nNeither++;
        }
    }
//
//    void printGraph(){
//        int i = 0;
//        map<string,list<string>>::iterator iter_map;
//        for(iter_map = graph.begin(); iter_map != graph.end(); iter_map++)
//        {
//            cout << i++<<":";
//            put_list(graph[iter_map->first],iter_map->first);
//        }
//    }

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

    //如果非平衡节点个数为0，半平衡节点个数为2，则有欧拉路径
    bool hasEulerianPath(){
        return (nNeither == 0 && nSemiBalanced == 2);
    }

    //如果非平衡节点个数为0，半平衡节点个数为0，即全是平衡节点，则有欧拉循环
    bool hasEulerianCycle(){
        return (nNeither == 0 && nSemiBalanced == 0);
    }

    //如果有欧拉路径，或者有欧拉循环，则可以找到一条欧拉路径只遍历一遍所有节点
    bool isEulerian(){
        return (this->hasEulerianPath() || this->hasEulerianCycle());
    }

    //输出所有信息，便于测试查看
    void showAll(){
//        this->printNodes();
//        this->printGraph();
        cout<<"head:"<<endl;
        this->head->show();
        cout<<"tail:"<<endl;
        this->tail->show();
        cout<<"nSemiBalanced:"<<this->nSemiBalanced<<"\t\tnBalanced:"<<this->nBalanced<<"\t\tnNeither:"<<this->nNeither<<endl;
    }

//    void visit(string str){
//        while(tour_map[str].size()>0){
//
//            string next = tour_map[str].front();
//            tour_map[str].pop_front();
//            tour.push_back(next);
//            visit(next);
//        }
//
//    }

    //拼接字符串
    void initEulerianPath(){
        if(this->isEulerian()){
            string curKmer = head->kmer;
            newString+=curKmer;
            while (nodes.at(curKmer).suf.length()>0){
                newString += nodes.at(curKmer).suf;
                curKmer = sufNode(curKmer);
            }
            newString+=nodes.at(curKmer).suf;
        }
    }
};


//string tour_to_string(){
//    STRINGLIST::iterator plist;
//    string str = tour.front();
//    str = str.substr(0,str.length()-1);
//    for(plist = tour.begin(); plist != tour.end(); plist++)
//        str+=(*plist).substr((*plist).length()-1);
//    return str;
//}

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

class MyFile{
public:
    ifstream in;
    ofstream out;
    int file_length;

    MyFile(){
        this->readFile();
        if( in.is_open() )
        {
            in.clear();
            in.seekg( 0, ios::end );
            file_length = in.tellg();
        }
        cout << "原始文件长度：" << file_length << endl;
        this->closeRead();
    }

    void readFile(){
        in.open(FILENAME);
    }
    void closeRead(){
        if(in.is_open()) in.close();
    }

    void writeFile(){
        out.open(FILENAME,ios::out | std::ios::app);
    }
    void closeWrite(){
        if(out.is_open()) out.close();
    }

    //产生n_reads*n_length大小的字符串，并将其写入"TestString.txt"文档
    void writeStringToText(){
        printf("写文件开始...\n");
        for (int i = 0; i < N_WRITES; ++i) {
            if (out.is_open()){
                out<<generateString(100);
            }
        }
//    FILE *fp;
//    //判断文件是否打开失败
//    if ( (fp = fopen("TestString.txt", "a+")) == NULL ) {
//        puts("Fail to open file!");
//        exit(0);
//    }

        //将文件中的位置指针重新定位到文件开头
//    (void)fseek(fp, 0, SEEK_SET);

//    for (int i = 0; i < N_READS; ++i) {
//        fputs(generateString(READ_LENGTH),fp);
//        printf("%s\n",generateString(n_length));
//    }
//    fclose(fp);
        printf("写文件结束。\n");
    }

    //读字符串
    string readStringFromText(int pos,int len){
//        printf("读文件开始...\n");

//    FILE *fp;
        //判断文件是否打开失败
//    if ( (fp = fopen("TestString.txt", "r+")) == NULL ) {
//        puts("Fail to open file!");
//        exit(0);
//    }
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

        char *str;
//        printf("字符串长度为：%d\n",len);
        // 动态分配内存空间，如果失败就退出程序
        str = (char *)malloc( (len+1)*sizeof(char) );
        if(!str){
            printf("创建字符串数组失败！\n");
            exit(1);
        }
//    if(fgets(str,str_length+1,fp)!= NULL){
//        printf("%s\n",str);
//        fseek(fp,str_length,SEEK_CUR);
//    }else
//        cout<<"读取字符串失败"<<endl;
//    fclose(fp);

        pos = pos%(file_length-len+1);
//        cout<<"file_length:"<<file_length<<"\t\tpos:"<<pos<<endl;
        in.clear();
        in.seekg(pos);
        if(!in.eof()){
            in.getline(str, len+1);
        }
//    cout << "文件长度：" << file_length << endl;
//        printf("读文件结束。\n");

        return str;
    }
};

//int counter = 0;
//mutex mtx;
//void fun(int thread_num){
//    lock_guard<mutex> mtx_locker(mtx);
//    for (int i = 0; i < 100; ++i) {
//        ++counter;
//    }
//
////    this_thread::sleep_for(chrono::milliseconds(3000));
//    cout<<"thread "<<thread_num<<"finish counting:"<<counter<<endl;
//}

DeBruijnGraph *deBruijnGraph = new DeBruijnGraph();
mutex mutx;
int counter = 0;
MyFile *myFile = new MyFile();

void readAndInsertToGraph(){
    //开始读字符串构造de bruijn graph

    while (1) {
        string str;
        lock_guard<mutex> mtx_locker(mutx);
        str = myFile->readStringFromText(rand(),READ_LENGTH);
        counter++;
//        cout<<"原字符串:\t\t"<<str<<endl;
        deBruijnGraph->addRead(str);
        if(counter > N_READS){
            break;
        }
    }
}


int main() {
    clock_t start,end;
    start = clock();

    //用系统当前时间设置rand()随机序列种子，保证每次运行随机序列不一样
    srand((unsigned)(time(NULL)));


//    MyFile *myFile = new MyFile();




//    开始读字符串构造de bruijn graph
    myFile->readFile();
    thread threads[N_THREAD];
    for (int i = 0; i < N_THREAD; ++i) {
        threads[i] = thread(readAndInsertToGraph);
//        cout<<"原字符串:\t\t"<<str<<endl;
    }

    for (auto& t: threads) {
        t.join();
    }
    myFile->closeRead();


    deBruijnGraph->affirmBalancedAndNode();
    deBruijnGraph->showAll();
    deBruijnGraph->initEulerianPath();

    cout<<"拼接字符串:\t\t"<<newString<<endl;
    cout<<"拼接字符串长度:"<<newString.length()<<endl;





//    cout<<"文件长度："<<myFile->file_length<<endl;


////    写一个测试txt文档
//    myFile->writeFile();
//    myFile->writeStringToText();
////    myFile->out.flush();
//    myFile->closeWrite();





//    pthread_t threads[N_THREAD];
//    void * status;
//    int index[N_THREAD];
//    for (int i = 0; i < N_THREAD; ++i) {
//        index[i] = i;
//        int rc = pthread_create(&threads[i], NULL,
//                                reinterpret_cast<void *(*)(void *)>(readAndInsertToGraph), (void *)&(index[i]));
//        if (rc){
//            cout << "Error:无法创建线程," << rc << endl;
//            exit(-1);
//        }
//    }
//    for (int i = 0; i < N_THREAD; ++i) {
//
//        int rc = pthread_join(threads[i],&status);
//        if (rc){
//            cout << "Error:unable to create thread," << rc << endl;
//            exit(-1);
//        }
//    }

//    int count_number = 0;
//    int thread_number = 0;
//    while (thread_number < 60){
//        int num  = thread_number;
//        thread thrd(fun,num);
//        thread_number++;
//        thrd.detach();
//    }


    end = clock();
    cout<<"程序运行时间："<<(double)(end-start)/CLOCKS_PER_SEC<<endl;


    return 0;
}
