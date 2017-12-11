#include "ptable_manager.hpp"

namespace nfd {

PTManager* PTManager::pt_manager = 0;

PTManager::PTManager(){}

void 
PTManager::insert_pentry(const Name& name, std::string nonce){
	p_table.insert(name, nonce);
}

void 
PTManager::insert_pentry(std::string name, std::string nonce){
	p_table.insert(name, nonce);
}

void 
PTManager::insert_pentry(const Name& name, int privacy_count, std::string nonce){
	p_table.insert(name, privacy_count, nonce);
}

bool
PTManager::isNamePrivate(const Name& name, std::string nonce){
	return p_table.isPrivate(name, nonce);
}

bool
PTManager::isNamePrivate(const Name& name){
	return p_table.isPrivate(name);
}

PEntry*
PTManager::find_pentry(const Name& name, std::string nonce){
	return p_table.find_entry(name, nonce);
}

void
PTManager::print_table() {
	p_table.print();
}

void 
PTManager::invalidate_all(const Name& name){
	for(int i = 0; i < p_table.size(); i++) {
		PEntry* pe = p_table.get(i);
		if(pe->getName() == name)
			pe->invalidate_myself();
	}
}

bool 
PTManager::peer_check(const Name& name, std::string nonce){
	return p_table.find_name_with_diff_nonce(name, nonce);
}

void 
PTManager::setDelayed(const Name& name, std::string nonce, bool delayed){
	p_table.setDelayed(name, nonce, delayed);
}

bool 
PTManager::hasDelayed(const Name& name, std::string nonce){
	return p_table.hasDelayed(name, nonce);
}

void 
PTManager::publist_insert(std::string name){
	pubList.push_back(name);
}

void 
PTManager::publist_find(std::string name){

}

void 
PTManager::publist_remove(std::string name){
	pubList.erase(std::remove(pubList.begin(), pubList.end(), name), pubList.end());
}

bool 
PTManager::isPublic(const Name& name){
	if ( std::find(pubList.begin(), pubList.end(), name.toUri()) != pubList.end() )
		return true;
	return false;
}

void
PTManager::print_publist(){
	for(size_t i=0;i<pubList.size();i++)
		std::cout << i << " : " << pubList[i] << std::endl;
}	

void 
PTManager::enqueue_pair(bool privacy, std::string nonce){
	//std::cout <<"EQ: " << privacy << std::endl;
	std::pair<bool, std::string> p (privacy, nonce);
	pair_queue.push(p);
}

std::pair<bool, std::string> 
PTManager::dequeue_pair(){
	//std::cout <<"DQ" << std::endl;
	std::pair<bool, std::string> a = pair_queue.front();
	pair_queue.pop();
	return a;
}

}