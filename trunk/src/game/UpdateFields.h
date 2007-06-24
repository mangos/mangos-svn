/*
 * Copyright (C) 2005,2006,2007 MaNGOS <http://www.mangosproject.org/>
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

#include "Common.h"

#ifndef _UPDATEFIELDS_AUTO_H
#define _UPDATEFIELDS_AUTO_H

enum EObjectFields
{
    OBJECT_FIELD_GUID                          = 0x00, // Size:2
    OBJECT_FIELD_TYPE                          = 0x02, // Size:1
    OBJECT_FIELD_ENTRY                         = 0x03, // Size:1
    OBJECT_FIELD_SCALE_X                       = 0x04, // Size:1
    OBJECT_FIELD_PADDING                       = 0x05, // Size:1
    OBJECT_END                                 = 0x06,
};

enum EItemFields
{
    ITEM_FIELD_OWNER                           = OBJECT_END + 0x00, // Size:2
    ITEM_FIELD_CONTAINED                       = OBJECT_END + 0x02, // Size:2
    ITEM_FIELD_CREATOR                         = OBJECT_END + 0x04, // Size:2
    ITEM_FIELD_GIFTCREATOR                     = OBJECT_END + 0x06, // Size:2
    ITEM_FIELD_STACK_COUNT                     = OBJECT_END + 0x08, // Size:1
    ITEM_FIELD_DURATION                        = OBJECT_END + 0x09, // Size:1
    ITEM_FIELD_SPELL_CHARGES                   = OBJECT_END + 0x0A, // Size:5
    ITEM_FIELD_SPELL_CHARGES_01                = OBJECT_END + 0x0B,
    ITEM_FIELD_SPELL_CHARGES_02                = OBJECT_END + 0x0C,
    ITEM_FIELD_SPELL_CHARGES_03                = OBJECT_END + 0x0D,
    ITEM_FIELD_SPELL_CHARGES_04                = OBJECT_END + 0x0E,
    ITEM_FIELD_FLAGS                           = OBJECT_END + 0x0F, // Size:1
    /*
    There is two types of enchantments: property based and suffix based.
    Item can have only one of the two.
    Suffix based linked to item.randomproperty_2 field and property based
    to item.randomproperty_1 field (item prototype).
    Suffix based enchantments sent to the client as negative value, in
    addition they require ITEM_FIELD_SUFFIX_FACTOR field to be send in
    order to calculate the bonus value. Property based enchantments are send
    as positive values and do not require any aditional values since the bonuses
    are already stored inside spellitemenchantment. Suffix based enchantments uses 
    6-8 fields in EnchantmentSlot while property based uses 8-10 in EnchantmentSlot.
    */
    ITEM_FIELD_ENCHANTMENT                     = OBJECT_END + 0x10, // Size: 33 = (temp+perm+sockets*3+bonus+temp2*2+property*3)*3
    ITEM_FIELD_SUFFIX_FACTOR                   = OBJECT_END + 0x31, // Size:1
    ITEM_FIELD_RANDOM_PROPERTIES_ID            = OBJECT_END + 0x32, // Size:1
    ITEM_FIELD_ITEM_TEXT_ID                    = OBJECT_END + 0x33, // Size:1
    ITEM_FIELD_DURABILITY                      = OBJECT_END + 0x34, // Size:1
    ITEM_FIELD_MAXDURABILITY                   = OBJECT_END + 0x35, // Size:1
    ITEM_END                                   = OBJECT_END + 0x36,
};

enum EContainerFields
{
    CONTAINER_FIELD_NUM_SLOTS                  = ITEM_END + 0x00, // Size:1
    CONTAINER_ALIGN_PAD                        = ITEM_END + 0x01, // Size:1
    CONTAINER_FIELD_SLOT_1                     = ITEM_END + 0x02, // count=72
    CONTAINER_FIELD_SLOT_LAST                  = ITEM_END + 0x49,
    CONTAINER_END                              = ITEM_END + 0x4A,
};

enum EUnitFields
{
    UNIT_FIELD_CHARM                           = 0x00 + OBJECT_END, // Size:2
    UNIT_FIELD_SUMMON                          = 0x02 + OBJECT_END, // Size:2
    UNIT_FIELD_CHARMEDBY                       = 0x04 + OBJECT_END, // Size:2
    UNIT_FIELD_SUMMONEDBY                      = 0x06 + OBJECT_END, // Size:2
    UNIT_FIELD_CREATEDBY                       = 0x08 + OBJECT_END, // Size:2
    UNIT_FIELD_TARGET                          = 0x0A + OBJECT_END, // Size:2
    UNIT_FIELD_PERSUADED                       = 0x0C + OBJECT_END, // Size:2
    UNIT_FIELD_CHANNEL_OBJECT                  = 0x0E + OBJECT_END, // Size:2
    UNIT_FIELD_HEALTH                          = 0x10 + OBJECT_END, // Size:1
    UNIT_FIELD_POWER1                          = 0x11 + OBJECT_END, // Size:1
    UNIT_FIELD_POWER2                          = 0x12 + OBJECT_END, // Size:1
    UNIT_FIELD_POWER3                          = 0x13 + OBJECT_END, // Size:1
    UNIT_FIELD_POWER4                          = 0x14 + OBJECT_END, // Size:1
    UNIT_FIELD_POWER5                          = 0x15 + OBJECT_END, // Size:1
    UNIT_FIELD_MAXHEALTH                       = 0x16 + OBJECT_END, // Size:1
    UNIT_FIELD_MAXPOWER1                       = 0x17 + OBJECT_END, // Size:1
    UNIT_FIELD_MAXPOWER2                       = 0x18 + OBJECT_END, // Size:1
    UNIT_FIELD_MAXPOWER3                       = 0x19 + OBJECT_END, // Size:1
    UNIT_FIELD_MAXPOWER4                       = 0x1A + OBJECT_END, // Size:1
    UNIT_FIELD_MAXPOWER5                       = 0x1B + OBJECT_END, // Size:1
    UNIT_FIELD_LEVEL                           = 0x1C + OBJECT_END, // Size:1
    UNIT_FIELD_FACTIONTEMPLATE                 = 0x1D + OBJECT_END, // Size:1
    UNIT_FIELD_BYTES_0                         = 0x1E + OBJECT_END, // Size:1
    UNIT_VIRTUAL_ITEM_SLOT_DISPLAY             = 0x1F + OBJECT_END, // Size:3
    UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_01          = 0x20 + OBJECT_END,
    UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_02          = 0x21 + OBJECT_END,
    UNIT_VIRTUAL_ITEM_INFO                     = 0x22 + OBJECT_END, // Size:6
    UNIT_VIRTUAL_ITEM_INFO_01                  = 0x23 + OBJECT_END,
    UNIT_VIRTUAL_ITEM_INFO_02                  = 0x24 + OBJECT_END,
    UNIT_VIRTUAL_ITEM_INFO_03                  = 0x25 + OBJECT_END,
    UNIT_VIRTUAL_ITEM_INFO_04                  = 0x26 + OBJECT_END,
    UNIT_VIRTUAL_ITEM_INFO_05                  = 0x27 + OBJECT_END,
    UNIT_FIELD_FLAGS                           = 0x28 + OBJECT_END, // Size:1
    UNIT_FIELD_FLAGS_2                         = 0x29 + OBJECT_END, // Size:1
    UNIT_FIELD_AURA                            = 0x2A + OBJECT_END, // Size:56
    UNIT_FIELD_AURA_LAST                       = 0x61 + OBJECT_END,
    UNIT_FIELD_AURAFLAGS                       = 0x62 + OBJECT_END, // Size:7
    UNIT_FIELD_AURAFLAGS_01                    = 0x63 + OBJECT_END,
    UNIT_FIELD_AURAFLAGS_02                    = 0x64 + OBJECT_END,
    UNIT_FIELD_AURAFLAGS_03                    = 0x65 + OBJECT_END,
    UNIT_FIELD_AURAFLAGS_04                    = 0x66 + OBJECT_END,
    UNIT_FIELD_AURAFLAGS_05                    = 0x67 + OBJECT_END,
    UNIT_FIELD_AURAFLAGS_06                    = 0x68 + OBJECT_END,
    UNIT_FIELD_AURALEVELS                      = 0x69 + OBJECT_END, // Size:14
    UNIT_FIELD_AURALEVELS_LAST                 = 0x76 + OBJECT_END,
    UNIT_FIELD_AURAAPPLICATIONS                = 0x77 + OBJECT_END, // Size:14
    UNIT_FIELD_AURAAPPLICATIONS_LAST           = 0x84 + OBJECT_END,
    UNIT_FIELD_AURASTATE                       = 0x85 + OBJECT_END, // Size:1
    UNIT_FIELD_BASEATTACKTIME                  = 0x86 + OBJECT_END, // Size:2
    UNIT_FIELD_BASEATTACKTIME_01               = 0x87 + OBJECT_END,
    UNIT_FIELD_OFFHANDATTACKTIME               = 0x87 + OBJECT_END, // Size:2
    UNIT_FIELD_RANGEDATTACKTIME                = 0x88 + OBJECT_END, // Size:1
    UNIT_FIELD_BOUNDINGRADIUS                  = 0x89 + OBJECT_END, // Size:1
    UNIT_FIELD_COMBATREACH                     = 0x8A + OBJECT_END, // Size:1
    UNIT_FIELD_DISPLAYID                       = 0x8B + OBJECT_END, // Size:1
    UNIT_FIELD_NATIVEDISPLAYID                 = 0x8C + OBJECT_END, // Size:1
    UNIT_FIELD_MOUNTDISPLAYID                  = 0x8D + OBJECT_END, // Size:1
    UNIT_FIELD_MINDAMAGE                       = 0x8E + OBJECT_END, // Size:1
    UNIT_FIELD_MAXDAMAGE                       = 0x8F + OBJECT_END, // Size:1
    UNIT_FIELD_MINOFFHANDDAMAGE                = 0x90 + OBJECT_END, // Size:1
    UNIT_FIELD_MAXOFFHANDDAMAGE                = 0x91 + OBJECT_END, // Size:1
    UNIT_FIELD_BYTES_1                         = 0x92 + OBJECT_END, // Size:1
    UNIT_FIELD_PETNUMBER                       = 0x93 + OBJECT_END, // Size:1
    UNIT_FIELD_PET_NAME_TIMESTAMP              = 0x94 + OBJECT_END, // Size:1
    UNIT_FIELD_PETEXPERIENCE                   = 0x95 + OBJECT_END, // Size:1
    UNIT_FIELD_PETNEXTLEVELEXP                 = 0x96 + OBJECT_END, // Size:1
    UNIT_DYNAMIC_FLAGS                         = 0x97 + OBJECT_END, // Size:1
    UNIT_CHANNEL_SPELL                         = 0x98 + OBJECT_END, // Size:1
    UNIT_MOD_CAST_SPEED                        = 0x99 + OBJECT_END, // Size:1
    UNIT_CREATED_BY_SPELL                      = 0x9A + OBJECT_END, // Size:1
    UNIT_NPC_FLAGS                             = 0x9B + OBJECT_END, // Size:1
    UNIT_NPC_EMOTESTATE                        = 0x9C + OBJECT_END, // Size:1
    UNIT_TRAINING_POINTS                       = 0x9D + OBJECT_END, // Size:1
    UNIT_FIELD_STAT0                           = 0x9E + OBJECT_END, // Size:1
    UNIT_FIELD_STAT1                           = 0x9F + OBJECT_END, // Size:1
    UNIT_FIELD_STAT2                           = 0xA0 + OBJECT_END, // Size:1
    UNIT_FIELD_STAT3                           = 0xA1 + OBJECT_END, // Size:1
    UNIT_FIELD_STAT4                           = 0xA2 + OBJECT_END, // Size:1
    UNIT_FIELD_POSSTAT0                        = 0xA3 + OBJECT_END, // Size:1
    UNIT_FIELD_POSSTAT1                        = 0xA4 + OBJECT_END, // Size:1
    UNIT_FIELD_POSSTAT2                        = 0xA5 + OBJECT_END, // Size:1
    UNIT_FIELD_POSSTAT3                        = 0xA6 + OBJECT_END, // Size:1
    UNIT_FIELD_POSSTAT4                        = 0xA7 + OBJECT_END, // Size:1
    UNIT_FIELD_NEGSTAT0                        = 0xA8 + OBJECT_END, // Size:1
    UNIT_FIELD_NEGSTAT1                        = 0xA9 + OBJECT_END, // Size:1
    UNIT_FIELD_NEGSTAT2                        = 0xAA + OBJECT_END, // Size:1
    UNIT_FIELD_NEGSTAT3                        = 0xAB + OBJECT_END, // Size:1
    UNIT_FIELD_NEGSTAT4                        = 0xAC + OBJECT_END, // Size:1
    UNIT_FIELD_RESISTANCES                     = 0xAD + OBJECT_END, // Size:7
    UNIT_FIELD_RESISTANCES_01                  = 0xAE + OBJECT_END,
    UNIT_FIELD_RESISTANCES_02                  = 0xAF + OBJECT_END,
    UNIT_FIELD_RESISTANCES_03                  = 0xB0 + OBJECT_END,
    UNIT_FIELD_RESISTANCES_04                  = 0xB1 + OBJECT_END,
    UNIT_FIELD_RESISTANCES_05                  = 0xB2 + OBJECT_END,
    UNIT_FIELD_RESISTANCES_06                  = 0xB3 + OBJECT_END,
    UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE      = 0xB4 + OBJECT_END, // Size:7
    UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE      = 0xBB + OBJECT_END, // Size:7
    UNIT_FIELD_BASE_MANA                       = 0xC2 + OBJECT_END, // Size:1
    UNIT_FIELD_BASE_HEALTH                     = 0xC3 + OBJECT_END, // Size:1
    UNIT_FIELD_BYTES_2                         = 0xC4 + OBJECT_END, // Size:1
    UNIT_FIELD_ATTACK_POWER                    = 0xC5 + OBJECT_END, // Size:1
    UNIT_FIELD_ATTACK_POWER_MODS               = 0xC6 + OBJECT_END, // Size:1
    UNIT_FIELD_ATTACK_POWER_MULTIPLIER         = 0xC7 + OBJECT_END, // Size:1
    UNIT_FIELD_RANGED_ATTACK_POWER             = 0xC8 + OBJECT_END, // Size:1
    UNIT_FIELD_RANGED_ATTACK_POWER_MODS        = 0xC9 + OBJECT_END, // Size:1
    UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER  = 0xCA + OBJECT_END, // Size:1
    UNIT_FIELD_MINRANGEDDAMAGE                 = 0xCB + OBJECT_END, // Size:1
    UNIT_FIELD_MAXRANGEDDAMAGE                 = 0xCC + OBJECT_END, // Size:1
    UNIT_FIELD_POWER_COST_MODIFIER             = 0xCD + OBJECT_END, // Size:7
    UNIT_FIELD_POWER_COST_MODIFIER_01          = 0xCE + OBJECT_END,
    UNIT_FIELD_POWER_COST_MODIFIER_02          = 0xCF + OBJECT_END,
    UNIT_FIELD_POWER_COST_MODIFIER_03          = 0xD0 + OBJECT_END,
    UNIT_FIELD_POWER_COST_MODIFIER_04          = 0xD1 + OBJECT_END,
    UNIT_FIELD_POWER_COST_MODIFIER_05          = 0xD2 + OBJECT_END,
    UNIT_FIELD_POWER_COST_MODIFIER_06          = 0xD3 + OBJECT_END,
    UNIT_FIELD_POWER_COST_MULTIPLIER           = 0xD4 + OBJECT_END, // Size:7
    UNIT_FIELD_POWER_COST_MULTIPLIER_01        = 0xD5 + OBJECT_END,
    UNIT_FIELD_POWER_COST_MULTIPLIER_02        = 0xD6 + OBJECT_END,
    UNIT_FIELD_POWER_COST_MULTIPLIER_03        = 0xD7 + OBJECT_END,
    UNIT_FIELD_POWER_COST_MULTIPLIER_04        = 0xD8 + OBJECT_END,
    UNIT_FIELD_POWER_COST_MULTIPLIER_05        = 0xD9 + OBJECT_END,
    UNIT_FIELD_POWER_COST_MULTIPLIER_06        = 0xDA + OBJECT_END,
    UNIT_FIELD_PADDING                         = 0xDB + OBJECT_END,
    UNIT_END                                   = 0xDC + OBJECT_END,

    PLAYER_DUEL_ARBITER                        = 0x00 + UNIT_END, // Size:2
    PLAYER_FLAGS                               = 0x02 + UNIT_END, // Size:1
    PLAYER_GUILDID                             = 0x03 + UNIT_END, // Size:1
    PLAYER_GUILDRANK                           = 0x04 + UNIT_END, // Size:1
    PLAYER_BYTES                               = 0x05 + UNIT_END, // Size:1
    PLAYER_BYTES_2                             = 0x06 + UNIT_END, // Size:1
    PLAYER_BYTES_3                             = 0x07 + UNIT_END, // Size:1
    PLAYER_DUEL_TEAM                           = 0x08 + UNIT_END, // Size:1
    PLAYER_GUILD_TIMESTAMP                     = 0x09 + UNIT_END, // Size:1
    PLAYER_QUEST_LOG_1_1                       = 0x0A + UNIT_END, // count = 25
    PLAYER_QUEST_LOG_1_2                       = 0x0B + UNIT_END,
    PLAYER_QUEST_LOG_1_3                       = 0x0C + UNIT_END,
    PLAYER_QUEST_LOG_LAST_1                    = 0x52 + UNIT_END,
    PLAYER_QUEST_LOG_LAST_2                    = 0x53 + UNIT_END,
    PLAYER_QUEST_LOG_LAST_3                    = 0x54 + UNIT_END,
    PLAYER_VISIBLE_ITEM_1_CREATOR              = 0x55 + UNIT_END, // Size:2, count = 19
    PLAYER_VISIBLE_ITEM_1_0                    = 0x57 + UNIT_END, // Size:12
    PLAYER_VISIBLE_ITEM_1_PROPERTIES           = 0x63 + UNIT_END, // Size:1
    PLAYER_VISIBLE_ITEM_1_PAD                  = 0x64 + UNIT_END, // Size:1
    PLAYER_VISIBLE_ITEM_LAST_CREATOR           = 0x175 + UNIT_END,
    PLAYER_VISIBLE_ITEM_LAST_0                 = 0x177 + UNIT_END,
    PLAYER_VISIBLE_ITEM_LAST_PROPERTIES        = 0x183 + UNIT_END,
    PLAYER_VISIBLE_ITEM_LAST_PAD               = 0x184 + UNIT_END,
    PLAYER_CHOSEN_TITLE                        = 0x185 + UNIT_END, // Size:1
    PLAYER_FIELD_INV_SLOT_HEAD                 = 0x186 + UNIT_END, // Size:46
    PLAYER_FIELD_PACK_SLOT_1                   = 0x1B4 + UNIT_END, // Size:32
    PLAYER_FIELD_PACK_SLOT_LAST                = 0x1D3 + UNIT_END,
    PLAYER_FIELD_BANK_SLOT_1                   = 0x1D4 + UNIT_END, // Size:56
    PLAYER_FIELD_BANK_SLOT_LAST                = 0x20B + UNIT_END,
    PLAYER_FIELD_BANKBAG_SLOT_1                = 0x20C + UNIT_END, // Size:14
    PLAYER_FIELD_BANKBAG_SLOT_LAST             = 0x219 + UNIT_END,
    PLAYER_FIELD_VENDORBUYBACK_SLOT_1          = 0x21A + UNIT_END, // Size:24
    PLAYER_FIELD_VENDORBUYBACK_SLOT_LAST       = 0x231 + UNIT_END,
    PLAYER_FIELD_KEYRING_SLOT_1                = 0x232 + UNIT_END, // Size:64
    PLAYER_FIELD_KEYRING_SLOT_LAST             = 0x271 + UNIT_END,
    PLAYER_FARSIGHT                            = 0x272 + UNIT_END, // Size:2
    PLAYER_FIELD_COMBO_TARGET                  = 0x274 + UNIT_END, // Size:2
    PLAYER_FIELD_KNOWN_TITLES                  = 0x276 + UNIT_END, // Size:2
    PLAYER_XP                                  = 0x278 + UNIT_END, // Size:1
    PLAYER_NEXT_LEVEL_XP                       = 0x279 + UNIT_END, // Size:1
    PLAYER_SKILL_INFO_1_1                      = 0x27A + UNIT_END, // Size:384
    PLAYER_CHARACTER_POINTS1                   = 0x3FA + UNIT_END, // Size:1
    PLAYER_CHARACTER_POINTS2                   = 0x3FB + UNIT_END, // Size:1
    PLAYER_TRACK_CREATURES                     = 0x3FC + UNIT_END, // Size:1
    PLAYER_TRACK_RESOURCES                     = 0x3FD + UNIT_END, // Size:1
    PLAYER_BLOCK_PERCENTAGE                    = 0x3FE + UNIT_END, // Size:1
    PLAYER_DODGE_PERCENTAGE                    = 0x3FF + UNIT_END, // Size:1
    PLAYER_PARRY_PERCENTAGE                    = 0x400 + UNIT_END, // Size:1
    PLAYER_CRIT_PERCENTAGE                     = 0x401 + UNIT_END, // Size:1
    PLAYER_RANGED_CRIT_PERCENTAGE              = 0x402 + UNIT_END, // Size:1
    PLAYER_OFFHAND_CRIT_PERCENTAGE             = 0x403 + UNIT_END, // Size:1
    PLAYER_SPELL_CRIT_PERCENTAGE1              = 0x404 + UNIT_END, // Size:7
    PLAYER_HOLY_SPELL_CRIT_PERCENTAGE          = 0x405 + UNIT_END, // custom
    PLAYER_FIRE_SPELL_CRIT_PERCENTAGE          = 0x406 + UNIT_END, // custom
    PLAYER_NATURE_SPELL_CRIT_PERCENTAGE        = 0x407 + UNIT_END, // custom
    PLAYER_FROST_SPELL_CRIT_PERCENTAGE         = 0x408 + UNIT_END, // custom
    PLAYER_SHADOW_SPELL_CRIT_PERCENTAGE        = 0x409 + UNIT_END, // custom
    PLAYER_ARCANE_SPELL_CRIT_PERCENTAGE        = 0x40A + UNIT_END, // custom
    PLAYER_EXPLORED_ZONES_1                    = 0x40B + UNIT_END, // Size:64
    PLAYER_REST_STATE_EXPERIENCE               = 0x44B + UNIT_END, // Size:1
    PLAYER_FIELD_COINAGE                       = 0x44C + UNIT_END, // Size:1
    PLAYER_FIELD_MOD_DAMAGE_DONE_POS           = 0x44D + UNIT_END, // Size:7
    PLAYER_FIELD_MOD_DAMAGE_DONE_NEG           = 0x454 + UNIT_END, // Size:7
    PLAYER_FIELD_MOD_DAMAGE_DONE_PCT           = 0x45B + UNIT_END, // Size:7
    PLAYER_FIELD_MOD_HEALING_DONE_POS          = 0x462 + UNIT_END, // Size:1
    PLAYER_FIELD_MOD_TARGET_RESISTANCE         = 0x463 + UNIT_END, // Size:1
    PLAYER_FIELD_BYTES                         = 0x464 + UNIT_END, // Size:1
    PLAYER_AMMO_ID                             = 0x465 + UNIT_END, // Size:1
    PLAYER_SELF_RES_SPELL                      = 0x466 + UNIT_END, // Size:1
    PLAYER_FIELD_PVP_MEDALS                    = 0x467 + UNIT_END, // Size:1
    PLAYER_FIELD_BUYBACK_PRICE_1               = 0x468 + UNIT_END, // count=12
    PLAYER_FIELD_BUYBACK_PRICE_LAST            = 0x473 + UNIT_END,
    PLAYER_FIELD_BUYBACK_TIMESTAMP_1           = 0x474 + UNIT_END, // count=12
    PLAYER_FIELD_BUYBACK_TIMESTAMP_LAST        = 0x47A + UNIT_END,
    PLAYER_FIELD_KILLS                         = 0x480 + UNIT_END, // Size:1   // ((uint16)kills_today<<16) | (uint16)kills_yesterday)
    PLAYER_FIELD_HONOR_TODAY                   = 0x481 + UNIT_END, // Size:1
    PLAYER_FIELD_HONOR_YESTERDAY               = 0x482 + UNIT_END, // Size:1
    PLAYER_FIELD_KILLS_LIFETIME                = 0x483 + UNIT_END, // Size:1
    PLAYER_FIELD_BYTES2                        = 0x484 + UNIT_END, // Size:1 
    PLAYER_FIELD_WATCHED_FACTION_INDEX         = 0x485 + UNIT_END, // Size:1
    PLAYER_FIELD_COMBAT_RATING_1               = 0x486 + UNIT_END, // Size:23
    PLAYER_FIELD_ALL_WEAPONS_SKILL_RATING      = 0x486 + UNIT_END, // custom
    PLAYER_FIELD_DEFENCE_RATING                = 0x487 + UNIT_END, // custom
    PLAYER_FIELD_DODGE_RATING                  = 0x488 + UNIT_END, // custom
    PLAYER_FIELD_PARRY_RATING                  = 0x489 + UNIT_END, // custom
    PLAYER_FIELD_BLOCK_RATING                  = 0x48A + UNIT_END, // custom
    PLAYER_FIELD_MELEE_HIT_RATING              = 0x48B + UNIT_END, // custom
    PLAYER_FIELD_RANGED_HIT_RATING             = 0x48C + UNIT_END, // custom
    PLAYER_FIELD_SPELL_HIT_RATING              = 0x48D + UNIT_END, // custom
    PLAYER_FIELD_MELEE_CRIT_RATING             = 0x48E + UNIT_END, // custom
    PLAYER_FIELD_RANGED_CRIT_RATING            = 0x48F + UNIT_END, // custom
    PLAYER_FIELD_SPELL_CRIT_RATING             = 0x490 + UNIT_END, // custom
    PLAYER_FIELD_HIT_RATING                    = 0x491 + UNIT_END, // unsure, was PLAYER_FIELD_UNK1_RATING
    PLAYER_FIELD_CRIT_RATING                   = 0x492 + UNIT_END, // unsure, was PLAYER_FIELD_UNK2_RATING
    PLAYER_FIELD_UNK3_RATING                   = 0x493 + UNIT_END, // custom
    PLAYER_FIELD_UNK4_RATING                   = 0x494 + UNIT_END, // custom
    PLAYER_FIELD_UNK5_RATING                   = 0x495 + UNIT_END, // custom
    PLAYER_FIELD_RESILIENCE_RATING             = 0x496 + UNIT_END, // custom
    PLAYER_FIELD_MELEE_HASTE_RATING            = 0x497 + UNIT_END, // custom
    PLAYER_FIELD_RANGED_HASTE_RATING           = 0x498 + UNIT_END, // custom
    PLAYER_FIELD_SPELL_HASTE_RATING            = 0x499 + UNIT_END, // custom
    PLAYER_FIELD_MELEE_WEAPON_SKILL_RATING     = 0x49A + UNIT_END, // custom
    PLAYER_FIELD_OFFHAND_WEAPON_SKILL_RATING   = 0x49B + UNIT_END, // custom
    PLAYER_FIELD_RANGED_WEAPON_SKILL_RATING    = 0x49C + UNIT_END, // custom
    PLAYER_FIELD_ARENA_TEAM_INFO_1_1           = 0x49D + UNIT_END, // Size:9
    PLAYER_FIELD_ARENA_TEAM_ID_2v2             = 0x49D + UNIT_END, // custom
    PLAYER_FIELD_ARENA_TEAM_ID_3v3             = 0x4A0 + UNIT_END, // custom
    PLAYER_FIELD_ARENA_TEAM_ID_5v5             = 0x4A3 + UNIT_END, // custom
    PLAYER_FIELD_HONOR_CURRENCY                = 0x4A6 + UNIT_END, // Size:1
    PLAYER_FIELD_ARENA_CURRENCY                = 0x4A7 + UNIT_END, // Size:1
    PLAYER_FIELD_MOD_MANA_REGEN                = 0x4A8 + UNIT_END, // Size:1
    PLAYER_FIELD_MOD_MANA_REGEN_INTERRUPT      = 0x4A9 + UNIT_END, // Size:1
    PLAYER_FIELD_MAX_LEVEL                     = 0x4AA + UNIT_END, // Size:1
    PLAYER_FIELD_PADDING                       = 0x4AB + UNIT_END, // Size:1
    PLAYER_END                                 = 0x4AC + UNIT_END,
};

enum EGameObjectFields
{
    OBJECT_FIELD_CREATED_BY                    = OBJECT_END + 0x00,
    GAMEOBJECT_DISPLAYID                       = OBJECT_END + 0x02,
    GAMEOBJECT_FLAGS                           = OBJECT_END + 0x03,
    GAMEOBJECT_ROTATION                        = OBJECT_END + 0x04,
    GAMEOBJECT_STATE                           = OBJECT_END + 0x08,
    GAMEOBJECT_POS_X                           = OBJECT_END + 0x09,
    GAMEOBJECT_POS_Y                           = OBJECT_END + 0x0A,
    GAMEOBJECT_POS_Z                           = OBJECT_END + 0x0B,
    GAMEOBJECT_FACING                          = OBJECT_END + 0x0C,
    GAMEOBJECT_DYN_FLAGS                       = OBJECT_END + 0x0D,
    GAMEOBJECT_FACTION                         = OBJECT_END + 0x0E,
    GAMEOBJECT_TYPE_ID                         = OBJECT_END + 0x0F,
    GAMEOBJECT_LEVEL                           = OBJECT_END + 0x10,
    GAMEOBJECT_ARTKIT                          = OBJECT_END + 0x11,
    GAMEOBJECT_ANIMPROGRESS                    = OBJECT_END + 0x12,
    GAMEOBJECT_PADDING                         = OBJECT_END + 0x13,
    GAMEOBJECT_END                             = OBJECT_END + 0x14,
};

enum EDynamicObjectFields
{
    DYNAMICOBJECT_CASTER                       = OBJECT_END + 0x00,
    DYNAMICOBJECT_BYTES                        = OBJECT_END + 0x02,
    DYNAMICOBJECT_SPELLID                      = OBJECT_END + 0x03,
    DYNAMICOBJECT_RADIUS                       = OBJECT_END + 0x04,
    DYNAMICOBJECT_POS_X                        = OBJECT_END + 0x05,
    DYNAMICOBJECT_POS_Y                        = OBJECT_END + 0x06,
    DYNAMICOBJECT_POS_Z                        = OBJECT_END + 0x07,
    DYNAMICOBJECT_FACING                       = OBJECT_END + 0x08,
    DYNAMICOBJECT_PAD                          = OBJECT_END + 0x09,
    DYNAMICOBJECT_END                          = OBJECT_END + 0x0A,
};

enum ECorpseFields
{
    CORPSE_FIELD_OWNER                         = OBJECT_END + 0x00,
    CORPSE_FIELD_FACING                        = OBJECT_END + 0x02,
    CORPSE_FIELD_POS_X                         = OBJECT_END + 0x03,
    CORPSE_FIELD_POS_Y                         = OBJECT_END + 0x04,
    CORPSE_FIELD_POS_Z                         = OBJECT_END + 0x05,
    CORPSE_FIELD_DISPLAY_ID                    = OBJECT_END + 0x06,
    CORPSE_FIELD_ITEM                          = OBJECT_END + 0x07, // 19
    CORPSE_FIELD_BYTES_1                       = OBJECT_END + 0x1A,
    CORPSE_FIELD_BYTES_2                       = OBJECT_END + 0x1B,
    CORPSE_FIELD_GUILD                         = OBJECT_END + 0x1C,
    CORPSE_FIELD_FLAGS                         = OBJECT_END + 0x1D,
    CORPSE_FIELD_DYNAMIC_FLAGS                 = OBJECT_END + 0x1E,
    CORPSE_FIELD_PAD                           = OBJECT_END + 0x1F,
    CORPSE_END                                 = OBJECT_END + 0x20,
};
#endif
