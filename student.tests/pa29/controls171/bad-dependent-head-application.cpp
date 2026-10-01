template<class T,T*... I> struct seq {};
template<template<class,int...> class S> struct host { typedef S<int,1> type; };
host<seq>::type object;
