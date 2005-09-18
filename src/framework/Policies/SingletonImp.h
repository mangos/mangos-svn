#ifndef MANGOS_SINGLETONIMPL_H
#define MANGOS_SINGLETONIMPL_H

#include "Singleton.h"

// avoid the using namespace here cuz
// its a .h file afterall


template
<
    typename T,
    class ThreadingModel,
    class CreatePolicy,
    class LifeTimePolicy
>
T&
mangos::Singleton<T, ThreadingModel, CreatePolicy, LifeTimePolicy >::Instance()
{
    if( !si_instance )
    {
	// double-checked Locking pattern
	typename ThreadingModel::Lock lock;
	Guard(&lock);
	if( !si_instance )
	{
	    if( si_destroyed )
	    {
		LifeTimePolicy::OnDeadReference();
		si_destroyed = false;
	    }
	    si_instance = CreatePolicy::Create();
	    LifeTimePolicy::ScheduleCall(si_instance, &DestroySingleton);
	}
    }

    return *si_instance;
}

template
<
    typename T,
    class ThreadingModel,
    class CreatePolicy,
    class LifeTimePolicy
>
void
mangos::Singleton<T, ThreadingModel, CreatePolicy, LifeTimePolicy>::DestroySingleton()
{
    CreatePolicy::Destroy(si_instance);
    si_instance = NULL;
    si_destroyed = true;
}


#endif
