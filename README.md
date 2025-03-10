# Summer2021-No.56 优化Linux内核中的复杂头文件依赖

#### 介绍
https://gitee.com/openeuler-competition/summer-2021/issues/I3EIOX

#### 软件架构

This tool is used to delete the superfluous header files in the linux kernel.The tool runs in three steps

1. For a specific source file A, check all the header files that it has included;
2. Check all the functions and macro that A has used , then count the numbers of macros/functions A has used in each header file.If some header files was included but used rarely,than it can be deleted and the the declarations can be moved to the other file. For example, A has included a header file B. B has tons of declarations but only one declaration was used.Then we will consider to delete it.

3. After we have confirm the header file we want to delete(say B).We will check whether we can move the only one declaration(say C) to another file(say D).To achieve the goal,we should check all the files in the linux kernel that has used the C.If D was included by every files who has used C.Then we can move the declaration C from B to D.Moreover,the source file A won't need to include the header file B.Thus we have reduced the superfluous header file.


#### 安装教程

1.  git clone https://gitee.com/openeuler-competition/summer2021-56.git
2.  if you want to draw the dependcy graph, install graphiz
- on ubuntu/Debian, use sudo apt install graphviz
- on Fedora project use sudo yum install graphviz
3.  compile the project.cpp code with the command: g++ project.cpp -std=c++17 -o project -O2 -fopenmp

#### 使用说明

there are several modes for running this tool
1.  input a path to the source file that you want to analyze,than the tool will draw the include dependency graph for you
2.  input nothing, then the tool will draw the DAG which indicates all the include relationship in the directory.
3.  input two path. One is the path to the source file,the other is the one you want to check whether it has been included by the first file.For example,./project a.c b.h will check whether a.c has included b.h directly or indirectly
4.  input a path to the source file and the tool will tell you whether there is a superfluous header file can be deleted
5.  input -p and a path to the source file and the tool will return all the functions and macros that it has used
6.  input -p and a path to the header file and the tool will return all the functions and macros that it has declared
7.  use -il to specify the path to the linux kernel, then the tool will search the header file in the kernel
8.  use -i to specify the path to the header file, then the tool will find the header file in the path you have specified
9.  use -o and a filename to redirect the output from terminal to the file you have specified
10. input -move and two path to the file and it will check whether the second file can be removed from the first one
11. input -mover and two path to the file and it will function recursively to check nwhether the second file can be removed from the first one

#### test

1. cd sample
2. compile the code with g++ project.cpp -std=c++17 -o project -O2 -fopenmp
3. check the DAG of the whole directory with ./project
4. check the including tree of a specific file A with ./project A
5. check whether A includes B with ./project A B
6. check the function/macro used with ./project -p A

#### autotest

1. cd sample
2. chmod +x check.sh
3. run ./check.sh to determine whether the tool is working properly(ignore other output, such as setrlimit returned result = -1)

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request


#### 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
