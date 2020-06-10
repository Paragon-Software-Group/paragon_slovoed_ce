#ifndef SLD_STACK_H_
#define SLD_STACK_H_

#include "SldVector.h"

namespace sld2 {

// basically an std::stack clone
template <typename T, typename Container = CSldVector<T>>
class Stack
{
public:
	using container_type  = Container;
	using value_type      = typename Container::value_type;
	using size_type       = typename Container::size_type;
	using reference       = typename Container::reference;
	using const_reference = typename Container::const_reference;

	// element access
	reference top() { return c.back(); }
	const_reference top() const { return c.back(); }

	// capacity
	size_type size() const { return c.size(); }
	bool empty() const { return c.empty(); }

	// modifiers
	void clear() { c.clear(); }
	reference push(const_reference v) { return c.push_back(v); }
	reference push(value_type&& v) { return c.push_back(sld2::move(v)); }
	template <typename... Args>
	reference emplace(Args&&... args) { return c.emplace_back(sld2::forward<Args>(args)...); }
	void pop() { c.pop_back(); }

	value_type popv()
	{
		value_type v = sld2::move(top());
		pop();
		return sld2::move(v);
	}

private:
	Container c;
};

} // namespace sld2

#endif // SLD_STACK_H_
