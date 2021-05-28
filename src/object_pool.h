#ifndef OBJECT_POOL_HEADER_
#define OBJECT_POOL_HEADER_

#include <stdio.h>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <mutex>

using namespace std;

template <typename T, int S = 10>
class object_pool
{
    // Wrapper for the actual data stored in the object pool
    struct pool_element
    {
        int healthy{1};
        T* entry;

        pool_element() : entry(nullptr) {}
        ~pool_element() { if (entry) delete entry; }
    };

    struct custom_deleter
    {
        object_pool* pool_;
        custom_deleter(object_pool* pool) : pool_(pool) {}
        void operator()(T* pobject)
        {
            if (pobject)
                pool_->reclaim_object(pobject);
        }
    };

    using storage_type = unordered_map<T*, pool_element*>;
    using custom_deleter_type = typename object_pool::custom_deleter;

    custom_deleter_type object_deleter;
    storage_type        used_items_;
    storage_type        free_items_;
    mutex               mtx_;

public:

    using deleter_type = typename object_pool::custom_deleter;
    using uptr_type = std::unique_ptr<T, custom_deleter>;

    template<class ...Args>
    object_pool(Args&& ...args) : object_deleter(this)
    {
        for (int i = 0; i < S; ++i)
        {
            T* obj = new T{std::forward<Args>(args)...};
            pool_element* pel = new pool_element;
            pel->entry = obj;
            free_items_[obj] = pel;
        }
    }

    template<class ...Args>
    void add(Args&& ...args)
    {
        T* pobj = new T(std::forward<Args>(args)...);
        pool_element* pel = new pool_element;
        pel->entry = pobj;
        {
            lock_guard<mutex> lck(mtx_);
            free_items_[pobj] = pel;
        }
    }

    void remove(uptr_type& ptobject)
    {
        if (!ptobject)
            return;

        storage_type::iterator it;
        {
            lock_guard<mutex> lck(mtx_);
            it = used_items_.find(ptobject.get());
            // Silently ignore this call on non existing entry
            if (it == used_items_.end())
                return;
        }

        pool_element* pel = it->second;
        pel->healthy = 0;

        //Just reset the pointer, the memory deallocation and the cleanup will be done in the deleter
        ptobject.reset();
    }

    void reclaim_object(T* pobject)
    {
        if (nullptr != pobject)
        {
            lock_guard<mutex> lck(mtx_);
            pobject->print();

            storage_type::iterator it = used_items_.find(pobject);
            if (it == used_items_.end())
                throw std::runtime_error("object not in the used items store");
            
            pool_element* pel = it->second;
            if (0 == pel->healthy)
            {
                used_items_.erase(it);
                delete pel;
            }
            else // Reuse this object by adding to the 'free' objects store
            {
                used_items_.erase(it);
                free_items_[pobject] = pel;
            }
        }
        else
        {
            printf("object not found\n");
        }
    }

    size_t size() const
    {
        lock_guard<mutex> lck(mtx_);
        return used_items_.size();
    }

    size_t capacity() const
    {
        lock_guard<mutex> lck(mtx_);
        return used_items_.size() + free_items_.size();
    }

    uptr_type get()
    {
        pool_element* pel(nullptr);
        
        if (0 == free_items_.size())
        {
            // Alloc new object and return it
            pel = new pool_element;
            pel->entry = new T();
            {
                lock_guard<mutex> lck(mtx_);
                used_items_[pel->entry] = pel;
            }
            return std::unique_ptr<T, custom_deleter>(pel->entry, object_deleter);
        }        
        
        {
            lock_guard<mutex> lck(mtx_);
            pel = free_items_.begin()->second;
            free_items_.erase(free_items_.begin());
            used_items_[pel->entry] = pel;
        }
        return std::unique_ptr<T, custom_deleter>(pel->entry, object_deleter);
    }
};

#endif // OBJECT_POOL_HEADER_
