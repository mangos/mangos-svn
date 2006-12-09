/* 
 * Copyright (C) 2005,2006 MaNGOS <http://www.mangosproject.org/>
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
    OBJECT_FIELD_GUID                       = 0,
    OBJECT_FIELD_TYPE                       = 2,
    OBJECT_FIELD_ENTRY                      = 3,
    OBJECT_FIELD_SCALE_X                    = 4,
    OBJECT_FIELD_PADDING                    = 5,
    OBJECT_END                              = 6,
};

enum EItemFields
{
    ITEM_FIELD_OWNER                        = 6,
    ITEM_FIELD_CONTAINED                    = 8,
    ITEM_FIELD_CREATOR                      = 10,
    ITEM_FIELD_GIFTCREATOR                  = 12,
    ITEM_FIELD_STACK_COUNT                  = 14,
    ITEM_FIELD_DURATION                     = 15,
    ITEM_FIELD_SPELL_CHARGES                = 16,
    ITEM_FIELD_FLAGS                        = 21,
    ITEM_FIELD_ENCHANTMENT                  = 22,
    ITEM_FIELD_PROPERTY_SEED                = 43,
    ITEM_FIELD_RANDOM_PROPERTIES_ID         = 44,
    ITEM_FIELD_ITEM_TEXT_ID                 = 45,
    ITEM_FIELD_DURABILITY                   = 46,
    ITEM_FIELD_MAXDURABILITY                = 47,
    ITEM_END                                = 48,
};

enum EContainerFields
{
    CONTAINER_FIELD_NUM_SLOTS               = 48,
    CONTAINER_ALIGN_PAD                     = 49,
    CONTAINER_FIELD_SLOT_1                  = 50,
    CONTAINER_END                           = 106,
    //CONTAINER_END                           = 90,
};

enum EUnitFields
{

    UNIT_FIELD_CHARM                          = 0 + OBJECT_END,
    UNIT_FIELD_SUMMON                         = 0x2 + OBJECT_END,
    UNIT_FIELD_CHARMEDBY                      = 0x4 + OBJECT_END,
    UNIT_FIELD_SUMMONEDBY                     = 0x6 + OBJECT_END,
    UNIT_FIELD_CREATEDBY                      = 0x8 + OBJECT_END,
    UNIT_FIELD_TARGET                         = 0xA + OBJECT_END,
    UNIT_FIELD_PERSUADED                      = 0xC + OBJECT_END,
    UNIT_FIELD_CHANNEL_OBJECT                 = 0xE + OBJECT_END,
    UNIT_FIELD_HEALTH                         = 0x10 + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_POWER1                         = 0x11 + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_POWER2                         = 0x12 + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_POWER3                         = 0x13 + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_POWER4                         = 0x14 + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_POWER5                         = 0x15 + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_MAXHEALTH                      = 0x16 + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_MAXPOWER1                      = 0x17 + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_MAXPOWER2                      = 0x18 + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_MAXPOWER3                      = 0x19 + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_MAXPOWER4                      = 0x1A + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_MAXPOWER5                      = 0x1B + OBJECT_END,
    UNIT_FIELD_LEVEL                          = 0x1C + OBJECT_END,
    UNIT_FIELD_FACTIONTEMPLATE                = 0x1D + OBJECT_END,
    UNIT_FIELD_BYTES_0                        = 0x1E + OBJECT_END,
    UNIT_VIRTUAL_ITEM_SLOT_DISPLAY            = 0x1F + OBJECT_END,
    UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_01         = 0x20 + OBJECT_END,
    UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_02         = 0x21 + OBJECT_END,
    UNIT_VIRTUAL_ITEM_INFO                    = 0x22 + OBJECT_END,
    UNIT_VIRTUAL_ITEM_INFO_01                 = 0x23 + OBJECT_END,
    UNIT_VIRTUAL_ITEM_INFO_02                 = 0x24 + OBJECT_END,
    UNIT_VIRTUAL_ITEM_INFO_03                 = 0x25 + OBJECT_END,
    UNIT_VIRTUAL_ITEM_INFO_04                 = 0x26 + OBJECT_END,
    UNIT_VIRTUAL_ITEM_INFO_05                 = 0x27 + OBJECT_END,

    UNIT_FIELD_FLAGS                          = 0x28 + OBJECT_END,
    UNIT_FIELD_AURA                           = 0x29 + OBJECT_END,

    UNIT_FIELD_AURAFLAGS                      = 0x59 + OBJECT_END,
    UNIT_FIELD_AURALEVELS                     = 0x5F + OBJECT_END,
    UNIT_FIELD_AURAAPPLICATIONS               = 0x6B + OBJECT_END,
    UNIT_FIELD_AURASTATE                      = 0x77 + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_BASEATTACKTIME                 = 0x78 + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_OFFHANDATTACKTIME              = 0x79 + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_RANGEDATTACKTIME               = 0x7A + OBJECT_END,
    UNIT_FIELD_BOUNDINGRADIUS                 = 0x7B + OBJECT_END,
    UNIT_FIELD_COMBATREACH                    = 0x7C + OBJECT_END,
    UNIT_FIELD_DISPLAYID                      = 0x7D + OBJECT_END,
    UNIT_FIELD_NATIVEDISPLAYID                = 0x7E + OBJECT_END,
    UNIT_FIELD_MOUNTDISPLAYID                 = 0x7F + OBJECT_END,
    UNIT_FIELD_MINDAMAGE                      = 0x80 + OBJECT_END,
    UNIT_FIELD_MAXDAMAGE                      = 0x81 + OBJECT_END,
    UNIT_FIELD_MINOFFHANDDAMAGE               = 0x82 + OBJECT_END,
    UNIT_FIELD_MAXOFFHANDDAMAGE               = 0x83 + OBJECT_END,
    UNIT_FIELD_BYTES_1                        = 0x84 + OBJECT_END,
    UNIT_FIELD_PETNUMBER                      = 0x85 + OBJECT_END,
    UNIT_FIELD_PET_NAME_TIMESTAMP             = 0x86 + OBJECT_END,
    UNIT_FIELD_PETEXPERIENCE                  = 0x87 + OBJECT_END,
    UNIT_FIELD_PETNEXTLEVELEXP                = 0x88 + OBJECT_END,
    UNIT_DYNAMIC_FLAGS                        = 0x89 + OBJECT_END,
    UNIT_CHANNEL_SPELL                        = 0x8A + OBJECT_END,
    UNIT_MOD_CAST_SPEED                       = 0x8B + OBJECT_END,
    UNIT_CREATED_BY_SPELL                     = 0x8C + OBJECT_END,
    UNIT_NPC_FLAGS                            = 0x8D + OBJECT_END,
    UNIT_NPC_EMOTESTATE                       = 0x8E + OBJECT_END,
    UNIT_TRAINING_POINTS                      = 0x8F + OBJECT_END,

                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_STATS                          = 0x90 + OBJECT_END,
    UNIT_FIELD_STR                            = UNIT_FIELD_STATS,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_AGILITY                        = 0x91 + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_STAMINA                        = 0x92 + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_IQ                             = 0x93 + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_SPIRIT                         = 0x94 + OBJECT_END,

                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_RESISTANCES                    = 0x95 + OBJECT_END,
    UNIT_FIELD_ARMOR                          = UNIT_FIELD_RESISTANCES,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_RESISTANCES_01                 = 0x96 + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_RESISTANCES_02                 = 0x97 + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_RESISTANCES_03                 = 0x98 + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_RESISTANCES_04                 = 0x99 + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_RESISTANCES_05                 = 0x9A + OBJECT_END,
                                                            // used at server in float format but send to client in uint32 format
    UNIT_FIELD_RESISTANCES_06                 = 0x9B + OBJECT_END,

    UNIT_FIELD_BASE_MANA                      = 0x9C + OBJECT_END,
    UNIT_FIELD_BASE_HEALTH                    = 0x9D + OBJECT_END,
    UNIT_FIELD_BYTES_2                        = 0x9E + OBJECT_END,
    UNIT_FIELD_ATTACK_POWER                   = 0x9F + OBJECT_END,
    UNIT_FIELD_ATTACK_POWER_MODS              = 0xA0 + OBJECT_END,
    UNIT_FIELD_ATTACK_POWER_MULTIPLIER        = 0xA1 + OBJECT_END,
    UNIT_FIELD_RANGED_ATTACK_POWER            = 0xA2 + OBJECT_END,
    UNIT_FIELD_RANGED_ATTACK_POWER_MODS       = 0xA3 + OBJECT_END,
    UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER = 0xA4 + OBJECT_END,
    UNIT_FIELD_MINRANGEDDAMAGE                = 0xA5 + OBJECT_END,
    UNIT_FIELD_MAXRANGEDDAMAGE                = 0xA6 + OBJECT_END,
    UNIT_FIELD_POWER_COST_MODIFIER            = 0xA7 + OBJECT_END,
    UNIT_FIELD_POWER_COST_MULTIPLIER          = 0xAE + OBJECT_END,
    UNIT_FIELD_PADDING                        = 0xB5 + OBJECT_END,
    UNIT_END                                  = 0xB6 + OBJECT_END,

    PLAYER_SELECTION                          = 0x0 + UNIT_END,
    PLAYER_DUEL_ARBITER                       = 0x2 + UNIT_END-2,
    PLAYER_FLAGS                              = 0x4 + UNIT_END-2,
    PLAYER_GUILDID                            = 0x5 + UNIT_END-2,
    PLAYER_GUILDRANK                          = 0x6 + UNIT_END-2,
    PLAYER_BYTES                              = 0x7 + UNIT_END-2,
    PLAYER_BYTES_2                            = 0x8 + UNIT_END-2,
    PLAYER_BYTES_3                            = 0x9 + UNIT_END-2,
    PLAYER_DUEL_TEAM                          = 0xA + UNIT_END-2,
    PLAYER_GUILD_TIMESTAMP                    = 0xB + UNIT_END-2,
    PLAYER_QUEST_LOG_1_1                      = 0xC + UNIT_END-2,
    PLAYER_QUEST_LOG_1_2                      = 0xD + UNIT_END-2,
    PLAYER_QUEST_LOG_2_1                      = 0xF + UNIT_END-2,
    PLAYER_QUEST_LOG_2_2                      = 0x10 + UNIT_END-2,
    PLAYER_QUEST_LOG_3_1                      = 0x12 + UNIT_END-2,
    PLAYER_QUEST_LOG_3_2                      = 0x13 + UNIT_END-2,
    PLAYER_QUEST_LOG_4_1                      = 0x15 + UNIT_END-2,
    PLAYER_QUEST_LOG_4_2                      = 0x16 + UNIT_END-2,
    PLAYER_QUEST_LOG_5_1                      = 0x18 + UNIT_END-2,
    PLAYER_QUEST_LOG_5_2                      = 0x19 + UNIT_END-2,
    PLAYER_QUEST_LOG_6_1                      = 0x1B + UNIT_END-2,
    PLAYER_QUEST_LOG_6_2                      = 0x1C + UNIT_END-2,
    PLAYER_QUEST_LOG_7_1                      = 0x1E + UNIT_END-2,
    PLAYER_QUEST_LOG_7_2                      = 0x1F + UNIT_END-2,
    PLAYER_QUEST_LOG_8_1                      = 0x21 + UNIT_END-2,
    PLAYER_QUEST_LOG_8_2                      = 0x22 + UNIT_END-2,
    PLAYER_QUEST_LOG_9_1                      = 0x24 + UNIT_END-2,
    PLAYER_QUEST_LOG_9_2                      = 0x25 + UNIT_END-2,
    PLAYER_QUEST_LOG_10_1                     = 0x27 + UNIT_END-2,
    PLAYER_QUEST_LOG_10_2                     = 0x28 + UNIT_END-2,
    PLAYER_QUEST_LOG_11_1                     = 0x2A + UNIT_END-2,
    PLAYER_QUEST_LOG_11_2                     = 0x2B + UNIT_END-2,
    PLAYER_QUEST_LOG_12_1                     = 0x2D + UNIT_END-2,
    PLAYER_QUEST_LOG_12_2                     = 0x2E + UNIT_END-2,
    PLAYER_QUEST_LOG_13_1                     = 0x30 + UNIT_END-2,
    PLAYER_QUEST_LOG_13_2                     = 0x31 + UNIT_END-2,
    PLAYER_QUEST_LOG_14_1                     = 0x33 + UNIT_END-2,
    PLAYER_QUEST_LOG_14_2                     = 0x34 + UNIT_END-2,
    PLAYER_QUEST_LOG_15_1                     = 0x36 + UNIT_END-2,
    PLAYER_QUEST_LOG_15_2                     = 0x37 + UNIT_END-2,
    PLAYER_QUEST_LOG_16_1                     = 0x39 + UNIT_END-2,
    PLAYER_QUEST_LOG_16_2                     = 0x3A + UNIT_END-2,
    PLAYER_QUEST_LOG_17_1                     = 0x3C + UNIT_END-2,
    PLAYER_QUEST_LOG_17_2                     = 0x3D + UNIT_END-2,
    PLAYER_QUEST_LOG_18_1                     = 0x3F + UNIT_END-2,
    PLAYER_QUEST_LOG_18_2                     = 0x40 + UNIT_END-2,
    PLAYER_QUEST_LOG_19_1                     = 0x42 + UNIT_END-2,
    PLAYER_QUEST_LOG_19_2                     = 0x43 + UNIT_END-2,
    PLAYER_QUEST_LOG_20_1                     = 0x45 + UNIT_END-2,
    PLAYER_QUEST_LOG_20_2                     = 0x46 + UNIT_END-2,

                                                            //260
    PLAYER_VISIBLE_ITEM_1_CREATOR             = 0x48 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_1_0                   = 0x4A + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_1_PROPERTIES          = 0x52 + UNIT_END-2,
                                                            //271
    PLAYER_VISIBLE_ITEM_1_PAD                 = 0x53 + UNIT_END-2,

                                                            //272
    PLAYER_VISIBLE_ITEM_2_CREATOR             = 0x54 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_2_0                   = 0x56 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_2_PROPERTIES          = 0x5E + UNIT_END-2,
                                                            //283
    PLAYER_VISIBLE_ITEM_2_PAD                 = 0x5F + UNIT_END-2,

                                                            //284
    PLAYER_VISIBLE_ITEM_3_CREATOR             = 0x60 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_3_0                   = 0x62 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_3_PROPERTIES          = 0x6A + UNIT_END-2,
                                                            //295
    PLAYER_VISIBLE_ITEM_3_PAD                 = 0x6B + UNIT_END-2,

                                                            //296
    PLAYER_VISIBLE_ITEM_4_CREATOR             = 0x6C + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_4_0                   = 0x6E + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_4_PROPERTIES          = 0x76 + UNIT_END-2,
                                                            //307
    PLAYER_VISIBLE_ITEM_4_PAD                 = 0x77 + UNIT_END-2,

                                                            //308
    PLAYER_VISIBLE_ITEM_5_CREATOR             = 0x78 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_5_0                   = 0x7A + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_5_PROPERTIES          = 0x82 + UNIT_END-2,
                                                            //319
    PLAYER_VISIBLE_ITEM_5_PAD                 = 0x83 + UNIT_END-2,

                                                            //320
    PLAYER_VISIBLE_ITEM_6_CREATOR             = 0x84 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_6_0                   = 0x86 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_6_PROPERTIES          = 0x8E + UNIT_END-2,
                                                            //331
    PLAYER_VISIBLE_ITEM_6_PAD                 = 0x8F + UNIT_END-2,

                                                            //332
    PLAYER_VISIBLE_ITEM_7_CREATOR             = 0x90 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_7_0                   = 0x92 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_7_PROPERTIES          = 0x9A + UNIT_END-2,
                                                            //343
    PLAYER_VISIBLE_ITEM_7_PAD                 = 0x9B + UNIT_END-2,

                                                            //344
    PLAYER_VISIBLE_ITEM_8_CREATOR             = 0x9C + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_8_0                   = 0x9E + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_8_PROPERTIES          = 0xA6 + UNIT_END-2,
                                                            //355
    PLAYER_VISIBLE_ITEM_8_PAD                 = 0xA7 + UNIT_END-2,

                                                            //356
    PLAYER_VISIBLE_ITEM_9_CREATOR             = 0xA8 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_9_0                   = 0xAA + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_9_PROPERTIES          = 0xB2 + UNIT_END-2,
                                                            //367
    PLAYER_VISIBLE_ITEM_9_PAD                 = 0xB3 + UNIT_END-2,

                                                            //368
    PLAYER_VISIBLE_ITEM_10_CREATOR            = 0xB4 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_10_0                  = 0xB6 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_10_PROPERTIES         = 0xBE + UNIT_END-2,
                                                            //379
    PLAYER_VISIBLE_ITEM_10_PAD                = 0xBF + UNIT_END-2,

                                                            //380
    PLAYER_VISIBLE_ITEM_11_CREATOR            = 0xC0 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_11_0                  = 0xC2 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_11_PROPERTIES         = 0xCA + UNIT_END-2,
                                                            //391
    PLAYER_VISIBLE_ITEM_11_PAD                = 0xCB + UNIT_END-2,

                                                            //392
    PLAYER_VISIBLE_ITEM_12_CREATOR            = 0xCC + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_12_0                  = 0xCE + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_12_PROPERTIES         = 0xD6 + UNIT_END-2,
                                                            //403
    PLAYER_VISIBLE_ITEM_12_PAD                = 0xD7 + UNIT_END-2,

                                                            //404
    PLAYER_VISIBLE_ITEM_13_CREATOR            = 0xD8 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_13_0                  = 0xDA + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_13_PROPERTIES         = 0xE2 + UNIT_END-2,
                                                            //415
    PLAYER_VISIBLE_ITEM_13_PAD                = 0xE3 + UNIT_END-2,

                                                            //416
    PLAYER_VISIBLE_ITEM_14_CREATOR            = 0xE4 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_14_0                  = 0xE6 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_14_PROPERTIES         = 0xEE + UNIT_END-2,
                                                            //427
    PLAYER_VISIBLE_ITEM_14_PAD                = 0xEF + UNIT_END-2,

                                                            //428
    PLAYER_VISIBLE_ITEM_15_CREATOR            = 0xF0 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_15_0                  = 0xF2 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_15_PROPERTIES         = 0xFA + UNIT_END-2,
                                                            //439
    PLAYER_VISIBLE_ITEM_15_PAD                = 0xFB + UNIT_END-2,

                                                            //440
    PLAYER_VISIBLE_ITEM_16_CREATOR            = 0xFC + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_16_0                  = 0xFE + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_16_PROPERTIES         = 0x106 + UNIT_END-2,
                                                            //451
    PLAYER_VISIBLE_ITEM_16_PAD                = 0x107 + UNIT_END-2,

                                                            //452
    PLAYER_VISIBLE_ITEM_17_CREATOR            = 0x108 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_17_0                  = 0x10A + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_17_PROPERTIES         = 0x112 + UNIT_END-2,
                                                            //463
    PLAYER_VISIBLE_ITEM_17_PAD                = 0x113 + UNIT_END-2,

                                                            //464
    PLAYER_VISIBLE_ITEM_18_CREATOR            = 0x114 + UNIT_END-2,
    //466 ranged
    PLAYER_VISIBLE_ITEM_18_0                  = 0x116 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_18_PROPERTIES         = 0x11E + UNIT_END-2,
                                                            //475
    PLAYER_VISIBLE_ITEM_18_PAD                = 0x11F + UNIT_END-2,

                                                            //476
    PLAYER_VISIBLE_ITEM_19_CREATOR            = 0x120 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_19_0                  = 0x122 + UNIT_END-2,
    PLAYER_VISIBLE_ITEM_19_PROPERTIES         = 0x12A + UNIT_END-2,
                                                            //487
    PLAYER_VISIBLE_ITEM_19_PAD                = 0x12B + UNIT_END-2,

    PLAYER_FIELD_INV_SLOT_HEAD                = 0x12C + UNIT_END-2,

    PLAYER_FIELD_PACK_SLOT_1                  = 0x15A + UNIT_END-2,
    PLAYER_FIELD_PACK_SLOT_2                  = 0x15B + UNIT_END-2,

    PLAYER_FIELD_BANK_SLOT_1                  = 0x17A + UNIT_END-2,
    PLAYER_FIELD_BANKBAG_SLOT_1               = 0x1AA + UNIT_END-2,
    PLAYER_FIELD_VENDORBUYBACK_SLOT_1         = 0x1B6 + UNIT_END-2,
    //PLAYER_FIELD_KEYRING_SLOT_1
    PLAYER_FARSIGHT                           = 0x1CE + UNIT_END+64-2,
    PLAYER_FIELD_COMBO_TARGET                 = 0x1D0 + UNIT_END+64-2,
    PLAYER_XP                                 = 0x1D2 + UNIT_END+64-2,
    PLAYER_NEXT_LEVEL_XP                      = 0x1D3 + UNIT_END+64-2,
    PLAYER_SKILL_INFO_START                   = 0x1D4 + UNIT_END+64-2,

    PLAYER_CHARACTER_POINTS1                   = 0x354 + PLAYER_SELECTION+64-2,
    PLAYER_CHARACTER_POINTS2                   = 0x355 + PLAYER_SELECTION+64-2,
    PLAYER_TRACK_CREATURES                     = 0x356 + PLAYER_SELECTION+64-2,
    PLAYER_TRACK_RESOURCES                     = 0x357 + PLAYER_SELECTION+64-2,
    PLAYER_BLOCK_PERCENTAGE                    = 0x358 + PLAYER_SELECTION+64-2,
    PLAYER_DODGE_PERCENTAGE                    = 0x359 + PLAYER_SELECTION+64-2,
    PLAYER_PARRY_PERCENTAGE                    = 0x35A + PLAYER_SELECTION+64-2,
    PLAYER_CRIT_PERCENTAGE                     = 0x35B + PLAYER_SELECTION+64-2,
    PLAYER_RANGED_CRIT_PERCENTAGE              = 0x35C + PLAYER_SELECTION+64-2,
    PLAYER_EXPLORED_ZONES_1                    = 0x35D + PLAYER_SELECTION+64-2,
    PLAYER_REST_STATE_EXPERIENCE               = 0x39D + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_COINAGE                       = 0x39E + PLAYER_SELECTION+64-2,
                                                            // used at server in float format but send to client in uint32 format
    PLAYER_FIELD_POSSTAT0                      = 0x39F + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_POSSTAT1                      = 0x3A0 + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_POSSTAT2                      = 0x3A1 + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_POSSTAT3                      = 0x3A2 + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_POSSTAT4                      = 0x3A3 + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_NEGSTAT0                      = 0x3A4 + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_NEGSTAT1                      = 0x3A5 + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_NEGSTAT2                      = 0x3A6 + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_NEGSTAT3                      = 0x3A7 + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_NEGSTAT4                      = 0x3A8 + PLAYER_SELECTION+64-2,
                                                            // used at server in float format but send to client in uint32 format
    PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE    = 0x3A9 + PLAYER_SELECTION+64-2,
                                                            // used at server in float format but send to client in uint32 format
    PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_01 = 0x3AA + PLAYER_SELECTION+64-2,
                                                            // used at server in float format but send to client in uint32 format
    PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_02 = 0x3AB + PLAYER_SELECTION+64-2,
                                                            // used at server in float format but send to client in uint32 format
    PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_03 = 0x3AC + PLAYER_SELECTION+64-2,
                                                            // used at server in float format but send to client in uint32 format
    PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_04 = 0x3AD + PLAYER_SELECTION+64-2,
                                                            // used at server in float format but send to client in uint32 format
    PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_05 = 0x3AE + PLAYER_SELECTION+64-2,
                                                            // used at server in float format but send to client in uint32 format
    PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_06 = 0x3AF + PLAYER_SELECTION+64-2,
                                                            // used at server in float format but send to client in uint32 format
    PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE    = 0x3B0 + PLAYER_SELECTION+64-2,
                                                            // used at server in float format but send to client in uint32 format
    PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_01 = 0x3B1 + PLAYER_SELECTION+64-2,
                                                            // used at server in float format but send to client in uint32 format
    PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_02 = 0x3B2 + PLAYER_SELECTION+64-2,
                                                            // used at server in float format but send to client in uint32 format
    PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_03 = 0x3B3 + PLAYER_SELECTION+64-2,
                                                            // used at server in float format but send to client in uint32 format
    PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_04 = 0x3B4 + PLAYER_SELECTION+64-2,
                                                            // used at server in float format but send to client in uint32 format
    PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_05 = 0x3B5 + PLAYER_SELECTION+64-2,
                                                            // used at server in float format but send to client in uint32 format
    PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_06 = 0x3B6 + PLAYER_SELECTION+64-2,
                                                            //float 1.0
    PLAYER_FIELD_MOD_DAMAGE_DONE_POS          = 0x3B7 + PLAYER_SELECTION+64-2,
                                                            //float 1.0
    PLAYER_FIELD_MOD_DAMAGE_DONE_NEG          = 0x3BE + PLAYER_SELECTION+64-2,
                                                            //float 1.0
    PLAYER_FIELD_MOD_DAMAGE_DONE_PCT          = 0x3C5 + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_BYTES                        = 0x3CC + PLAYER_SELECTION+64-2,
    PLAYER_AMMO_ID                            = 0x3CD + PLAYER_SELECTION+64-2,
    PLAYER_SELF_RES_SPELL                     = 0x3CE + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_PVP_MEDALS                   = 0x3CF + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_BUYBACK_PRICE_1              = 0x3D0 + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_BUYBACK_TIMESTAMP_1          = 0x3DC + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_SESSION_KILLS                = 0x3E8 + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_YESTERDAY_KILLS              = 0x3E9 + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_LAST_WEEK_KILLS              = 0x3EA + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_THIS_WEEK_KILLS              = 0x3EB + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_THIS_WEEK_CONTRIBUTION       = 0x3EC + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_LIFETIME_HONORABLE_KILLS     = 0x3ED + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_LIFETIME_DISHONORABLE_KILLS  = 0x3EE + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_YESTERDAY_CONTRIBUTION       = 0x3EF + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_LAST_WEEK_CONTRIBUTION       = 0x3F0 + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_LAST_WEEK_RANK               = 0x3F1 + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_HONOR_BAR                    = 0x3F2 + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_BYTES2                       = 0x3F2 + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_WATCHED_FACTION_INDEX        = 0x3F3 + PLAYER_SELECTION+64-2,
    PLAYER_FIELD_COMBAT_RATING_1              = 0x3F4 + PLAYER_SELECTION+64-2,
    PLAYER_END                                = 0x3F4 + PLAYER_SELECTION+64-2+20,
};

enum EGameObjectFields
{
    OBJECT_FIELD_CREATED_BY                 =    6,
    GAMEOBJECT_DISPLAYID                    =    8,
    GAMEOBJECT_FLAGS                        =    9,
    GAMEOBJECT_ROTATION                     =   10,
    GAMEOBJECT_STATE                        =   14,
    /*
    GAMEOBJECT_TIMESTAMP                    =   15,
    GAMEOBJECT_POS_X                        =   16,
    GAMEOBJECT_POS_Y                        =   17,
    GAMEOBJECT_POS_Z                        =   18,
    GAMEOBJECT_FACING                       =   19,
    GAMEOBJECT_DYN_FLAGS                    =   20,
    GAMEOBJECT_FACTION                      =   21,
    GAMEOBJECT_TYPE_ID                      =   22,
    GAMEOBJECT_LEVEL                        =   23,
    GAMEOBJECT_END                          =   24,
    */
    GAMEOBJECT_POS_X                       =   15,
    GAMEOBJECT_POS_Y                       =   16,
    GAMEOBJECT_POS_Z                       =   17,
    GAMEOBJECT_FACING                      =   18,
    GAMEOBJECT_DYN_FLAGS                   =   19,
    GAMEOBJECT_FACTION                     =   20,
    GAMEOBJECT_TYPE_ID                     =   21,
    GAMEOBJECT_LEVEL                       =   22,
    GAMEOBJECT_ARTKIT                      =   23,
    GAMEOBJECT_ANIMPROGRESS                =   24,
    GAMEOBJECT_PADDING                     =   25,
    GAMEOBJECT_END                         =   26,
};

enum EDynamicObjectFields
{
    DYNAMICOBJECT_CASTER                    =    6,
    DYNAMICOBJECT_BYTES                     =    8,
    DYNAMICOBJECT_SPELLID                   =    9,
    DYNAMICOBJECT_RADIUS                    =   10,
    DYNAMICOBJECT_POS_X                     =   11,
    DYNAMICOBJECT_POS_Y                     =   12,
    DYNAMICOBJECT_POS_Z                     =   13,
    DYNAMICOBJECT_FACING                    =   14,
    DYNAMICOBJECT_PAD                       =   15,
    DYNAMICOBJECT_END                       =   16,
};

enum ECorpseFields
{
    CORPSE_FIELD_OWNER                      =    6,
    CORPSE_FIELD_FACING                     =    8,
    CORPSE_FIELD_POS_X                      =    9,
    CORPSE_FIELD_POS_Y                      =   10,
    CORPSE_FIELD_POS_Z                      =   11,
    CORPSE_FIELD_DISPLAY_ID                 =   12,
    CORPSE_FIELD_ITEM                       =   13,
    CORPSE_FIELD_BYTES_1                    =   32,
    CORPSE_FIELD_BYTES_2                    =   33,
    CORPSE_FIELD_GUILD                      =   34,
    CORPSE_FIELD_FLAGS                      =   35,
    CORPSE_FIELD_DYNAMIC_FLAGS              =   36,
    CORPSE_FIELD_PAD                        =   37,
    CORPSE_END                              =   38,
};
#endif
