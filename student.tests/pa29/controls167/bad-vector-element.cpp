typedef int V __attribute__((vector_size(16)));
inline V invalid(){return (V){1,"bad",3,4};}
