#ifndef PEERS_LIST_H
#define PEERS_LIST_H

#include "peers_list_base.h"

class PeersList: public PeersListBase
{
public:
	PeersList() {}
	~PeersList() {}
};

extern PeersList peers_list;

#endif
