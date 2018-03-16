/*                                                                           
 *  Copyright (c) 2000-2017 NOKIA Technologies.                              
 *  All rights reserved.                                                     
 *                                                                           
 *  ALL RIGHTS ARE RESERVED BY NOKIA TECHNOLOGIES.                           
 *  ACCESS TO THIS SOURCE CODE IS STRICTLY RESTRICTED UNDER                  
 *  CONTRACT. THIS CODE IS TO BE KEPT STRICTLY CONFIDENTIAL                  
 *  UNAUTHORIZED MODIFICATIONS OF THIS FILE WILL VOID YOUR                   
 *  SUPPORT CONTRACT WITH NOKIA TECHNOLOGIES.  IF SUCH                       
 *  MODIFICATIONS ARE FOR THE PURPOSE OF CIRCUMVENTING                       
 *  LICENSING LIMITATIONS, LEGAL ACTION MAY RESULT.                          
 */                                                                          
                                                                              
#ifndef __IBCF_INDEX_CONTAINER_H__
#define __IBCF_INDEX_CONTAINER_H__

#if defined(__cplusplus)

#include "boost/multi_index_container.hpp"
#include "boost/multi_index/member.hpp"
#include "boost/multi_index/ordered_index.hpp"
#include "IBCF_billing_option.h"

using namespace boost::multi_index;
using boost::multi_index_container;

extern bool
operator<(const Data_Index &lhs, const Data_Index &rhs);

typedef multi_index_container<
    DB_Indexed *,
    indexed_by<
        ordered_unique<tag<DB_Index_Tag>, member<DB_Indexed, Data_Index, &DB_Indexed::m_index> > > >
                                                           MyContainer_T;
typedef MyContainer_T::index<DB_Index_Tag>::type           MyContainerIndex_T;
typedef MyContainer_T::index<DB_Index_Tag>::type::iterator MyContainerIterator_T;
typedef std::pair<MyContainerIterator_T, bool> MyContainerPair_T;

// a template class
template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T>
class DBContainer
{
    MultiIndexContainer_T theContainer;

   public:
    void
    insert(Data_T *data);
    Data_T *
    find(const Index_T &index);
    void
    free();
    void
    erase(const Index_T &index);
};

template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T>
void
DBContainer<MultiIndexContainer_T, Tag_T, Data_T, Index_T>::insert(Data_T *data)
{
    theContainer.insert(data);
}

template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T>
Data_T *
DBContainer<MultiIndexContainer_T, Tag_T, Data_T, Index_T>::find(const Index_T &index)
{
    const typename boost::multi_index::index<MultiIndexContainer_T, Tag_T>::type &indexSet =
        get<Tag_T>(theContainer);
    const typename boost::multi_index::index<MultiIndexContainer_T, Tag_T>::type::iterator iter =
        indexSet.find(index);
    if (indexSet.end() == iter)
    {
        return NULL;
    }
    return (*iter);
}

template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T>
void
DBContainer<MultiIndexContainer_T, Tag_T, Data_T, Index_T>::free()
{
    typedef typename MultiIndexContainer_T::value_type value_type;
    while (!theContainer.empty())
    {
        typename MultiIndexContainer_T::iterator iter = theContainer.begin();
        if (NULL == (*iter))
        {
            theContainer.erase(iter);
            continue;
        }
        value_type pobj = *iter;
        theContainer.erase(iter);
        delete pobj;
    }
}

template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T>
void
DBContainer<MultiIndexContainer_T, Tag_T, Data_T, Index_T>::erase(const Index_T &index)
{
    const typename boost::multi_index::index<MultiIndexContainer_T, Tag_T>::type &indexSet =
        get<Tag_T>(theContainer);
    const typename boost::multi_index::index<MultiIndexContainer_T, Tag_T>::type::iterator iter =
        indexSet.find(index);

    if (indexSet.end() == iter)
    {
        return;
    }

    typedef typename MultiIndexContainer_T::value_type value_type;
    value_type pobj = *iter;
    theContainer.erase(iter);
    delete pobj;

    return;
}

#endif // #if defined(__cplusplus)
#endif //__IBCF_INDEX_CONTAINER_H__
