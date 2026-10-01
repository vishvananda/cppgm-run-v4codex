int evaluations;
int data[2]={2,3};int (&get())[2]{++evaluations;return data;}
constexpr int test(){int a[2]={2,3};auto [x,y]=a;x=8;return x+y+a[0];}
static_assert(test()==13,"array copies in constants");
int main(){auto [x,y]=get();x=7;if(evaluations!=1||data[0]!=2||x+y!=10)return 1;
 int matrix[2][2]={{1,2},{3,4}};auto [a,b]=matrix;a[0]=8;static_assert(__is_same(decltype(a),int[2]),"array type");
 int sum=0;for(auto [c,d]:matrix){sum+=c+d;c=100;}return matrix[0][0]!=1||a[0]!=8||b[1]!=4||sum!=10;
}
