#ifndef NFD_DAEMON_TABLE_PTABLE_ENTRY_HPP
#define NFD_DAEMON_TABLE_PTABLE_ENTRY_HPP

namespace nfd {
//namespace cs {
class PEntry {
private:
	Name m_name;
	int pri_count;
	std::string m_nonce;
	bool valid;
	bool delayed;

public:

	// default constructor;
	PEntry();

	// construct entry from a name and desired privacy count;
	PEntry(const Name& name, int init_count = 1, std::string nonce = "");

	// turn the entry into invalid;
	void invalidate_myself();

	// return the name associated with entry;
	const Name& getName() const { return m_name; }

	// return current privacy count;
	int get_pri_count() { return pri_count; }

	// check if valid;
	bool isValid() { return valid; }

	// check if i have been delayed for peers.
	bool isDelayed() { return delayed; }

	void setDelayed(bool delay) { delayed = delay; }

	std::string getNonce() {
		return m_nonce;
	}
	// decrement the privacy count;
	void dec_count() { 
		std::cout << "Called lah" << std::endl;
		pri_count--; 
	}

	bool operator<(const PEntry& other) const;
};

//}
}

#endif