#ifndef MANGOS_UTILITIES_H
#define MANGOS_UTILITIES_H



#include "Object.h"
#include "Map.h"

// old code
// VISIBILITY_RANGE = UPDATE_DISTANCE*UPDATE_DISTANCE = 155.8*155.8 = 24274
// New code
// VISIBILITY_RANGE = x_square + y_square + z_square where x,y,z is a quarter of the grid size.
#define VISIBILITY_RANGE    10000
#define SPIRIT_HEALER       5233

// 160*160
#define IN_LINE_OF_SIGHT         25600

namespace MaNGOS
{
    namespace Utilities
    {
    // mathematicall calculations
    inline float calculate_distance_square(const float &x1, const float &y1, const float &h1, const float &x2, const float &y2, const float &h2)
    {
        float x_p = (x2 - x1);
        float y_p = (y2 - y1);
        float h_p = (h2 - h1);
        x_p *= x_p;
        y_p *= y_p;
        h_p *= h_p;
        return (x_p + y_p + h_p);
    }

    // is in range only make sense for player since it    
    // deal with third person view where as is_in_line_of_sight deals with
    // first person.. which is the creatures and npcs
    inline bool is_in_range(Player *obj1, Object *obj2)
    {
        assert(obj1->GetMapId() == obj2->GetMapId());
        return (calculate_distance_square(obj1->GetPositionX(), obj1->GetPositionY(), obj1->GetPositionZ(), obj2->GetPositionX(), obj2->GetPositionY(), obj2->GetPositionZ()) < VISIBILITY_RANGE);
    }

    /** is_in_line_of_sight calculates where unit1 is in the line of sight of unit2.  Note the orientation
     * of unit 2 is no necessary cuz what unit1 facing determines he/she can see unit2
     */
    inline bool is_in_line_of_sight(const Unit &unit1, const Unit &unit2, const float off_set)
    {
        return( (calculate_distance_square(unit1.GetPositionX(), unit1.GetPositionY(), unit1.GetPositionZ(), unit2.GetPositionX(), unit2.GetPositionY(), unit2.GetPositionZ())*off_set) <= IN_LINE_OF_SIGHT );
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
