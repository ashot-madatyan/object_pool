#include <stdio.h>
#include "object_pool.h"

using namespace std;

struct no_ctor
{
private:
    no_ctor() {}
    ~no_ctor() {}

public:
    int v;
    string s;
};

struct cstm_ctbl
{
    int v;
    string s;

    cstm_ctbl(int vv, const string& str) : v(vv), s(str) {}
};

struct dflt_ctbl
{
    int v;
    string s;

    dflt_ctbl() : v(0) {}
    ~dflt_ctbl() {}
};


struct test_struct
{
    int val;
    string str;

    void print()
    {
        printf("val: %d str: %s\n", val, str.c_str());
    }

    test_struct() : val(0)
    {
        static int counter = 1;
        str = to_string(counter);
        ++counter;
    }

    test_struct(const int& val, const string& s) : val(val), str(s)
    {
        static int counter = 1;
        //str = to_string(counter);
        ++counter;
    }
};


void test_object_pool()
{
    object_pool<test_struct> gm;
    vector<std::unique_ptr<test_struct, object_pool<test_struct>::deleter_type>> vvv;
    using ptr_type = std::unique_ptr<test_struct, object_pool<test_struct>::deleter_type>;

    gm.add(5, "ashot_madatyan");

    for (int i = 0; i < 20; ++i)
    {
        ptr_type ptr = gm.get();
        //printf("got object: %d %s\n", ptr->val, ptr->str.c_str());
        vvv.push_back(move(ptr));
    }
    
    // Test the remove functional
    ptr_type ptr = std::move(vvv[vvv.size() - 1]);
    vvv.pop_back();
    gm.remove(ptr);
    
    for (int i = 0; i < 4; ++i)
        vvv.pop_back();

}

void test_object_pool_init_list()
{
    object_pool<test_struct> gm{ 
                                test_struct{1, "one"}, 
                                test_struct{2, "two"}, 
                                test_struct{3, "three"} 
                                };

    vector<std::unique_ptr<test_struct, object_pool<test_struct>::deleter_type>> vvv;
    using ptr_type = std::unique_ptr<test_struct, object_pool<test_struct>::deleter_type>;
    
    int sz = (int)gm.size();

    for (int i = 0; i < sz; ++i)
    {
        ptr_type ptr = gm.get();
        vvv.push_back(move(ptr));
    }

    for (int i = 0; i < (int)vvv.size(); ++i)
        vvv[i]->print();
}


int main()
{
    //test_object_pool();
    test_object_pool_init_list();
    return 0;
    
    object_pool<dflt_ctbl> op;
    object_pool<cstm_ctbl> op1(16, "ashot_madatyan");

    
    return 0;
}

