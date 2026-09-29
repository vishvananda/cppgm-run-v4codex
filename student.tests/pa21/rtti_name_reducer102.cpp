namespace std { class type_info {}; }
namespace n { 
template<class T, class... U> struct V {};
template<class K, class T, class C, class A> struct M {};
template<template<class, class, class, class> class O = M, template<class, class...> class X = V, class Z = V<int>> struct W {};
using Test = W<n::M>;
}
template<class T> struct O {};
const std::type_info& probe() { return typeid(O<n::Test>); }
int main() { return &probe() ? 0 : 1; }
