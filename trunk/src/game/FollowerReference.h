#ifndef _FOLLOWERREFERENCE_H
#define _FOLLOWERREFERENCE_H

#include "Reference.h"

class TargetedMovementGenerator;
class Unit;

class MANGOS_DLL_SPEC FollowerReference : public Reference<Unit, TargetedMovementGenerator>
{
protected:
    void targetObjectBuildLink();
    void targetObjectDestroyLink();
    void sourceObjectDestroyLink();
};

#endif
