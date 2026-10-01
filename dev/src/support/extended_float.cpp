#include "support/extended_float.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace cppgm {
namespace {
using Wide = unsigned __int128;
Wide bits(ExtendedFloat x) { Wide n; std::memcpy(&n,&x,16); return n; }
ExtendedFloat value(Wide n) { ExtendedFloat x; std::memcpy(&x,&n,16); return x; }
// Temporary limbs are released at the literal boundary. The fixed-format
// bound is 20,000 significant decimal digits: every binary128 rounding
// midpoint terminates before this, including the least subnormal midpoint.
// Later digits contribute sticky information only, so scanning stays linear.
struct Integer {
    std::vector<std::uint32_t> a;
    explicit Integer(unsigned n = 0) { if (n) a.push_back(n); }
    void trim() { while (!a.empty() && !a.back()) a.pop_back(); }
    void multiply(unsigned n, unsigned carry = 0) {
        for (auto& x : a) { std::uint64_t t = std::uint64_t(x)*n+carry; x=t; carry=t>>32; }
        if (carry) a.push_back(carry);
    }
    unsigned width() const { if (a.empty()) return 0; unsigned n=32*(a.size()-1); for (auto x=a.back();x;x>>=1) ++n; return n; }
    void shift(unsigned n) {
        if (a.empty()) return;
        unsigned words=n/32, rest=n%32;
        a.insert(a.begin(),words,0);
        if (!rest) return;
        unsigned carry=0;
        for (auto& x : a) { auto t=(std::uint64_t(x)<<rest)|carry; x=t; carry=t>>32; }
        if (carry) a.push_back(carry);
    }
    int compare(const Integer& b) const {
        if (a.size()!=b.a.size()) return a.size()<b.a.size() ? -1:1;
        for (unsigned k=a.size();k;--k) if (a[k-1]!=b.a[k-1]) return a[k-1]<b.a[k-1] ? -1:1;
        return 0;
    }
    void subtract(const Integer& b) {
        std::uint64_t borrow=0;
        for (unsigned k=0;k<a.size();++k) { auto n=std::uint64_t(k<b.a.size()?b.a[k]:0)+borrow; borrow=a[k]<n; a[k]-=n; }
        trim();
    }
    void halve() { unsigned carry=0; for (unsigned k=a.size();k;--k) { auto next=a[k-1]&1; a[k-1]=(a[k-1]>>1)|(carry<<31); carry=next; } trim(); }
};
Wide round_shift(Wide n, unsigned shift) {
    if (!shift) return n;
    if (shift>128) return 0;
    Wide half=Wide(1)<<(shift-1), top=shift==128?0:n>>shift;
    Wide rest=n&(half-1);
    return top+((n&half) && (rest || (top&1)));
}
}
bool finite_float(ExtendedFloat x) { return ((bits(x)>>112)&32767)!=32767; }
bool nan_float(ExtendedFloat x) { return !finite_float(x) && (bits(x)&((Wide(1)<<112)-1)); }
ExtendedFloat integer_float(Wide n, bool negative, unsigned precision) {
    if (negative) n=0-n;
    if (!n) return 0;
    unsigned width=0; for (auto part=n;part;part>>=1) ++width;
    int exponent=width-1;
    if (width>precision) { n=round_shift(n,width-precision); width=precision; }
    if (n>>width) { n>>=1; ++exponent; }
    return value((Wide(negative)<<127)|(Wide(exponent+16383)<<112)|((n<<(113-width))&((Wide(1)<<112)-1)));
}
ExtendedFloat trunc_float(ExtendedFloat x) {
    Wide n=bits(x); int e=int((n>>112)&32767)-16383;
    if (e<0) return value(n&(Wide(1)<<127));
    if (e<112) n&=~((Wide(1)<<(112-e))-1);
    return value(n);
}
bool sign_float(ExtendedFloat x) { return bits(x)>>127; }
ExtendedFloat half_value(std::uint16_t n) {
    unsigned e=(n>>10)&31, f=n&1023;
    Wide r=Wide(n>>15)<<127;
    if (e==31) r|=(Wide(32767)<<112)|(Wide(f)<<102);
    else if (e) r|=(Wide(e+16368)<<112)|(Wide(f)<<102);
    else if (f) { int shift=0; while (!(f&1024)) { f<<=1; ++shift; } r|=(Wide(16369-shift)<<112)|(Wide(f&1023)<<102); }
    return value(r);
}
std::uint16_t half_bits(ExtendedFloat x) {
    Wide n=bits(x), f=n&((Wide(1)<<112)-1);
    unsigned sign=(n>>127)<<15, e=(n>>112)&32767;
    if (e==32767) { unsigned payload=f>>102; return sign|31744|(f ? (payload?payload:1) : 0); }
    if (!e) return sign;
    int exponent=int(e)-16383;
    if (exponent>15) return sign|31744;
    f|=Wide(1)<<112;
    if (exponent < -14) return sign|unsigned(round_shift(f,102+(-14-exponent)));
    auto rounded=round_shift(f,102);
    return sign|unsigned((exponent+14)*1024+rounded);
}
ExtendedFloat parse_extended_float(TextView text, unsigned precision) {
    if (precision!=11 && precision!=113) throw std::logic_error("unsupported extended literal precision");
    std::size_t pos=0; bool negative=false;
    if (pos<text.size && (text.data[pos]=='-' || text.data[pos]=='+')) negative=text.data[pos++]=='-';
    bool hex=pos+2<text.size && text.data[pos]=='0' && (text.data[pos+1]=='x'||text.data[pos+1]=='X');
    if (hex) pos+=2;
    Integer numerator, denominator(1);
    int fraction=0, discarded=0, retained=0; bool point=false, started=false, sticky=false, digit=false;
    for (;pos<text.size;++pos) {
        char c=text.data[pos];
        if (c=='.' && !point) { point=true; continue; }
        int d=c>='0'&&c<='9'?c-'0':hex&&c>='a'&&c<='f'?c-'a'+10:hex&&c>='A'&&c<='F'?c-'A'+10:-1;
        if (d<0) break;
        digit=true; if (point) ++fraction;
        started|=d!=0;
        if (!started) continue;
        if (retained<20000) { numerator.multiply(hex?16:10,d); ++retained; }
        else { ++discarded; sticky|=d!=0; }
    }
    int exponent=0;
    if (pos<text.size && (text.data[pos]==(hex?'p':'e')||text.data[pos]==(hex?'P':'E'))) {
        ++pos; bool minus=false;
        if (pos<text.size&&(text.data[pos]=='-'||text.data[pos]=='+')) minus=text.data[pos++]=='-';
        auto first=pos;
        for (;pos<text.size&&text.data[pos]>='0'&&text.data[pos]<='9';++pos) exponent=std::min(1000000,exponent*10+text.data[pos]-'0');
        if (first==pos) throw std::runtime_error("invalid floating exponent");
        if (minus) exponent=-exponent;
    }
    if (!digit || pos!=text.size) throw std::runtime_error("invalid extended floating literal");
    Wide sign=Wide(negative)<<127;
    if (!started) return value(sign);
    int scale=exponent-(hex?4:1)*(fraction-discarded);
    int magnitude=scale+(hex?4:1)*retained;
    if (magnitude>(hex?17000:5100)) return value(sign|(Wide(32767)<<112));
    if (magnitude<-(hex?17000:5100)) return value(sign);
    if (hex) { if (scale>=0) numerator.shift(scale); else denominator.shift(-scale); }
    else if (scale>=0) for (int k=0;k<scale;++k) numerator.multiply(10);
    else for (int k=0;k<-scale;++k) denominator.multiply(10);
    int binary=int(numerator.width())-int(denominator.width());
    Integer probe=binary>=0?denominator:numerator; probe.shift(binary>=0?binary:-binary);
    if ((binary>=0?numerator.compare(probe):probe.compare(denominator))<0) --binary;
    int minimum=precision==11?-14:-16382, maximum=precision==11?15:16383;
    if (binary>maximum) return value(sign|(Wide(32767)<<112));
    int unit=std::max(binary,minimum)-int(precision)+1;
    if (unit<0) numerator.shift(-unit); else denominator.shift(unit);
    int quotient_bits=int(numerator.width())-int(denominator.width());
    Wide quotient=0;
    if (quotient_bits>=0) {
        probe=denominator; probe.shift(quotient_bits);
        for (int k=quotient_bits;k>=0;--k) { if (numerator.compare(probe)>=0) { numerator.subtract(probe); quotient|=Wide(1)<<k; } probe.halve(); }
    }
    numerator.shift(1); int half=numerator.compare(denominator);
    if (half>0 || (half==0 && (sticky || (quotient&1)))) ++quotient;
    if (!quotient) return value(sign);
    unsigned width=0; for (Wide n=quotient;n;n>>=1) ++width;
    int e=unit+width-1;
    if (e>maximum) return value(sign|(Wide(32767)<<112));
    if (e>=-16382) return value(sign|(Wide(e+16383)<<112)|((quotient<<(113-width))&((Wide(1)<<112)-1)));
    return value(sign|(quotient<<unsigned(unit+16494)));
}
std::string extended_float_text(ExtendedFloat x) {
    Wide n=bits(x); bool sign=n>>127; unsigned e=(n>>112)&32767; Wide f=n&((Wide(1)<<112)-1);
    std::string s=sign?"-":"";
    if (e==32767) return s+(f?"nan":"inf");
    s+=e?"0x1.":"0x0.";
    static const char digits[]="0123456789abcdef";
    for (int k=27;k>=0;--k) s+=digits[unsigned(f>>(k*4))&15];
    s+='p'; s+=std::to_string(e?int(e)-16383:-16382); return s;
}
}
