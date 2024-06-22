
#ifndef EMP_COMPOENENT_SYSTEM_HPP
#define EMP_COMPOENENT_SYSTEM_HPP
#include "debug/log.hpp"
#include "memory/pool_allocator.hpp"
#include <vector>

namespace emp {
template<class Comp>
class ComponentSystem {
protected:
    static std::vector<Comp*>& list() {
        static std::vector<Comp*> m_list;
        return m_list;
    }
    static PoolAllocator& allocator() {
        static PoolAllocator m_alloc(512U * sizeof(Comp), sizeof(Comp));
        return m_alloc;
    }
public:
    static void allocateMemory() {
        allocator().Init();
    }
    static void deallocateMemory() {
        allocator().Cleanup();
    }
    template<class ...InitVals>
    static Comp* create(InitVals ...vals){
        
        void* data = allocator().Allocate(sizeof(Comp));
        auto x = new (data) Comp(vals...);
        list().push_back(x);
        return x;

    }
    static void destroy(Comp* x) {
        auto itr = std::find(list().begin(), list().end(), x);
        if(itr != list().end()) {
            std::swap(*itr, list().back());
            list().pop_back();
        }
        allocator().Free(x);
    }
};
template<class Comp, class CompSys>
class ComponentInstance {
    Comp* inst;
public:
    inline Comp& get() { return *inst; }
    inline Comp* ptr() { return inst; }
    inline const Comp& get() const { return *inst; }
    inline const Comp* ptr() const { return inst; }

    Comp* operator&() { return inst; }
    const Comp* operator&() const { return inst; }

    inline operator Comp&() { return *inst; }

    template<class ...InitVals>
    ComponentInstance(InitVals... vals) {
        inst = CompSys::create(vals...);
        EMP_LOG(DEBUG3) << "created: " << typeid(Comp).name() << "\tat: " << inst; 
    }
    ~ComponentInstance() {
        EMP_LOG(DEBUG3) << "destroyed: " << typeid(Comp).name() << "\tat: " << inst; 
        CompSys::destroy(inst);
    }
};
};
#endif
