/* ItemPrototype.h
 *
 * Copyright (C) 2004 Wow Daemon
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

#ifndef _ITEMPROTOTYPE_H
#define _ITEMPROTOTYPE_H

#include "Common.h"

enum ITEM_STAT_TYPE
{
    HEALTH     = 1,
    UNKNOWN    = 2,
    AGILITY    = 3,
    STRENGHT   = 4,
    INTELLECT  = 5,
    SPIRIT     = 6,
    STAMINA    = 7,
};

enum ITEM_DAMAGE_TYPE
{
    NORMAL_DAMAGE  = 0,
    HOLY_DAMAGE    = 1,
    FIRE_DAMAGE    = 2,
    NATURE_DAMAGE  = 3,
    FROST_DAMAGE   = 4,
    SHADOW_DAMAGE  = 5,
    ARCANE_DAMAGE  = 6,
};

enum ITEM_SPELLTRIGGER_TYPE
{
    USE           = 0,
    ON_EQUIP      = 1,
    CHANCE_ON_HIT = 2,
    SOULSTONE     = 4,
};

enum ITEM_BONDING_TYPE
{
    NO_BIND             = 0,
    BIND_WHEN_PICKED_UP = 1,
    BIND_WHEN_EQUIPED   = 2,
};

enum INVENTORY_TYPES
{
    INVTYPE_NON_EQUIP      = 0x0,
    INVTYPE_HEAD           = 0x1,
    INVTYPE_NECK           = 0x2,
    INVTYPE_SHOULDERS      = 0x3,
    INVTYPE_BODY           = 0x4,
    INVTYPE_CHEST          = 0x5,
    INVTYPE_WAIST          = 0x6,
    INVTYPE_LEGS           = 0x7,
    INVTYPE_FEET           = 0x8,
    INVTYPE_WRISTS         = 0x9,
    INVTYPE_HANDS          = 0xa,
    INVTYPE_FINGER         = 0xb,
    INVTYPE_TRINKET        = 0xc,
    INVTYPE_WEAPON         = 0xd,
    INVTYPE_SHIELD         = 0xe,
    INVTYPE_RANGED         = 0xf,
    INVTYPE_CLOAK          = 0x10,
    INVTYPE_2HWEAPON       = 0x11,
    INVTYPE_BAG            = 0x12,
    INVTYPE_TABARD         = 0x13,
    INVTYPE_ROBE           = 0x14,
    INVTYPE_WEAPONMAINHAND = 0x15,
    INVTYPE_WEAPONOFFHAND  = 0x16,
    INVTYPE_HOLDABLE       = 0x17,
    INVTYPE_AMMO           = 0x18,
    INVTYPE_THROWN         = 0x19,
    INVTYPE_RANGEDRIGHT    = 0x1a,
	INVTYPE_SLOT_ITEM      = 0x1b,
    NUM_INVENTORY_TYPES    = 0x1c,
};

struct ItemPrototype
{
    uint32 ItemId;
    uint32 Class;
    uint32 SubClass;
    std::string Name1;
    std::string Name2;
    std::string Name3;
    std::string Name4;
    uint32 DisplayInfoID;
    uint32 Quality;
    uint32 Flags;
    uint32 BuyPrice;
    uint32 SellPrice;
    uint32 InventoryType;
    uint32 AllowableClass;
    uint32 AllowableRace;
    uint32 ItemLevel;
    uint32 RequiredLevel;
    uint32 RequiredSkill;
    uint32 RequiredSkillRank;
    uint32 Field20;
    uint32 Field21;
    uint32 Field22;
    uint32 Field23;
    uint32 MaxCount;
    uint32 ContainerSlots;
    uint32 ItemStatType[10];
    uint32 ItemStatValue[10];
    float DamageMin[6];
    float DamageMax[6];
    uint32 DamageType[6];
    uint32 Armor;
    uint32 HolyRes;
    uint32 FireRes;
    uint32 NatureRes;
    uint32 FrostRes;
    uint32 ShadowRes;
    uint32 ArcaneRes;
    uint32 Delay;
    uint32 Field69;
    uint32 SpellId[5];
    uint32 SpellTrigger[5];
    uint32 SpellCharges[5];
    uint32 SpellCooldown[5];
    uint32 SpellCategory[5];
    uint32 SpellCategoryCooldown[5];
    uint32 Bonding;
    std::string Description;
    uint32 Field102;
    uint32 Field103;
    uint32 Field104;
    uint32 Field105;
    uint32 Field106;
    uint32 Field107;
    uint32 Field108;
    uint32 Field109;
    uint32 Field110;
    uint32 Field111;
    uint32 MaxDurability;
};
#endif
