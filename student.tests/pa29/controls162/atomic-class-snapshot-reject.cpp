struct S{int a;};int main(){_Atomic(S) s={3};const S& r=s;__c11_atomic_store(&s,S{9},5);return r.a==3?0:1;}
