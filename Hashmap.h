#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <initializer_list>
#include <stdexcept>

using namespace std;

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> > class HashMap{
	using KeyValue = typename std::pair<KeyType, ValueType>;
	using ConstKeyValue = typename std::pair<const KeyType, ValueType>;
	using Iter = typename std::vector<KeyValue>::iterator;
	using ConstIter = typename std::vector<KeyValue>::const_iterator;

	size_t size_hashmap;
	Hash hasher;
	std::vector<int> hashmap;
	std::vector<KeyValue> elements; // elements[i] - pair<Key, Value>
	//hashmap[i] = (-1 if hash i was deleted); (0 if hash is empty); (k > 0 - hash matches elemets[k - 1])
public:

	constexpr static size_t cnst_for_size = 6;
	constexpr static size_t p_for_SecondHash = 19;
	constexpr static size_t mod_for_SecondHash = 500009;

	size_t GetSecondHash(size_t hash) const {
		size_t ans = 0;
		while (hash > 0) {
			ans *= p_for_SecondHash;
			ans += (hash & 15);
			ans %= mod_for_SecondHash;
			hash >>= 4;
		}
		ans %= size_hashmap;
		if (ans % 2 == 0) {
			++ans;
		}
		return ans;
	}

	class iterator {
		friend HashMap;

		Iter cur;
		iterator(Iter f) : cur(f) {}
	public:	
		iterator() = default;

		iterator operator++ (int) {
			Iter old_iter = cur;
			++cur;
			return iterator(old_iter);
		}
		iterator& operator++ () {
			++cur;
			return *this;
		}

		bool operator!= (const iterator& it) const { return cur != it.cur; }
		bool operator== (const iterator& it) const { return cur == it.cur; }

		KeyValue& operator* () { return *cur; }
		ConstKeyValue* operator->() { return reinterpret_cast<ConstKeyValue*>(&(*cur)); }

	};
	iterator begin() { return iterator(elements.begin()); }
	iterator end() { return iterator(elements.end()); }

	class const_iterator {
		friend HashMap;

		ConstIter cur;
		const_iterator(const ConstIter f) : cur(f) {}
	public:
		const_iterator() = default;

		const_iterator operator++ (int) {
			ConstIter old_iter = cur;
			++cur;
			return const_iterator(old_iter);
		}
		const_iterator& operator++ () {
			++cur;
			return *this;
		}

		bool operator!= (const const_iterator& it) const { return cur != it.cur; }
		bool operator== (const const_iterator& it) const { return cur == it.cur; }

		const KeyValue& operator* () { return *cur; }
		const KeyValue* operator->() { return (&(*cur)); }
	};
	const_iterator begin() const { return const_iterator(elements.cbegin()); }
	const_iterator end() const { return const_iterator(elements.cend()); }

	void Build() {
		size_hashmap = 8;
		hashmap.clear();
		hashmap.resize(size_hashmap);
		elements.clear();
	}

	Hash hash_function() const {
		return hasher;
	}

	size_t GetHash(KeyType a) const {
		return hasher(a) % size_hashmap;
	}
	size_t size() const {
		return elements.size();
	}
	bool empty() const {
		return elements.empty();
	}

	HashMap(const Hash& hasher_ = Hash()) : hasher(hasher_) {
		Build();
	}
	template<typename It>
	HashMap(It begin, It end, const Hash& hasher_ = Hash()) : hasher(hasher_) {
		Build();
		while (begin != end) {
			insert(*begin);
			++begin;	
		}
	}
	HashMap(const std::initializer_list<KeyValue>& lst, const Hash& hasher_ = Hash()) : hasher(hasher_) {
		Build();
		for (const KeyValue& x : lst) {
			insert(x);
		}
	}

	KeyType GetKey(size_t nowhash) const {
		return elements[hashmap[nowhash] - 1].first;
	}
	ValueType GetValue(size_t nowhash) const {
		return elements[hashmap[nowhash] - 1].second;
	}
	KeyValue GetPair(size_t nowhash) const {
		return elements[hashmap[nowhash] - 1];
	}
	size_t GetIndex(size_t nowhash) const {
		return hashmap[nowhash] - 1;
	}

	size_t IncreaseNowhash(size_t nowhash, size_t del) const {
		nowhash += del;
		if (nowhash >= size_hashmap) {
			nowhash -= size_hashmap;
		}
		return nowhash;
	}
	bool Upgrade() {
		if (elements.size() * cnst_for_size < size_hashmap) {
			return 0;
		}

		size_hashmap *= 2;
		hashmap.clear();
		hashmap.resize(size_hashmap);

		for (size_t i = 0; i < elements.size(); ++i) {
			size_t nowhash = GetHash(elements[i].first);
			size_t del = GetSecondHash(nowhash);
			while (hashmap[nowhash] > 0) {
				nowhash = IncreaseNowhash(nowhash, del);
			}
			hashmap[nowhash] = i + 1;
		}
		return 1;
	}
	void insert(const KeyValue& element) {
		size_t nowhash = GetHash(element.first);
		size_t del = GetSecondHash(nowhash);
		while (hashmap[nowhash] > 0) {
			if (GetKey(nowhash) == element.first) {
				return;
			}
			nowhash = IncreaseNowhash(nowhash, del);
		}
		elements.push_back(element);
		hashmap[nowhash] = elements.size();
		Upgrade();
	}
	void Delete(size_t nowhash) {
		std::swap(elements[hashmap[nowhash] - 1], elements.back());
		elements.pop_back();
		hashmap[nowhash] = -1;
	}
	void erase(const KeyType& key) {
		size_t nowhash = GetHash(key);
		size_t del = GetSecondHash(nowhash);
		while (hashmap[nowhash]) {
			if (hashmap[nowhash] > 0 && GetKey(nowhash) == key) {
				size_t last_hash = GetHash(elements.back().first);
				size_t last_del = GetSecondHash(last_hash);
				while (hashmap[last_hash]) {
					if (hashmap[last_hash] == static_cast<int>(elements.size())) {
						std::swap(elements[hashmap[nowhash] - 1], elements.back());
						elements.pop_back();
						hashmap[last_hash] = hashmap[nowhash];
						hashmap[nowhash] = -1;
						break;
					}	
					last_hash = IncreaseNowhash(last_hash, last_del);
				}
				return;
			}
			nowhash = IncreaseNowhash(nowhash, del);
		}
	}
	ValueType& operator [] (const KeyType& key) {
		size_t nowhash = GetHash(key);
		size_t del = GetSecondHash(nowhash);
		while (hashmap[nowhash]) {
			if (hashmap[nowhash] > 0 && GetKey(nowhash) == key) {
				return elements[hashmap[nowhash] - 1].second;
			}
			nowhash = IncreaseNowhash(nowhash, del);
		}
		elements.push_back(KeyValue({key, ValueType()}));
		hashmap[nowhash] = elements.size();
		if (Upgrade()) {
			return find(key) -> second;
		} else {
			return elements.back().second;
		}
	}
	iterator find(const KeyType& key) {
		size_t nowhash = GetHash(key);
		size_t del = GetSecondHash(nowhash);
		while (hashmap[nowhash]) {
			if (hashmap[nowhash] > 0 && GetKey(nowhash) == key) {
				return iterator(elements.begin() + GetIndex(nowhash));
			}
			nowhash = IncreaseNowhash(nowhash, del);
		}
		return end();
	}
	const_iterator find(const KeyType& key) const {
		size_t nowhash = GetHash(key);
		size_t del = GetSecondHash(nowhash);
		while (hashmap[nowhash] > 0 && hashmap[nowhash]) {
			if (GetKey(nowhash) == key) {
				return const_iterator(elements.cbegin() + GetIndex(nowhash));
			}
			nowhash = IncreaseNowhash(nowhash, del);
		}
		return end();
	}
	const ValueType& at (const KeyType& key) const {
		const_iterator iter_key = find(key);
		if (iter_key == end()) {
			throw std::out_of_range("element not in HashTable");
		}
		return iter_key -> second;
	}
	void clear() {
		Build();
	}
};
