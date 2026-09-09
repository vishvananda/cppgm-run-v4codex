struct alignas(16) Forward;
struct alignas(16) Forward { char c; };
struct AlignedField { char first; alignas(8) char second; char third; };
struct Nested { struct Inner; };
struct alignas(long double) Nested::Inner { char c; };
#define BEGIN_PACK _Pragma("pack(push, packet, 1)")
#define END_PACK _Pragma("pack(pop, packet)")
BEGIN_PACK
struct Packet { char c; int i; };
#pragma pack(push, 2)
struct Half { char c; int i; };
#pragma pack(pop)
struct StillPacked { char c; int i; };
END_PACK
struct Natural { char c; int i; };
struct __attribute__((packed)) AttributePacked { char c; int i; };
int main() {
    if (sizeof(Forward) != 16 || alignof(Forward) != 16) return 1;
    if (sizeof(AlignedField) != 16 || alignof(AlignedField) != 8) return 2;
    if (sizeof(Nested::Inner) != 16 || alignof(Nested::Inner) != 16) return 3;
    if (sizeof(Packet) != 5 || sizeof(Half) != 6 || sizeof(StillPacked) != 5) return 4;
    if (sizeof(Natural) != 8 || sizeof(AttributePacked) != 5) return 5;
    Packet p = {3,17};
    return p.c != 3 || p.i != 17;
}
