# Summer2021-No.56 优化Linux内核中的复杂头文件依赖

#### 介绍
https://gitee.com/openeuler-competition/summer-2021/issues/I3EIOX

#### 软件架构

This tool is used to delete the superfluous header files in the linux kernel.The tool runs in three steps

1. For a specific source file A, check all the header files that it has included;
2. Check all the functions and macro that A has used , then count the numbers of macros/functions A has used in each header file.If some header files was included but used rarely,than it can be deleted and the the declarations can be moved to the other file. For example,A has included a header file B. B has tons of declarations but only one declaration was used.Then we will consider to delete it.

3. After we have confirm the header file we want to delete(say B).We will check whether we can move the only one declaration(say C) to another file(say D).To achieve the goal,we should check all the files in the linux kernel that has used the C.If D was included by every files who has used C.Then we can move the declaration C from B to D.Moreover,the source file A won't need to include the header file B.Thus we have reduced the superfluous header file.


#### 安装教程

1.  git clone https://gitee.com/openeuler-competition/summer2021-56.git
2.  compile the project.cpp code with the command: g++ project.cpp -std=c++17 -o project -O2 -fopenmp

#### 使用说明

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
