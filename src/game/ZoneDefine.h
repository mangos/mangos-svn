/* ZoneDefine.h
 *
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MANGOS_ZONEDEFINE_H
#define MANGOS_ZONEDEFINE_H

#include "Zone.h"



enum zone_t
{
    ZONE_38 = 38,
    ZONE_1519 = 1519,
    ZONE_1537 = 1537,
    ZONE_1377 = 1377,
    ZONE_3358 = 3358,
    ZONE_141 = 141,
    ZONE_215 = 215,
    ZONE_2597 = 2597,
    ZONE_148 = 148,
    ZONE_361 = 361,
    ZONE_40 = 40,
    ZONE_41 = 41,
    ZONE_44 = 44,
    ZONE_28 = 28,
    ZONE_618 = 618,
    ZONE_45 = 45,
    ZONE_46 = 46,
    ZONE_490 = 490,
    ZONE_47 = 47,
    ZONE_493 = 493,
    ZONE_85 = 85,
    ZONE_1637 = 1637,
    ZONE_1638 = 1638,
    ZONE_3277 = 3277,
    ZONE_1657 = 1657,
    ZONE_130 = 130,
    ZONE_1497 = 1497,
    ZONE_400 = 400,
    ZONE_1 = 1,
    ZONE_331 = 331,
    ZONE_3 = 3,
    ZONE_405 = 405,
    ZONE_4 = 4,
    ZONE_10 = 10,
    ZONE_11 = 11,
    ZONE_406 = 406,
    ZONE_139 = 139,
    ZONE_12 = 12,
    ZONE_440 = 440,
    ZONE_14 = 14,
    ZONE_8 = 8,
    ZONE_15 = 15,
    ZONE_16 = 16,
    ZONE_17 = 17,
    ZONE_33 = 33,
    ZONE_267 = 267,
    ZONE_357 = 357,
    ZONE_51 = 51,
    ZONE_36 = 36,
};


template<zone_t T>
struct ZoneDefinition
{
};

template<> struct ZoneDefinition<ZONE_38>
{
    static Zone* Create(void)
    {
        return( new Zone(-1993.74987792969, -4752.0830078125, -4487.5, -6327.0830078125) );
    }
};

template<> struct ZoneDefinition<ZONE_1519>
{
    static Zone* Create(void)
    {
        return( new Zone(1380.97143554688, 36.7006301879883, -8278.8505859375, -9175.205078125) );
    }
};

template<> struct ZoneDefinition<ZONE_1537>
{
    static Zone* Create(void)
    {
        return( new Zone(-713.591369628906, -1504.21643066406, -4569.2412109375, -5096.845703125) );
    }
};

template<> struct ZoneDefinition<ZONE_1377>
{
    static Zone* Create(void)
    {
        return( new Zone(4641.66650390625, -2308.33325195312, -5800, -10433.3330078125) );
    }
};

template<> struct ZoneDefinition<ZONE_3358>
{
    static Zone* Create(void)
    {
        return( new Zone(1858.33325195312, 102.08332824707, 1508.33325195312, 337.5) );
    }
};

template<> struct ZoneDefinition<ZONE_141>
{
    static Zone* Create(void)
    {
        return( new Zone(3814.58325195312, -1277.08325195312, 11831.25, 8437.5) );
    }
};

template<> struct ZoneDefinition<ZONE_215>
{
    static Zone* Create(void)
    {
        return( new Zone(2047.91662597656, -3089.58325195312, -272.916656494141, -3697.91650390625) );
    }
};

template<> struct ZoneDefinition<ZONE_2597>
{
    static Zone* Create(void)
    {
        return( new Zone(1781.24987792969, -2456.25, 1085.41662597656, -1739.58325195312) );
    }
};

template<> struct ZoneDefinition<ZONE_148>
{
    static Zone* Create(void)
    {
        return( new Zone(2941.66650390625, -3608.33325195312, 8333.3330078125, 3966.66650390625) );
    }
};

template<> struct ZoneDefinition<ZONE_361>
{
    static Zone* Create(void)
    {
        return( new Zone(1641.66662597656, -4108.3330078125, 7133.3330078125, 3299.99975585938) );
    }
};

template<> struct ZoneDefinition<ZONE_40>
{
    static Zone* Create(void)
    {
        return( new Zone(3016.66650390625, -483.333312988281, -9400, -11733.3330078125) );
    }
};

template<> struct ZoneDefinition<ZONE_41>
{
    static Zone* Create(void)
    {
        return( new Zone(-833.333312988281, -3333.33325195312, -9866.666015625, -11533.3330078125) );
    }
};

template<> struct ZoneDefinition<ZONE_44>
{
    static Zone* Create(void)
    {
        return( new Zone(-1570.83325195312, -3741.66650390625, -8575, -10022.916015625) );
    }
};

template<> struct ZoneDefinition<ZONE_28>
{
    static Zone* Create(void)
    {
        return( new Zone(416.666656494141, -3883.33325195312, 3366.66650390625, 499.999969482422) );
    }
};

template<> struct ZoneDefinition<ZONE_618>
{
    static Zone* Create(void)
    {
        return( new Zone(-316.666656494141, -7416.66650390625, 8533.3330078125, 3799.99975585938) );
    }
};

template<> struct ZoneDefinition<ZONE_45>
{
    static Zone* Create(void)
    {
        return( new Zone(-866.666625976562, -4466.66650390625, -133.33332824707, -2533.33325195312) );
    }
};

template<> struct ZoneDefinition<ZONE_46>
{
    static Zone* Create(void)
    {
        return( new Zone(-266.666656494141, -3195.83325195312, -7031.24951171875, -8983.3330078125) );
    }
};

template<> struct ZoneDefinition<ZONE_490>
{
    static Zone* Create(void)
    {
        return( new Zone(533.333312988281, -3166.66650390625, -5966.66650390625, -8433.3330078125) );
    }
};

template<> struct ZoneDefinition<ZONE_47>
{
    static Zone* Create(void)
    {
        return( new Zone(-1575, -5425, 1466.66662597656, -1100) );
    }
};

template<> struct ZoneDefinition<ZONE_493>
{
    static Zone* Create(void)
    {
        return( new Zone(-1381.25, -3689.58325195312, 8491.666015625, 6952.0830078125) );
    }
};

template<> struct ZoneDefinition<ZONE_85>
{
    static Zone* Create(void)
    {
        return( new Zone(3033.33325195312, -1485.41662597656, 3837.49975585938, 824.999938964844) );
    }
};

template<> struct ZoneDefinition<ZONE_1637>
{
    static Zone* Create(void)
    {
        return( new Zone(-3680.60107421875, -5083.20556640625, 2273.87719726562, 1338.46057128906) );
    }
};

template<> struct ZoneDefinition<ZONE_1638>
{
    static Zone* Create(void)
    {
        return( new Zone(516.666625976562, -527.083312988281, -849.999938964844, -1545.83325195312) );
    }
};

template<> struct ZoneDefinition<ZONE_3277>
{
    static Zone* Create(void)
    {
        return( new Zone(2041.66662597656, 895.833312988281, 1627.08325195312, 862.499938964844) );
    }
};

template<> struct ZoneDefinition<ZONE_1657>
{
    static Zone* Create(void)
    {
        return( new Zone(2938.36279296875, 1880.02954101562, 10238.31640625, 9532.5869140625) );
    }
};

template<> struct ZoneDefinition<ZONE_130>
{
    static Zone* Create(void)
    {
        return( new Zone(3449.99975585938, -750, 1666.66662597656, -1133.33325195312) );
    }
};

template<> struct ZoneDefinition<ZONE_1497>
{
    static Zone* Create(void)
    {
        return( new Zone(873.192626953125, -86.1824035644531, 1877.9453125, 1237.84118652344) );
    }
};

template<> struct ZoneDefinition<ZONE_400>
{
    static Zone* Create(void)
    {
        return( new Zone(-433.333312988281, -4833.3330078125, -3966.66650390625, -6899.99951171875) );
    }
};

template<> struct ZoneDefinition<ZONE_1>
{
    static Zone* Create(void)
    {
        return( new Zone(1802.08325195312, -3122.91650390625, -3877.08325195312, -7160.41650390625) );
    }
};

template<> struct ZoneDefinition<ZONE_331>
{
    static Zone* Create(void)
    {
        return( new Zone(1699.99987792969, -4066.66650390625, 4672.91650390625, 829.166625976562) );
    }
};

template<> struct ZoneDefinition<ZONE_3>
{
    static Zone* Create(void)
    {
        return( new Zone(-2079.16650390625, -4566.66650390625, -5889.5830078125, -7547.91650390625) );
    }
};

template<> struct ZoneDefinition<ZONE_405>
{
    static Zone* Create(void)
    {
        return( new Zone(4233.3330078125, -262.5, 452.083312988281, -2545.83325195312) );
    }
};

template<> struct ZoneDefinition<ZONE_4>
{
    static Zone* Create(void)
    {
        return( new Zone(-1241.66662597656, -4591.66650390625, -10566.666015625, -12800) );
    }
};

template<> struct ZoneDefinition<ZONE_10>
{
    static Zone* Create(void)
    {
        return( new Zone(833.333312988281, -1866.66662597656, -9716.666015625, -11516.666015625) );
    }
};

template<> struct ZoneDefinition<ZONE_11>
{
    static Zone* Create(void)
    {
        return( new Zone(-389.583312988281, -4525, -2147.91650390625, -4904.16650390625) );
    }
};

template<> struct ZoneDefinition<ZONE_406>
{
    static Zone* Create(void)
    {
        return( new Zone(3245.83325195312, -1637.49987792969, 2916.66650390625, -339.583312988281) );
    }
};

template<> struct ZoneDefinition<ZONE_139>
{
    static Zone* Create(void)
    {
        return( new Zone(-2185.41650390625, -6056.25, 3799.99975585938, 1218.75) );
    }
};

template<> struct ZoneDefinition<ZONE_12>
{
    static Zone* Create(void)
    {
        return( new Zone(1535.41662597656, -1935.41662597656, -7939.5830078125, -10254.166015625) );
    }
};

template<> struct ZoneDefinition<ZONE_440>
{
    static Zone* Create(void)
    {
        return( new Zone(-218.749984741211, -7118.74951171875, -5875, -10475) );
    }
};

template<> struct ZoneDefinition<ZONE_14>
{
    static Zone* Create(void)
    {
        return( new Zone(-1962.49987792969, -7249.99951171875, 1808.33325195312, -1716.66662597656) );
    }
};

template<> struct ZoneDefinition<ZONE_8>
{
    static Zone* Create(void)
    {
        return( new Zone(-2222.91650390625, -4516.66650390625, -9620.8330078125, -11150) );
    }
};

template<> struct ZoneDefinition<ZONE_15>
{
    static Zone* Create(void)
    {
        return( new Zone(-974.999938964844, -6225, -2033.33325195312, -5533.3330078125) );
    }
};

template<> struct ZoneDefinition<ZONE_16>
{
    static Zone* Create(void)
    {
        return( new Zone(-3277.08325195312, -8347.916015625, 5341.66650390625, 1960.41662597656) );
    }
};

template<> struct ZoneDefinition<ZONE_17>
{
    static Zone* Create(void)
    {
        return( new Zone(2622.91650390625, -7510.41650390625, 1612.49987792969, -5143.75) );
    }
};

template<> struct ZoneDefinition<ZONE_33>
{
    static Zone* Create(void)
    {
        return( new Zone(2220.83325195312, -4160.41650390625, -11168.75, -15422.916015625) );
    }
};

template<> struct ZoneDefinition<ZONE_267>
{
    static Zone* Create(void)
    {
        return( new Zone(1066.66662597656, -2133.33325195312, 400, -1733.33325195312) );
    }
};

template<> struct ZoneDefinition<ZONE_357>
{
    static Zone* Create(void)
    {
        return( new Zone(5441.66650390625, -1508.33325195312, -2366.66650390625, -6999.99951171875) );
    }
};

template<> struct ZoneDefinition<ZONE_51>
{
    static Zone* Create(void)
    {
        return( new Zone(-322.916656494141, -2554.16650390625, -6100, -7587.49951171875) );
    }
};

template<> struct ZoneDefinition<ZONE_36>
{
    static Zone* Create(void)
    {
        return( new Zone(783.333312988281, -2016.66662597656, 1500, -366.666656494141) );
    }
};

#endif
