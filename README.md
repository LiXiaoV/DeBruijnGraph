我预先生成好的TestString.txt，里面是10万长的字符串。直接编译运行debruijn.cpp运行就好了。

也可以先把主函数读文件的部分注释掉，debruijn.cpp有一个写文件的方法，先把cmake-build-debug里面的TestString.txt删掉（如果有，因为是append模式写），在main函数中把写文件的部分取消注释，先写一个10万字符长的TestString.txt用于测试。

还可以更改fileName.h里面的文件名，用其他测试文件用于测试。

debruijn.cpp中有用到多线程，编译时注意链接。
终端可直接运行命令： g++ debruijn.cpp -o debruijn -pthread -I./ -std=c++11