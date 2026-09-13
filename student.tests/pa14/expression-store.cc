// Immutable source facts and distinct mutable use state survive recursive growth.
#include "semantic/expression_store.h"
#include <cassert>
int main() {
    using namespace cppgm::semantic;
    ExpressionStore store; store.resize(5);
    Expression source; source.type=3; source.conversions=7; source.count=2; source.ready=true;
    store.set(1,source);
    store.inherit(2,1); store.inherit(3,1);
    source.incoming=11; source.evaluated=true; store.set(2,source);
    source.incoming=13; source.evaluated=false; store.set(3,source);
    assert(store.fact_count()==1 && store[1].incoming==0 && !store[1].evaluated);
    assert(store[2].incoming==11 && store[2].evaluated);
    assert(store[3].incoming==13 && !store[3].evaluated);
    auto snapshot=store[2];
    store.inherit_conversions(2,1,17); store.inherit_conversions(3,1,17);
    source.conversions=17; store.set(3,source);
    assert(store.fact_count()==2 && store.variants==1 && store[1].conversions==7);
    assert(store[2].incoming==11 && store[2].conversions==17 && store[3].incoming==13);
    source.type=9; store.set(1,source);
    for (unsigned i=0;i<4000;++i) { source.type=100+i; store.set(4,source); }
    assert(snapshot.type==3 && snapshot.conversions==7 && snapshot.incoming==11);
    assert(store[2].type==3 && store[3].type==3 && store[1].type==9);
    source.inputs=CallInputs::Source; source.arguments=19; source.argument_count=2;
    store.set(1,source); store.inherit(2,1); store.set(2,store[1]);
    assert(store[1].inputs==CallInputs::Context && store[1].arguments==1);
    assert(store[2].inputs==CallInputs::Context && store[2].arguments==2);
    assert(store.argument_slice(1)==19 && store.argument_slice(2)==19);
}
