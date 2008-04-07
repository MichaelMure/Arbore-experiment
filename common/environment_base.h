#ifndef ENVIRONMENT_BASE_H
#define ENVIRONMENT_BASE_H
#include "mutex.h"
#include "pf_types.h"

class EnvironmentBase
{
protected:
	template <typename A> class SharedVar : private Mutex
	{
		A var;
	public:
		SharedVar(A _var) : var(_var) {}
		~SharedVar() {}

		void Set(A _var)
		{
			BlockLockMutex lock(this);
			var = _var;
		}
		A Get()
		{
			BlockLockMutex lock(this);
			return var;
		}
	};
public:
	EnvironmentBase() : my_id(0), listening_port(0) {}
	SharedVar<pf_id> my_id;
	SharedVar<uint16_t> listening_port;
};

#endif /* ENVIRONMENT_BASE_H */
