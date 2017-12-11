#include "ptable_entry.hpp"
namespace nfd {
//namespace cs {

PEntry::PEntry(){
	m_name = Name();
	pri_count = 0;
	valid = false;
	m_nonce = "";
	delayed = false;
}

PEntry::PEntry(const Name& name, int init_count, std::string nonce){
	m_name = name;
	pri_count = init_count;
	valid = true;
	m_nonce = nonce;
	delayed = false;
}

bool 
PEntry::operator<(const PEntry& other) const{
	return m_name < other.getName();
}

void 
PEntry::invalidate_myself(){
	valid = false;
}

//}
}