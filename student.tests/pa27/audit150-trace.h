#ifndef AUDIT150_TRACE_H
#define AUDIT150_TRACE_H
extern int destroyed;
extern long imported[2];
extern thread_local int ticket;

struct Resource150 {
    int value;
    Resource150(int n):value(n) { if (n < 0) throw n; }
    ~Resource150() { ++destroyed; }
};
template<int N> struct Packet150 {
    long prefix;
    union { int lane; long alternate; };
    Resource150 resource;
    Packet150(int n):prefix(N),lane(n),resource(n+1) {}
    long read() const { return prefix+lane+resource.value+imported[1]+ticket; }
    template<class T> void unused() { typename T::missing invalid; }
};
template<int N> __attribute__((noinline)) long measure150(int input) {
    Packet150<N> packet(input);
    return packet.read();
}
typedef long (*Measure150)(int);
#endif
