#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
using namespace std;

class IBCFNetwork
{
public:
    IBCFNetwork(int in_vn_id){vn_id = in_vn_id;}
    ~IBCFNetwork(){}
    int getID()
    {
           return vn_id; 
    }
private:
    int vn_id;
};

class IBCFtrunkGroup
{
public:
    IBCFtrunkGroup(IBCFNetwork* parent, int in_tg_id)
    {
        parentVN = parent;
        tg_id = in_tg_id;
    }
    ~IBCFtrunkGroup(){}
    
    IBCFNetwork *getParent() 
    {
        return parentVN;
    }

private:
    int tg_id;
    IBCFNetwork *parentVN;
};

typedef struct{
    
}updated_queue;


void
queue_delete_verify()
{
    IBCFNetwork *virtual_network = new IBCFNetwork(1);
    if (virtual_network == NULL)
    {
       std::cout<<"new IBCFNetwork fails"<<std::endl;
       return;
    }
    
    std::cout<<"virtual_network: "<<virtual_network<<std::endl;

    IBCFtrunkGroup *trunk_group = new IBCFtrunkGroup(virtual_network, 1);
    if (trunk_group == NULL)
    {
       std::cout<<"new IBCFtrunkGroup fails"<<std::endl;
       return;
    }

    std::cout<<"VN ID is: "<<virtual_network->getID()<<std::endl;

    delete virtual_network;
    memset(virtual_network, 0, sizeof(IBCFNetwork));
    virtual_network = NULL;
    std::cout<<"parent in trunkgroup: "<<trunk_group->getParent()<<std::endl;
    std::cout<<"VN ID is: "<<trunk_group->getParent()->getID()<<std::endl;

    return;
}

int main()
{
    queue_delete_verify();

    system("pause");
    return 0;
}

