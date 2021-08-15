# Summer2021-No.56 优化Linux内核中的复杂头文件依赖

#### Description
https://gitee.com/openeuler-competition/summer-2021/issues/I3EIOX

#### Software Architecture
This tool is used to delete the superfluous header files in the linux kernel.The tool runs in three steps

1. For a specific source file A, check all the header files that it has included;
2. Check all the functions and macro that A has used , then count the numbers of macros/functions A has used in each header file.If some header files was included but used rarely,than it can be deleted and the the declarations can be moved to the other file. For example,A has included a header file B. B has tons of declarations but only one declaration was used.Then we will consider to delete it.

3. After we have confirm the header file we want to delete(say B).We will check whether we can move the only one declaration(say C) to another file(say D).To achieve the goal,we should check all the files in the linux kernel that has used the C.If D was included by every files who has used C.Then we can move the declaration C from B to D.Moreover,the source file A won't need to include the header file B.Thus we have reduced the superfluous header file.

#### Installation

1.  git clone https://gitee.com/openeuler-competition/summer2021-56.git
2.  compile the project.cpp code with the command: g++ project.cpp -std=c++17 -o project -O2 -fopenmp

#### Instructions

there are three mode for running this tool
1.  input a path to the source file that you want to analyze,than the tool will draw the include dependency graph for you
2.  input nothing,then the tool will draw the DAG which indicates all the include relationship in the directory.
3.  input two path.One is the path to the source file,the other is the one you want to check whether it has been included by the first file.For example,./project a.cpp b.hpp will check whether a.cpp has included b.hpp directly or indirectly
4.  input a path to the source file and the tool will tell you whether there is a superfluous header file can be deleted(to do)
5.  input a path to the source file and the tool will return all the functions and macros that it has used(to do)

#### test

1. cd sample
2. compile the code with g++ project.cpp -std=c++17 -o project -O2 -fopenmp
3. if you want to draw the DAG of the whole directory use ./test
4. if you want to draw a including tree of a specific file A  use ./test A
5. if you want to determine whether A includes B use ./test A B

#### Contribution

1.  Fork the repository
2.  Create Feat_xxx branch
3.  Commit your code
4.  Create Pull Request


#### Gitee Feature

1.  You can use Readme\_XXX.md to support different languages, such as Readme\_en.md, Readme\_zh.md
2.  Gitee blog [blog.gitee.com](https://blog.gitee.com)
3.  Explore open source project [https://gitee.com/explore](https://gitee.com/explore)
4.  The most valuable open source project [GVP](https://gitee.com/gvp)
5.  The manual of Gitee [https://gitee.com/help](https://gitee.com/help)
6.  The most popular members  [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
