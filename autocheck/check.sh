#! /bin/bash
g++ project.cpp -o project -std=c++17 -O2
./check test.c
./check -p check.c -o test2.txt
./check -p check.h -o test3.txt
t1=`diff check.svg output.svg`
t2=`diff test2.txt checksource.txt`
t3=`diff test3.txt checkhead.txt`
if ${t1};
then
    echo "pass test 1"
else
    echo "test 1 failed"
fi
if ${t2};
then
    echo "pass test 2"
else
    echo "test 2 failed"
fi
if ${t3};
then
    echo "pass test 3"
else
    echo "test 3 failed"
fi
rm test2.txt test3.txt output.svg project