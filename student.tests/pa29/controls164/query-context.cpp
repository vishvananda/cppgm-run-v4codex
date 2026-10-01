template<int N,int M> constexpr int suffix(const char(&)[N],const char(&)[M]){return M-1;}
template<class T> int bound(){return sizeof(char[1+suffix(__PRETTY_FUNCTION__,"int bound() [T = ")])-1;}
template<class T> int first(){static_assert(__PRETTY_FUNCTION__[0]=='i',"");return __PRETTY_FUNCTION__[0];}
template<class T> int refs(){const char(&name)[sizeof(__PRETTY_FUNCTION__)]=__PRETTY_FUNCTION__;return name[0];}
int main(){return bound<int>()!=17 || bound<double>()!=17 || first<int>()!='i' || refs<long>()!='i';}
