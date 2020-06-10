#pragma once

#include <cassert>
#include <cstddef>
#include <type_traits>

#include "util.h"

// kernel style intrusive list; adapated to c++ (somewhat)
namespace sld {

// should be embedded inside the relevant struct/class as a *public* member
struct list_node
{
	list_node *next;
	list_node *prev;

	list_node();
};

// "initializes" the list to an "empty" state
static inline void make_empty(list_node *node) {
	node->next = node;
	node->prev = node;
}

// removes the list_node from the parent list
static inline void unlink(list_node *node) {
	node->prev->next = node->next;
	node->next->prev = node->prev;
	node->prev = node->next = node;
}
static inline void unlink(list_node &node) { unlink(&node); }

inline list_node::list_node() { make_empty(this); }

// helper for accessing the node/its containing object
namespace detail {

template<typename T, typename M, M T::* member>
struct member_access {
	static const T* container_of(const M *m) {
		return (const T*)((const char*)(m) - offset());
	}
	static T* container_of(M *m) {
		return (T*)((char*)(m) - offset());
	}
	static const M* member_of(const T *o) { return &(o->*member); }
	static M* member_of(T *o) { return &(o->*member); }
private:
	static inline size_t offset() { return (size_t)((char*)(&(((T*)nullptr)->*member))); }
};

} // namespace detail

// the actual list "head"
template <typename T, list_node T::* Node>
class simple_list
{
	typedef detail::member_access<T, list_node, Node> access;

	// base list iterator
	template <bool isConst = true>
	struct iterator_base {
		typedef typename std::conditional<isConst, const list_node, list_node>::type node_type;
		typedef typename std::conditional<isConst, const T, T>::type value_type;

		iterator_base(node_type *node) : node(node) {}
		iterator_base(const iterator_base<false>& other) : node(other.node) {}
		bool operator!=(const iterator_base& other) const {
			return this->node != other.node;
		}
		iterator_base& operator++() {
			node = node->next;
			return *this;
		}
		value_type* operator*() const {
			return access::container_of(node);
		}

		friend struct iterator_base<true>;
	private:
		node_type *node;
	};

public:
	typedef iterator_base<false> iterator;
	typedef iterator_base<true> const_iterator;

	void push_front(T *o) {
		list_node *node = access::member_of(o);
		node->prev = &root;
		node->next = root.next;
		root.next->prev = node;
		root.next = node;
	}
	void push_front(T &o) { push_front(&o); }

	void push_back(T *o) {
		list_node *node = access::member_of(o);
		node->next = &root;
		node->prev = root.prev;
		root.prev->next = node;
		root.prev = node;
	}
	void push_back(T &o) { push_back(&o); }

	T* pop_front() {
		list_node *node = root.next;
		unlink(node);
		return access::container_of(node);
	}

	T* pop_back() {
		list_node *node = root.prev;
		unlink(node);
		return access::container_of(node);
	}

	void move_to_front(T *o) {
		list_node *node = access::member_of(o);
		unlink(node);
		push_front(o);
	}
	void move_to_front(T &o) { move_to_front(&o); }

	void move_to_back(T *o) {
		list_node *node = access::member_of(o);
		unlink(node);
		push_back(o);
	}
	void move_to_back(T &o) { move_to_back(&o); }

	void remove(T *o) { unlink(access::member_of(o)); }
	void remove(T &o) { remove(&o); }

	void clear() { make_empty(&root); }

	template <typename Cb>
	void clear(Cb cb);

	bool empty() const { return root.next == &root; }

	size_t size() const {
		size_t length = 0;
		for (const list_node *node = root.next; node != &root; node = node->next)
			length++;
		return length;
	}

	T* front() { return access::container_of(root.next); }
	T* back() { return access::container_of(root.prev); }

	const T* front() const { return access::container_of(root.next); }
	const T* back() const { return access::container_of(root.prev); }

	// range based for-loop support
	iterator begin() { return iterator(root.next); }
	iterator end() { return iterator(&root); }
	const_iterator begin() const { return const_iterator(root.next); }
	const_iterator end() const { return const_iterator(&root); }

	// root accessors
	list_node& _root() { return root; }
	const list_node& _root() const { return root; }
private:
	list_node root;
};

namespace detail {
// "safe" iterator - can safely unlink/delete the *curent* node
template <typename T, list_node T::* Node>
struct safe_iterator
{
	typedef detail::member_access<T, list_node, Node> access;

	safe_iterator(list_node *node) : node(node), temp(node->next) {}
	bool operator!=(const safe_iterator& other) const {
		return node != other.node;
	}
	safe_iterator& operator++() {
		node = temp;
		temp = temp->next;
		return *this;
	}
	T* operator*() const {
		return access::container_of(node);
	}
private:
	list_node *node, *temp;
};
} // namespace detail

// safely iterate over a list (as in the current node can be safely manipulated with)
template <typename T, list_node T::* Node, typename It = detail::safe_iterator<T, Node>>
inline RangeImpl<It> safe_iterate(simple_list<T, Node> &list) {
	return{ It(list._root().next), It(&list._root()) };
}

// clear with a callback
template<typename T, list_node T::* Node>
template<typename Cb>
inline void simple_list<T, Node>::clear(Cb cb)
{
	for (T* o : safe_iterate(*this))
		cb(o);
	clear();
}

// pretty simple validation routine, really does anything only in debug builds
template <typename T, list_node T::* Node>
inline void validate(const simple_list<T, Node> &list) {
	const list_node &root = list._root();
	assert(root.next->prev == &root && root.prev->next == &root);
	for (const list_node *node = root.next; node != &root; node = node->next)
		assert(node->next->prev == node && node->prev->next == node);
}

} // namespace sld