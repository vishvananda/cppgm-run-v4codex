struct HostRoot152 {
  int value;
  HostRoot152();
  virtual ~HostRoot152();
  operator bool() const;
};
struct HostMiddle152 : virtual HostRoot152 {
  HostMiddle152();
  virtual ~HostMiddle152();
};
struct HostLeaf152 : HostMiddle152 {
  HostLeaf152();
  virtual ~HostLeaf152();
};
extern int destroyed152;
HostMiddle152& fill152(HostMiddle152&,int);
