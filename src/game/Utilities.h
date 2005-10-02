#ifndef MANGOS_UTILITIES_H
#define MANGOS_UTILITIES_H



#include "Object.h"
#include "Map.h"

// old code
// VISIBILITY_RANGE = UPDATE_DISTANCE*UPDATE_DISTANCE = 155.8*155.8 = 24274
#define VISIBILITY_RANGE    24274
#define SPIRIT_HEALER       5233

namespace MaNGOS
{
    namespace Utilities
    {
	// mathematicall calculations
	inline float calculate_distance(const float &x1, const float &y1, const float &x2, const float &y2, const float &h1, const float &h2)
	{
	    float x_p = (x2 - x1);
	    float y_p = (y2 - y1);
	    float h_p = (h2 - h1);
	    x_p *= x_p;
	    y_p *= y_p;
	    h_p *= h_p;
	    return sqrt(x_p + y_p + h_p);
	}
	
	inline bool is_in_range(Object *obj1, Object *obj2)
	{
	    assert(obj1->GetMapId() == obj2->GetMapId());
	    float dx  = obj2->GetPositionX() - obj1->GetPositionX();
	    float dy  = obj2->GetPositionY() - obj1->GetPositionY();
	    return( ((dx*dx) + (dy*dy)) <= VISIBILITY_RANGE );
	}

	// helpful functions determining if its a spirit healer
	template<class T> inline bool IsSpiritHealer(T *obj)
	{
	    return false;
	}

	template<> inline bool IsSpiritHealer(Creature *obj)
	{
	    return ( obj->GetUInt32Value(UNIT_FIELD_DISPLAYID) == SPIRIT_HEALER );
	}
    }
}

#endif
