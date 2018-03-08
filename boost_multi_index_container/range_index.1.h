#include <iostream>
#include <stdio.h>
#include <stdlib.h>

// define multiple index
class range_index{
public:
    unsigned int index_start;
    unsigned int index_end;

    range_index() : index_start(0), index_end(0)
    {
        //
    }
    range_index(unsigned int start, unsigned int end) : index_start(start), index_end(end)
    {
        //
    }
    range_index(const range_index &obj)
    {
        this->index_start = obj.index_start;
        this->index_end   = obj.index_end;
    }

    range_index operator=(const range_index &obj)
    {
        this->index_start = obj.index_start;
        this->index_end   = obj.index_end;
        return *this;
    }

    bool
    operator==(const range_index &obj)  //重载 ==
    {
        // when start==end, it indicates the struct is a value, not a range!
        if (this->index_start == this->index_end) 
        {
            return ((this->index_start >= obj.index_start) && (this->index_end <= obj.index_end));
        }

        // when start==end, it indicates the struct is a value, not a range!
        if (obj.index_start == obj.index_end)
        {
            return ((this->index_start <= obj.index_start) && (this->index_end >= obj.index_end));
        }

        // if above conditions aren't satisfied, it indicates two range are compared!
        return (this->index_start == obj.index_start && this->index_end == obj.index_end);
    }

    bool
    operator<(const range_index &obj)  //重载 ==
    {
        if (this->index_start > obj.index_start)
        {
            return (false);
        }

        // (0, 10) < (1,2), (0, 2) < (1,2), (1,1)<(2,2), (1,1)<(2,3)
        if (this->index_start < obj.index_start)
        {
            return (true);
        }

        // the start index is equal
        // (1,1) > (0,2)
        if (this->index_end > obj.index_end)
        {
            return (false);
        }

        if (this->index_end < obj.index_end)
        {
            return (true);
        }

        return (false);
    }

    bool
    operator>(const range_index &obj)  //重载 ==
    {
        if (this->index_start < obj.index_start)
        {
            return (false);
        }

        // (0, 10) < (1,2), (0, 2) < (1,2), (1,1)<(2,2), (1,1)<(2,3)
        if (this->index_start > obj.index_start)
        {
            return (true);
        }

        // the start index is equal
        // (1,1) > (0,2)
        if (this->index_end < obj.index_end)
        {
            return (false);
        }

        if (this->index_end > obj.index_end)
        {
            return (true);
        }

        return (false);
    }

};