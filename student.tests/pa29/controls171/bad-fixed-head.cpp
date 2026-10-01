template<class T,long... I> struct seq {};
template<template<class,int...> class S> struct host {};
host<seq> object;
