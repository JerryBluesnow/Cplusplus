#include <stdio.h>
#include <iostream>

using namespace std;

typedef enum {
    BILLING_OPTION_NONE = 0,
    BILLING_OPTION_RF,
    BILLING_OPTION_ICCF,
    BILLING_OPTION_MAX
} BILLING_OPTIONS;

const char * billing_option_string[BILLING_OPTION_MAX+1] = {
    "BILLING_OPTION_NONE",
    "BILLING_OPTION_RF",
    "BILLING_OPTION_ICCF",
    "BILLING_OPTION_MAX"
};

// define multiple index
typedef struct Data_Index
{
    int index1;
    int index2;
    int index3;
    int index4;

    Data_Index(int ax = 0, int ay = 0, int az = 0, int at = 0)
        : index1(ax), index2(ay), index3(az), index4(at)
    {

    }

    void
    print(const char *prompt) const
    {
        cout << "(" << index1 << ", " << index2 << ", " << index3 << ", " << index4 << ") - "
             << prompt << endl;
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
        m_index.index1 = index1;
        m_index.index2 = index2;
        m_index.index3 = index3;
        m_index.index4 = index4;

        m_data.billing_option = bill_op;
        if (bill_addr != NULL)
        {
            strncpy(m_data.billing_address, bill_addr, BILLING_ADDRESS_LENGTH_MAX - 1);
        }

        if (bill_addr_bk != NULL)
        {
            strncpy(m_data.billing_address_backup, bill_addr_bk, BILLING_ADDRESS_LENGTH_MAX - 1);
        }
    }

    ~DB_Indexed() { print(", destructed"); }

    void
    print(const char *prompt = "") const
    {
        cout << "(" << m_index.index1 << ", " << m_index.index2 << ", " << m_index.index3 << ", "
             << m_index.index4 << ") - ";
        cout << "(" << billing_option_string[m_data.billing_option] << ", " << m_data.billing_address << ", " << m_data.billing_address_backup << ")" << prompt << endl;
    }

private:
    DB_Indexed(const DB_Indexed &);
    DB_Indexed &
    operator=(const DB_Indexed &);

};

// define index tag, multi_index_container, and its type
struct DB_Index_Tag
{

};