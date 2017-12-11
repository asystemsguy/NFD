#include "ptable.hpp"

namespace nfd {

Ptable::Ptable() {
	table = new PEntry[DEFAULT_TABLE_SIZE];
	capacity = DEFAULT_TABLE_SIZE;
	srand (std::time(NULL));
}

bool	
Ptable::isPrivate(const Name& name, std::string nonce) {
	PEntry * p = find_entry(name, nonce);
	if(p)
		return p->get_pri_count() > 0;
	return false;
}

bool	
Ptable::isPrivate(const Name& name) {
	PEntry * p = find_entry(name);
	if(p)
		return p->get_pri_count() > 0;
	return false;
}

void 
Ptable::insert(const Name& name, int privacy_count,std::string nonce){
	PEntry pe = PEntry(name, privacy_count, nonce);
	insert(pe);
}

void 
Ptable::insert(std::string name,std::string nonce) {
	Name n = Name(name);
	insert(n, DEFAULT_PRIVACY_COUNT,nonce);
}


void 
Ptable::insert(const Name& name,std::string nonce){
	insert(name,DEFAULT_PRIVACY_COUNT,nonce);
}

void 
Ptable::insert(PEntry pe){
	PEntry * p = find_entry(pe.getName(), pe.getNonce());
	if(!p){
		for(int i=0;i<capacity;i++){
			if(!table[i].isValid()){
				table[i] = pe;
				return;
			}
		}
		table[rand() % capacity] = pe;
	}
}

PEntry*
Ptable::find_entry(const Name& name, std::string nonce){
	for(int i = 0; i<capacity; i++) {
		if(table[i].isValid() && table[i].getName() == name && table[i].getNonce() == nonce) {
			//std::cout<<"Found same entry"<<std::endl;
			return &table[i];
		}
	}
	return NULL;
}

PEntry*
Ptable::find_entry(const Name& name){
	for(int i = 0; i<capacity; i++) {
		if(table[i].isValid() && table[i].getName() == name) {
			//std::cout<<"Found entry with same name"<<std::endl;
			return &table[i];
		}
	}
	return NULL;
}

void
Ptable::print() {
	std::cout<<"		Ptable Start:"<<std::endl;
	for(int i = 0; i<capacity; i++) {
		std::cout<< "		" <<table[i].getName() << " " << table[i].get_pri_count() << " " << table[i].isValid() << " "
		<< table[i].getNonce() << std::endl;;
	}
	std::cout<<"		Ptable End:"<<std::endl;
}

PEntry* 
Ptable::get(int index){
	return &table[index];
}

int
Ptable::size(){
	return capacity;
}

bool 
Ptable::find_name_with_diff_nonce(const Name& name, std::string nonce){
	for(int i = 0; i<capacity; i++) {
		if(table[i].isValid() && table[i].getName() == name && table[i].getNonce() != nonce) {
			//std::cout<<"Found peer with diff nonce"<<std::endl;
			return true;
		}
	}
	return false;
}

void 
Ptable::setDelayed(const Name& name, std::string nonce, bool delayed){
	find_entry(name,nonce)->setDelayed(delayed);
}

bool 
Ptable::hasDelayed(const Name& name, std::string nonce){
	return find_entry(name,nonce)->isDelayed();
}

}