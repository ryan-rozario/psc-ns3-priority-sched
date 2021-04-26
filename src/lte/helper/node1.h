#ifndef NODE1_H
#define NODE1_H


#include "ns3/node.h"


namespace ns3 {

class Node1 : public Node{
public:
	char type;
	Node1(char t){
		type=t;
	}
};


}

#endif