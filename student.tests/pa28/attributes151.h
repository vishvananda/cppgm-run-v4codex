struct __attribute__((abi_tag("ze" "ta", "alpha"))) Record;
struct Record {
  int value;
  Record(int);
  virtual ~Record();
  virtual int read() const;
};
__attribute__((abi_tag("version"))) int tagged(int);
extern int versioned __attribute__((abi_tag("version")));
template<class T> __attribute__((const)) T identity(T);
template<class T> T identity(T v) { return v; }
