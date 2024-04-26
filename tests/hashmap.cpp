
#include <cstdio>

#include "../hashmap.hpp"
#include <unordered_map>

struct allocator {
	allocator(uint64_t count) {
		this->count = count;
		nodes = new T*[count];
		all = new T[count];
		for(uint64_t i=0; i<count; ++i) {
			nodes[i] = &(all[i]);
		}
		head = 0;
		tail = 0;
	}
	
	~allocator() {
		delete[] all;
	}
	
	using T = concurrent::mpmc::kp<uint64_t, uint64_t>
		::hashmap<concurrent::default_hash::hash<uint64_t>, 64>::Node;
	
	T* allocate() {
		uint64_t id = (head++)%count;
		return nodes[id];
	}
	
	void free(T* ptr) {
		uint64_t id = (tail++)%count;
		nodes[id] = ptr;
	}
	
	T* all;
	T** nodes;
	uint64_t count;
	std::atomic<uint64_t> head, tail;
} _def_alloc(1024*1024*256/512);

concurrent::mpmc::kp<uint64_t, uint64_t>
	::hashmap<concurrent::default_hash::hash<uint64_t>, 64, allocator::T, allocator> hashmap(16777619, &_def_alloc);
std::unordered_map<uint64_t, uint64_t> stdmap;

void add(uint64_t key, uint64_t value) {
	if(key&1) {
		uint64_t v;
		if(hashmap.try_get(key, v) == false)
			hashmap.try_put_new(key, value);
	}
	stdmap[key] = value;
	hashmap.try_set(key, value);
}

void remove(uint64_t key) {
	stdmap.erase(key);
	hashmap.remove(key);
}

bool get(uint64_t key) {
	auto it = stdmap.find(key);
	if(it == stdmap.end()) {
		uint64_t v;
		if(hashmap.try_get(key, v) == false)
			return true;
	} else {
		uint64_t v;
		if(hashmap.try_get(key, v) == false)
			return false;
		if(v == it->second)
			return true;
		return false;
	}
	return false;
}

bool __false(int line) {
	printf(" line = %i\n", line);
	return false;
}

#define FALSE __false(__LINE__)

bool compare() {
	uint64_t size = 0;
	uint64_t v;
	uint64_t nid=~0;
	bool failed = false;
	hashmap.__debug_foreach([&](uint64_t k, uint64_t val) {
				++size;
				auto it = stdmap.find(k);
				if(it == stdmap.end()) {
					failed = true;
					return FALSE;
				}
				if(k != it->first) {
					failed = true;
					return FALSE;
				}
				nid=val-it->second;
				if(val - it->second && nid) {
					printf(" %lu != %lu, diff = %lu\n", val, it->second, nid);
					failed = true;
					return FALSE;
				}
				return true;
			});
	if(failed)
		return FALSE;
	if(size != stdmap.size()) {
		printf(" sizes: %lu != %lu\n", size, stdmap.size());
		return FALSE;
	}
	for(auto it=stdmap.begin(); it!=stdmap.end(); ++it) {
		if(hashmap.try_get(it->first, v)) {
			if(v != it->second)
				return FALSE;
		} else {
			return FALSE;
		}
	}
	return true;
}

void verify() {
	if(compare() == false)
		printf("   FAILED!!!\n");
	else
		printf("   OK\n");
}

int main() {
	int mult = 10000000/1000;
	
	for(int i=0; i<10*mult; ++i) {
		uint64_t k=random(), v=random();
		add(k, v);
	}
	verify();
	
	for(int i=0; i<3*mult; ++i) {
		remove(stdmap.begin()->first);
		remove(stdmap.cbegin()->first);
	}
	verify();
	
	for(int i=0; i<10*mult; ++i) {
		uint64_t k=random(), v=random();
		add(k, v);
	}
	verify();
	
	for(int i=0; i<5*mult; ++i) {
		remove(random());
	}
	verify();
	
	return compare() ? 0 : 1;
}


