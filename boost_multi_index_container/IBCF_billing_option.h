/*                                                                           
 *  Copyright (c) 2000-2016 NOKIA Technologies.                              
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
                                                                              
#ifndef __IBCF_BILLING_OPTION__
#define __IBCF_BILLING_OPTION__

#if defined(__cplusplus)

#include <stdio.h>
#include <iostream>
using namespace std;
//#include "MEMpublic.h"
//#include "INIT.h"

typedef enum {
    BILLING_OPTION_NONE = 0,
    BILLING_OPTION_RF,
    BILLING_OPTION_ICCF,
    BILLING_OPTION_MAX
} BILLING_OPTIONS;

/*
const char * billing_option_string[BILLING_OPTION_MAX+1] = {
    "BILLING_OPTION_NONE",
    "BILLING_OPTION_RF",
    "BILLING_OPTION_ICCF",
    "BILLING_OPTION_MAX"
};
*/

#define IS_VALID_PTR(__ptr) (__ptr != NULL)

// define multiple index
typedef struct Data_Index
{
    int index1;
    int index2;
    int index3;
    int index4;

    Data_Index(int ax = 0, int ay = 0, int az = 0, int at = 0)
       // : index1(ax), index2(ay), index3(az), index4(at)
    {
        index1 = ax;
        index2 = ay;
        index3 = az;
        index4 = at;
        cout << __func__ << " constructor is called with:" << index1 << "-" << index2 << "-"
             << index3 << "-" << index4 << "-" << endl;
        return;
    }

} Data_Index;

#define BILLING_ADDRESS_LENGTH_MAX (128+1)

// define data to be indexed
typedef struct MultiContainerData
{
    BILLING_OPTIONS billing_option;
    char            billing_address[BILLING_ADDRESS_LENGTH_MAX];
    char            billing_address_backup[BILLING_ADDRESS_LENGTH_MAX];
    MultiContainerData()
    {
        cout<<__func__<<" Constructor is called"<<endl;
        billing_option = BILLING_OPTION_NONE;
        memset(&billing_address, 0, BILLING_ADDRESS_LENGTH_MAX);
        memset(&billing_address_backup, 0, BILLING_ADDRESS_LENGTH_MAX);
    };
} MultiContainerData;

// define object to be indexed
class DB_Indexed
{
public:
    Data_Index         m_index;
    MultiContainerData m_data;

public:
    DB_Indexed(int index1, int index2, int index3, int index4, BILLING_OPTIONS bill_op, const char*bill_addr, const char *bill_addr_bk)
    {
        cout << __func__<<" Constructor is called with :"<<"incoming" << endl;
        if (IS_VALID_PTR(this) == TRUE)
        {
            //INIT_DATA_INFO init;
            //INIT_START_UPD(this, sizeof(DB_Indexed), &init);
            m_index.index1 = index1;
            m_index.index2 = index2;
            m_index.index3 = index3;
            m_index.index4 = index4;

            m_data.billing_option = bill_op;
            if (IS_VALID_PTR(bill_addr) == TRUE)
            {
                strncpy(m_data.billing_address, bill_addr, BILLING_ADDRESS_LENGTH_MAX - 1);
            }

            if (IS_VALID_PTR(bill_addr_bk) == TRUE)
            {
                strncpy(m_data.billing_address_backup, bill_addr_bk, BILLING_ADDRESS_LENGTH_MAX - 1);
            }
            //INIT_STOP_UPD(this, sizeof(DB_Indexed), &init);
        }
        cout << "DB_Indexed: outgoing" << endl;
        cout << "DB_Indexed : " <<m_index.index1<<"-"<<m_index.index2<<"-"<<m_index.index3<<"-"<<m_index.index4<<"-"<< m_data.billing_option<<"-"<<m_data.billing_address<<"-"<<m_data.billing_address_backup<< endl;
    }

    void operator delete(void *p)
    {
        cout << __func__ << " is called" << endl;
        free(p);
    }
    

    void *operator new(size_t s)
    {
        cout << __func__ << " is called with size: "<< s << endl;
        void *myPtr = malloc(s);
        //IMS_GET_WPROV(&myPtr, MEM_DEFAULT, s);
        return myPtr;

    }

    ~DB_Indexed() { }

private:
    DB_Indexed(const DB_Indexed &);
    DB_Indexed &
    operator=(const DB_Indexed &);

};

// define index tag, multi_index_container, and its type
struct DB_Index_Tag
{

};

#endif // #if defined(__cplusplus)
#endif // __IBCF_BILLING_OPTION__
