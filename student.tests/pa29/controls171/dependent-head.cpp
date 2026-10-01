template<class T,int... I> struct fixed {};
template<template<class T,T...> class S> struct host { typedef S<int,0> type; };
host<fixed>::type object;
int main(){return 0;}
