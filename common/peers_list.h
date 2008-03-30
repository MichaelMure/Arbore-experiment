#ifndef PEERS_LIST_H
#define PEERS_LIST_H

#include <map>
#include <vector>
#include "peer.h"
#include "mutex.h"

class PeersList: public std::vector<Peer*>, public Mutex
{
private:
	typedef std::map<int, Peer*> PeerMap;
	PeerMap fd2peer;
	pf_id my_id;

	Peer* PeerFromFD(int fd);
public:
	PeersList();
	~PeersList();

	unsigned int Size() const { return size(); }
	void Add(Peer* p);
	void Erase(int fd); /* Remove the peer from the list and erase it */
	Peer* Remove(int fd); /* Remove the peer from the list */
	void PeerFlush(int fd);
	bool PeerReceive(int fd);

	void CloseAll();

	pf_id GetMyID() const { return my_id; } // TODO:Protect-me
	void SetMyID(const pf_id id) { my_id = id; } // TODO:Protect-me

	/* Create an ID not used by any other peer in network */
	pf_id CreateID();

	// TODO: move-me into private section
	Peer* PeerFromID(pf_id id);
};

extern PeersList peers_list;

#endif

