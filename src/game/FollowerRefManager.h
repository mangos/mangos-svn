#ifndef _FOLLOWERREFMANAGER
#define _FOLLOWERREFMANAGER

#include "RefManager.h"

class Unit;
class TargetedMovementGenerator;

class FollowerRefManager : public RefManager<Unit, TargetedMovementGenerator>
{

};


#endif
