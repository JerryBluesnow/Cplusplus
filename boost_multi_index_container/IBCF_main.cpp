#include "IBCF_index_container.h"

bool
operator<(const Data_Index &lhs, const Data_Index &rhs)
{
    if (lhs.index1 < rhs.index1)
    {
        return true;
    }
    else if (lhs.index1 > rhs.index1)
    {
        return false;
    }
    else if (lhs.index2 < rhs.index2)
    {
        return true;
    }
    else if (lhs.index2 > rhs.index2)
    {
        return false;
    }
    else if (lhs.index3 < rhs.index3)
    {
        return true;
    }
    else if (lhs.index3 > rhs.index3)
    {
        return false;
    }
    else if (lhs.index4 < rhs.index4)
    {
        return true;
    }
    else if (lhs.index4 > rhs.index4)
    {
        return false;
    }

    return false;
}

int main()
{
    // instantiate a instance for this template class
    DBContainer<MyContainer_T, DB_Index_Tag, DB_Indexed, Data_Index> mycontainer;
    new DB_Indexed(1, 1, 1, 1, BILLING_OPTION_NONE, 0, 0);
    mycontainer.insert(new DB_Indexed(1, 1, 1, 1, BILLING_OPTION_NONE, 0, 0));

    return 0;
} 