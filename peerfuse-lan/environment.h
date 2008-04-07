#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H
#include "environment_base.h"

class Environment : public EnvironmentBase
{
public:
	Environment() : merging(false) {}
	SharedVar<bool> merging;
};

extern Environment environment;
#endif /* ENVIRONMENT_H */
