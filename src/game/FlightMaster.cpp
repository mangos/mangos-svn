

#include "FlightMaster.h"
#include "Policies/SingletonImp.h"

#define CLASS_LOCK MaNGOS::ClassLevelLockable<FlightMaster, ZThread::FastMutex>
INSTANTIATE_SINGLETON_2(FlightMaster, CLASS_LOCK);
INSTANTIATE_CLASS_MUTEX(FlightMaster, ZThread::FastMutex);
