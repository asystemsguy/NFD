#ifndef NFD_DAEMON_TABLE_PTABLE_MANAGER_HPP
#define NFD_DAEMON_TABLE_PTABLE_MANAGER_HPP
/*
	Singleton Class Private Table Manager.
	Manage Private Table defined in Ptable class, and advanced control.
	keep current request states as <isPrivate, nonce> pair.

	PTManager will also check for the history access of contents in CS.
*/
#include "ptable.hpp"
#include <queue>

namespace nfd {

class PTManager{

	static PTManager* pt_manager;
	std::vector<std::string> pubList; 
	Ptable p_table;
	std::pair<bool, std::string> last_pair; // current state info.
	std::queue<std::pair<bool, std::string>> pair_queue;
private:

	PTManager();

public:
	// return the singleton
	static PTManager* getInstance() {
		if(!pt_manager)
			pt_manager = new PTManager();
		return pt_manager;
	}

	// return true if is a private request
	bool amIPrivate() { return pair_queue.front().first; }

	// return the nonce within the private request
	std::string getMyNonce() { return pair_queue.front().second; }

	// set states
	void setLastPair(bool privacy, std::string nonce) { last_pair.first = privacy; last_pair.second = nonce; }

	// reset states
	void resetLastPair() { last_pair.first = false; last_pair.second = ""; }

	// check if there are other pentry with same name but different nonce.
	bool peer_check(const Name& name, std::string nonce);

	// insert PEntry.
	void insert_pentry(const ndn::Name& name, std::string nonce);

	void insert_pentry(std::string name, std::string nonce);

	void insert_pentry(const ndn::Name& name, int privacy_count, std::string nonce);

	// return the PEntry with matching name
	PEntry* find_pentry(const Name& name, std::string nonce);

	// check if the name is in table and is private with matching nonce.
	bool isNamePrivate(const ndn::Name& name, std::string nonce);

	// check if only the name is in table and is private.
	bool isNamePrivate(const ndn::Name& name);

	// remove entry with name.
	void remove_entry(const Name& name);

	// print the content of the tabel;
	void print_table();

	// invalidate the pentry when the cache line has been evicted.
	void invalidate_pentry(const Name& name);

	bool wasPrivate(const Name& name);

	// invalidate all the pentry with given name.
	void invalidate_all(const Name& name);

	// set pentry with name and nonce as delayed.
	void setDelayed(const Name& name, std::string nonce, bool delayed);

	// check if pentry with name and nonce has delayed.
	bool hasDelayed(const Name& name, std::string nonce);

	// insert entry into publist
	void publist_insert(std::string name);

	// look up a entry in publist not needed
	void publist_find(std::string name);

	// remove entry in publist
	void publist_remove(std::string name);

	// check if name is in publist
	bool isPublic(const Name& name);

	// print publist
	void print_publist();

	void enqueue_pair(bool privacy, std::string nonce);

	std::pair<bool, std::string> dequeue_pair();
};

} // nfd
#endif