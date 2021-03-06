#pragma once

#include "Vector.h"
#include "Error.h"
#include <algorithm>

namespace ax_gen {

// Ordered Dict
template<typename KEY, typename VALUE, typename IN_KEY = KEY>
class Dict : public NonCopyable {
public:
	using Key   = KEY;
	using InKey = IN_KEY;
	using Value = VALUE;

	~Dict() { clear(); }
	void clear();

			VALUE& operator[](int i)		{ return *_pairs[i].value; }
	const	VALUE& operator[](int i) const	{ return *_pairs[i].value; }

	int size() const { return _pairs.size(); }
	explicit operator bool() const { return size() != 0; }

	VALUE* getOrAdd(const InKey& key);
	VALUE* add(const InKey& key);
	VALUE* addIfNotExists(const InKey& key);

	class Pair : NonCopyable {
	public:
		Pair(const InKey& key_, Value* value_) : key(key_), value(value_) {}
		Pair(Pair && rhs) {
			value = nullptr;
			operator=(std::move(rhs));
		}
		~Pair() { delete value; }
		void operator=(Pair && rhs);

		bool operator<(const Pair& r) const { return key < r.key; }

		Key		key;
		Value*	value{nullptr};
	};

	class PairIterator {
	public:
		PairIterator(Pair* p = nullptr) : _p(p) {}

		operator const Pair&	()  { return *_p; }
		Pair&		operator*	()	{ return *_p; }
		void		operator++	()	{ ++_p; }
		bool		operator==	( const PairIterator & rhs )	{ return _p == rhs._p; }
		bool		operator!=	( const PairIterator & rhs )	{ return _p != rhs._p; }

	private:
		Pair* _p;
	};

	class PairEnumerator {
	public:
		PairEnumerator(Dict* map) : _map(map) {}
		PairIterator begin	() { return PairIterator(_map->_pairs.begin()); }
		PairIterator end	() { return PairIterator(_map->_pairs.end()); }

	private:
		Dict* _map;
	};

	PairEnumerator	pairs() { return PairEnumerator(this); }

	class KeyIterator {
	public:
		KeyIterator(Pair* p = nullptr) : _p(p) {}

		operator const Key&		()  { return _p->key; }
		Key&		operator*	()	{ return _p->key; }
		void		operator++	()	{ ++_p; }
		bool		operator==	( const KeyIterator & rhs )	{ return _p == rhs._p; }
		bool		operator!=	( const KeyIterator & rhs )	{ return _p != rhs._p; }

	private:
		Pair* _p;
	};

	class KeyEnumerator {
	public:
		KeyEnumerator(Dict* map) : _map(map) {}
		KeyIterator begin	() { return KeyIterator(_map->_pairs.begin()); }
		KeyIterator end		() { return KeyIterator(_map->_pairs.end()); }

	private:
		Dict* _map;
	};

	KeyEnumerator	keys()	{ return KeyEnumerator(this); }

	class ValueIterator {
	public:
		ValueIterator(Pair* p = nullptr) : _p(p) {}

		operator const Value&	()  { return *_p->value; }
		Value&		operator*	()	{ return *_p->value; }
		void		operator++	()	{ ++_p; }
		bool		operator==	( const ValueIterator & rhs )	{ return _p == rhs._p; }
		bool		operator!=	( const ValueIterator & rhs )	{ return _p != rhs._p; }

	private:
		Pair* _p;
	};

	class ValueEnumerator {
	public:
		ValueEnumerator(Dict* map) : _map(map) {}
		ValueIterator begin	() { return ValueIterator(_map->_pairs.begin()); }
		ValueIterator end	() { return ValueIterator(_map->_pairs.end()); }

	private:
		Dict* _map;
	};

	ValueEnumerator	values()	{ return ValueEnumerator(this); }

	ValueIterator begin	() { return ValueIterator(_pairs.data()); }
	ValueIterator end	() { return ValueIterator(_pairs.end()); }


	Value*	find(const InKey& key);
	bool	remove(const InKey& key);

	template<typename S>
	S& onStreamOut(S& s) {
		int i = 0;
		for (auto& it : pairs()) {
			if (i > 0) s << "\n" << std::setw(ax_dump_padding + 3) << " ";
			s << it.key << " : " << *it.value;
			i++;
		}
		return s;
	}

	void uniqueExtend(const Dict& r_) {
		auto& r = const_cast<Dict&>(r_);
		for (auto& p : r.pairs()) {
			auto* dst = addIfNotExists(p.key);
			if (!dst) continue;
			*dst = *p.value;
		}
	}
	
	void sort() { _pairs.sort(); }

	void operator=(const Dict& r) {
		clear();
		for (auto& e : r._pairs) {
			*add(e.key) = *e.value;
		}
	}

protected:
	Vector<Pair>			_pairs; // keep add order
	std::map<Key, Value*>	_map;
};

template<typename KEY, typename VALUE, typename IN_KEY /*= KEY*/>
VALUE* ax_gen::Dict<KEY, VALUE, IN_KEY>::getOrAdd(const InKey& key) {
	auto* o = find(key);
	return o ? o : add(key);
}

template<typename KEY, typename VALUE, typename IN_KEY>
inline std::ostream& operator<<(std::ostream& s, Dict<KEY, VALUE, IN_KEY>& v) {
	return v.onStreamOut(s); 
}

template<typename KEY, typename VALUE, typename IN_KEY> inline
bool Dict<KEY, VALUE, IN_KEY>::remove(const InKey& key) {
	auto it = _map.find(key);
	if (it == _map.end()) return false;
	_map.erase(it);


	int i = 0;
	for (auto& q : _pairs) {
		if (q.key == key) {
			_pairs.removeAt(i);
			return true;
		}
		i++;
	}
	throw Error("key is not found in Dict pairs");
}

template<typename KEY, typename VALUE, typename IN_KEY> inline
VALUE* Dict<KEY, VALUE, IN_KEY>::find(const InKey& key) {
	auto it = _map.find(key);
	if (it == _map.end()) return nullptr;
	return it->second;
}

template<typename KEY, typename VALUE, typename IN_KEY> inline
void Dict<KEY, VALUE, IN_KEY>::clear() {
	_pairs.clear();
	_map.clear();
}

template<typename KEY, typename VALUE, typename IN_KEY> inline
VALUE* Dict<KEY, VALUE, IN_KEY>::addIfNotExists(const InKey& key) {
	if (find(key)) return nullptr;
	return add(key);
}

template<typename KEY, typename VALUE, typename IN_KEY> inline
VALUE* Dict<KEY, VALUE, IN_KEY>::add(const InKey& key) {
	if (find(key)) throw Error("Dict has duplicated key");
	auto* v = new Value();
	_pairs.emplaceBack(key, v);
	_map[key] = v;
	return v;
}

template<typename KEY, typename VALUE, typename IN_KEY> inline
void Dict<KEY, VALUE, IN_KEY>::Pair::operator=(Pair && rhs) {
	delete value;
	key = std::move(rhs.key);
	value = rhs.value;
	rhs.value = nullptr;
}

template<typename VALUE>
using StringDict = Dict<String, VALUE, StrView>;


} //namespace
