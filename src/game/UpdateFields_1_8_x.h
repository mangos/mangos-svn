/* UpdateFields.h
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

#ifdef _VERSION_1_8_0_

#include "Common.h"

#ifndef _UPDATEFIELDS_AUTO_H
#define _UPDATEFIELDS_AUTO_H

enum EObjectFields
{
    OBJECT_FIELD_GUID                       =    0,
    OBJECT_FIELD_TYPE                       =    2,
    OBJECT_FIELD_ENTRY                      =    3,
    OBJECT_FIELD_SCALE_X                    =    4,
    OBJECT_FIELD_PADDING                    =    5,
    OBJECT_END                              =    6,
};

enum EItemFields
{
    ITEM_FIELD_OWNER                        =    6,
    ITEM_FIELD_CONTAINED                    =    8,
    ITEM_FIELD_CREATOR                      =   10,
    ITEM_FIELD_GIFTCREATOR                  =   12,
    ITEM_FIELD_STACK_COUNT                  =   14,
    ITEM_FIELD_DURATION                     =   15,
    ITEM_FIELD_SPELL_CHARGES                =   16,      //  5 of them
    ITEM_FIELD_FLAGS                        =   21,
    ITEM_FIELD_ENCHANTMENT                  =   22,      //  21 of them
    ITEM_FIELD_PROPERTY_SEED                =   43,
    ITEM_FIELD_RANDOM_PROPERTIES_ID         =   44,
    ITEM_FIELD_ITEM_TEXT_ID                 =   45,
    ITEM_FIELD_DURABILITY                   =   46,
    ITEM_FIELD_MAXDURABILITY                =   47,
    ITEM_END                                =   48,
};

enum EContainerFields
{
    CONTAINER_FIELD_NUM_SLOTS               =   48,
    CONTAINER_ALIGN_PAD                     =   49,
    CONTAINER_FIELD_SLOT_1                  =   50,      //  40 of them
    CONTAINER_END                           =   90,
};

enum EUnitFields
{
    UNIT_FIELD_CHARM                        =    6,
    UNIT_FIELD_SUMMON                       =    8,
    UNIT_FIELD_CHARMEDBY                    =   10,
    UNIT_FIELD_SUMMONEDBY                   =   12,
    UNIT_FIELD_CREATEDBY                    =   14,
    UNIT_FIELD_TARGET                       =   16,
    UNIT_FIELD_PERSUADED                    =   18,
    UNIT_FIELD_CHANNEL_OBJECT               =   20,
    UNIT_FIELD_HEALTH                       =   22,
    UNIT_FIELD_POWER1                       =   23,
    UNIT_FIELD_POWER2                       =   24,
    UNIT_FIELD_POWER3                       =   25,
    UNIT_FIELD_POWER4                       =   26,
    UNIT_FIELD_POWER5                       =   27,
    UNIT_FIELD_MAXHEALTH                    =   28,
    UNIT_FIELD_MAXPOWER1                    =   29,
    UNIT_FIELD_MAXPOWER2                    =   30,
    UNIT_FIELD_MAXPOWER3                    =   31,
    UNIT_FIELD_MAXPOWER4                    =   32,
    UNIT_FIELD_MAXPOWER5                    =   33,
    UNIT_FIELD_LEVEL                        =   34,
    UNIT_FIELD_FACTIONTEMPLATE              =   35,
    UNIT_FIELD_BYTES_0                      =   36,
    UNIT_VIRTUAL_ITEM_SLOT_DISPLAY          =   37,
    UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_01       =   38,
    UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_02       =   39,
    UNIT_VIRTUAL_ITEM_INFO                  =   40,       //  6 of them
    UNIT_FIELD_FLAGS                        =   46,
    UNIT_FIELD_AURA                         =   47,       //  64 of them
    UNIT_FIELD_AURAFLAGS                    =  111,       //  8 of them
    UNIT_FIELD_AURALEVELS                   =  119,       //  8 of them
    UNIT_FIELD_AURAAPPLICATIONS             =  127,       //  16 of them
    UNIT_FIELD_AURASTATE                    =  143,
    UNIT_FIELD_BASEATTACKTIME               =  144,
    UNIT_FIELD_RANGEDATTACKTIME             =  146,
    UNIT_FIELD_BOUNDINGRADIUS               =  147,
    UNIT_FIELD_COMBATREACH                  =  148,
    UNIT_FIELD_DISPLAYID                    =  149,
    UNIT_FIELD_NATIVEDISPLAYID              =  150,
    UNIT_FIELD_MOUNTDISPLAYID               =  151,
    UNIT_FIELD_MINDAMAGE                    =  152,
    UNIT_FIELD_MAXDAMAGE                    =  153,
    UNIT_FIELD_MINOFFHANDDAMAGE             =  154,
    UNIT_FIELD_MAXOFFHANDDAMAGE             =  155,
    UNIT_FIELD_BYTES_1                      =  156,
    UNIT_FIELD_PETNUMBER                    =  157,
    UNIT_FIELD_PET_NAME_TIMESTAMP           =  158,
    UNIT_FIELD_PETEXPERIENCE                =  159,
    UNIT_FIELD_PETNEXTLEVELEXP              =  160,
    UNIT_DYNAMIC_FLAGS                      =  161,
    UNIT_CHANNEL_SPELL                      =  162,
    UNIT_MOD_CAST_SPEED                     =  163,
    UNIT_CREATED_BY_SPELL                   =  164,
    UNIT_NPC_FLAGS                          =  165,
    UNIT_NPC_EMOTESTATE                     =  166,
    UNIT_TRAINING_POINTS                    =  167,
    UNIT_FIELD_STAT0                        =  168,
    UNIT_FIELD_STR = UNIT_FIELD_STAT0,
    UNIT_FIELD_STAT1                        =  169,
    UNIT_FIELD_AGILITY = UNIT_FIELD_STAT1,
    UNIT_FIELD_STAT2                        =  170,
    UNIT_FIELD_STAMINA = UNIT_FIELD_STAT2,
    UNIT_FIELD_STAT3                        =  171,
    UNIT_FIELD_SPIRIT = UNIT_FIELD_STAT3,
    UNIT_FIELD_STAT4                        =  172,
    UNIT_FIELD_IQ = UNIT_FIELD_STAT4,
    UNIT_FIELD_ARMOR = UNIT_FIELD_STAT4,    // UQ1: I dont think this is correct! Was IQ +1 in old code...
    UNIT_FIELD_RESISTANCES                  =  173,
    UNIT_FIELD_RESISTANCES_01               =  174,
    UNIT_FIELD_RESISTANCES_02               =  175,
    UNIT_FIELD_RESISTANCES_03               =  176,
    UNIT_FIELD_RESISTANCES_04               =  177,
    UNIT_FIELD_RESISTANCES_05               =  178,
    UNIT_FIELD_RESISTANCES_06               =  179,
    UNIT_FIELD_ATTACKPOWER                  =  180,
    UNIT_FIELD_BASE_MANA                    =  181,
    UNIT_FIELD_BASE_HEALTH                  =  182,
    UNIT_FIELD_ATTACK_POWER_MODS            =  183,
    UNIT_FIELD_BYTES_2                      =  184,
    UNIT_FIELD_RANGEDATTACKPOWER            =  185,
    UNIT_FIELD_RANGED_ATTACK_POWER_MODS     =  186,
    UNIT_FIELD_MINRANGEDDAMAGE              =  187,
    UNIT_FIELD_MAXRANGEDDAMAGE              =  188,
    UNIT_FIELD_POWER_COST_MODIFIER          =  189,
    UNIT_FIELD_POWER_COST_MULTIPLIER        =  190,
    UNIT_FIELD_PADDING                      =  191,
    UNIT_FIELD_UNKNOWN180                   =  192,      //  12 of them
    UNIT_END                                =  204,
};

enum EPlayerFields
{
    PLAYER_SELECTION                        =  204,
    PLAYER_DUEL_ARBITER                     =  206,
    PLAYER_FLAGS                            =  208,
    PLAYER_GUILDID                          =  209,
    PLAYER_GUILDRANK                        =  210,
    PLAYER_BYTES                            =  211,
    PLAYER_BYTES_2                          =  212,
    PLAYER_BYTES_3                          =  213,
    PLAYER_DUEL_TEAM                        =  214,
    PLAYER_GUILD_TIMESTAMP                  =  215,
    PLAYER_QUEST_LOG_1_1                    =  216,
    PLAYER_QUEST_LOG_1_2                    =  217,
    PLAYER_QUEST_LOG_2_1                    =  219,
    PLAYER_QUEST_LOG_2_2                    =  220,
    PLAYER_QUEST_LOG_3_1                    =  222,
    PLAYER_QUEST_LOG_3_2                    =  223,
    PLAYER_QUEST_LOG_4_1                    =  225,
    PLAYER_QUEST_LOG_4_2                    =  226,
    PLAYER_QUEST_LOG_5_1                    =  228,
    PLAYER_QUEST_LOG_5_2                    =  229,
    PLAYER_QUEST_LOG_6_1                    =  231,
    PLAYER_QUEST_LOG_6_2                    =  232,
    PLAYER_QUEST_LOG_7_1                    =  234,
    PLAYER_QUEST_LOG_7_2                    =  235,
    PLAYER_QUEST_LOG_8_1                    =  237,
    PLAYER_QUEST_LOG_8_2                    =  238,
    PLAYER_QUEST_LOG_9_1                    =  240,
    PLAYER_QUEST_LOG_9_2                    =  241,
    PLAYER_QUEST_LOG_10_1                   =  243,
    PLAYER_QUEST_LOG_10_2                   =  244,
    PLAYER_QUEST_LOG_11_1                   =  246,
    PLAYER_QUEST_LOG_11_2                   =  247,
    PLAYER_QUEST_LOG_12_1                   =  249,
    PLAYER_QUEST_LOG_12_2                   =  250,
    PLAYER_QUEST_LOG_13_1                   =  252,
    PLAYER_QUEST_LOG_13_2                   =  253,
    PLAYER_QUEST_LOG_14_1                   =  255,
    PLAYER_QUEST_LOG_14_2                   =  256,
    PLAYER_QUEST_LOG_15_1                   =  258,
    PLAYER_QUEST_LOG_15_2                   =  259,
    PLAYER_QUEST_LOG_16_1                   =  261,
    PLAYER_QUEST_LOG_16_2                   =  262,
    PLAYER_QUEST_LOG_17_1                   =  264,
    PLAYER_QUEST_LOG_17_2                   =  265,
    PLAYER_QUEST_LOG_18_1                   =  267,
    PLAYER_QUEST_LOG_18_2                   =  268,
    PLAYER_QUEST_LOG_19_1                   =  270,
    PLAYER_QUEST_LOG_19_2                   =  271,
    PLAYER_QUEST_LOG_20_1                   =  273,
    PLAYER_QUEST_LOG_20_2                   =  274,
    PLAYER_VISIBLE_ITEM_1_CREATOR           =  276,
    PLAYER_VISIBLE_ITEM_1_0                 =  278,      //  8 of them
    PLAYER_VISIBLE_ITEM_1_PROPERTIES        =  286,
    PLAYER_VISIBLE_ITEM_1_PAD               =  287,
    PLAYER_VISIBLE_ITEM_2_CREATOR           =  288,
    PLAYER_VISIBLE_ITEM_2_0                 =  290,      //  8 of them
    PLAYER_VISIBLE_ITEM_2_PROPERTIES        =  298,
    PLAYER_VISIBLE_ITEM_2_PAD               =  299,
    PLAYER_VISIBLE_ITEM_3_CREATOR           =  300,
    PLAYER_VISIBLE_ITEM_3_0                 =  302,      //  8 of them
    PLAYER_VISIBLE_ITEM_3_PROPERTIES        =  310,
    PLAYER_VISIBLE_ITEM_3_PAD               =  311,
    PLAYER_VISIBLE_ITEM_4_CREATOR           =  312,
    PLAYER_VISIBLE_ITEM_4_0                 =  314,      //  8 of them
    PLAYER_VISIBLE_ITEM_4_PROPERTIES        =  322,
    PLAYER_VISIBLE_ITEM_4_PAD               =  323,
    PLAYER_VISIBLE_ITEM_5_CREATOR           =  324,
    PLAYER_VISIBLE_ITEM_5_0                 =  326,      //  8 of them
    PLAYER_VISIBLE_ITEM_5_PROPERTIES        =  334,
    PLAYER_VISIBLE_ITEM_5_PAD               =  335,
    PLAYER_VISIBLE_ITEM_6_CREATOR           =  336,
    PLAYER_VISIBLE_ITEM_6_0                 =  338,      //  8 of them
    PLAYER_VISIBLE_ITEM_6_PROPERTIES        =  346,
    PLAYER_VISIBLE_ITEM_6_PAD               =  347,
    PLAYER_VISIBLE_ITEM_7_CREATOR           =  348,
    PLAYER_VISIBLE_ITEM_7_0                 =  350,      //  8 of them
    PLAYER_VISIBLE_ITEM_7_PROPERTIES        =  358,
    PLAYER_VISIBLE_ITEM_7_PAD               =  359,
    PLAYER_VISIBLE_ITEM_8_CREATOR           =  360,
    PLAYER_VISIBLE_ITEM_8_0                 =  362,      //  8 of them
    PLAYER_VISIBLE_ITEM_8_PROPERTIES        =  370,
    PLAYER_VISIBLE_ITEM_8_PAD               =  371,
    PLAYER_VISIBLE_ITEM_9_CREATOR           =  372,
    PLAYER_VISIBLE_ITEM_9_0                 =  374,      //  8 of them
    PLAYER_VISIBLE_ITEM_9_PROPERTIES        =  382,
    PLAYER_VISIBLE_ITEM_9_PAD               =  383,
    PLAYER_VISIBLE_ITEM_10_CREATOR          =  384,
    PLAYER_VISIBLE_ITEM_10_0                =  386,      //  8 of them
    PLAYER_VISIBLE_ITEM_10_PROPERTIES       =  394,
    PLAYER_VISIBLE_ITEM_10_PAD              =  395,
    PLAYER_VISIBLE_ITEM_11_CREATOR          =  396,
    PLAYER_VISIBLE_ITEM_11_0                =  398,      //  8 of them
    PLAYER_VISIBLE_ITEM_11_PROPERTIES       =  406,
    PLAYER_VISIBLE_ITEM_11_PAD              =  407,
    PLAYER_VISIBLE_ITEM_12_CREATOR          =  408,
    PLAYER_VISIBLE_ITEM_12_0                =  410,      //  8 of them
    PLAYER_VISIBLE_ITEM_12_PROPERTIES       =  418,
    PLAYER_VISIBLE_ITEM_12_PAD              =  419,
    PLAYER_VISIBLE_ITEM_13_CREATOR          =  420,
    PLAYER_VISIBLE_ITEM_13_0                =  422,      //  8 of them
    PLAYER_VISIBLE_ITEM_13_PROPERTIES       =  430,
    PLAYER_VISIBLE_ITEM_13_PAD              =  431,
    PLAYER_VISIBLE_ITEM_14_CREATOR          =  432,
    PLAYER_VISIBLE_ITEM_14_0                =  434,      //  8 of them
    PLAYER_VISIBLE_ITEM_14_PROPERTIES       =  442,
    PLAYER_VISIBLE_ITEM_14_PAD              =  443,
    PLAYER_VISIBLE_ITEM_15_CREATOR          =  444,
    PLAYER_VISIBLE_ITEM_15_0                =  446,      //  8 of them
    PLAYER_VISIBLE_ITEM_15_PROPERTIES       =  454,
    PLAYER_VISIBLE_ITEM_15_PAD              =  455,
    PLAYER_VISIBLE_ITEM_16_CREATOR          =  456,
    PLAYER_VISIBLE_ITEM_16_0                =  458,      //  8 of them
    PLAYER_VISIBLE_ITEM_16_PROPERTIES       =  466,
    PLAYER_VISIBLE_ITEM_16_PAD              =  467,
    PLAYER_VISIBLE_ITEM_17_CREATOR          =  468,
    PLAYER_VISIBLE_ITEM_17_0                =  470,      //  8 of them
    PLAYER_VISIBLE_ITEM_17_PROPERTIES       =  478,
    PLAYER_VISIBLE_ITEM_17_PAD              =  479,
    PLAYER_VISIBLE_ITEM_18_CREATOR          =  480,
    PLAYER_VISIBLE_ITEM_18_0                =  482,      //  8 of them
    PLAYER_VISIBLE_ITEM_18_PROPERTIES       =  490,
    PLAYER_VISIBLE_ITEM_18_PAD              =  491,
    PLAYER_VISIBLE_ITEM_19_CREATOR          =  492,
    PLAYER_VISIBLE_ITEM_19_0                =  494,      //  8 of them
    PLAYER_VISIBLE_ITEM_19_PROPERTIES       =  502,
    PLAYER_VISIBLE_ITEM_19_PAD              =  503,
    PLAYER_FIELD_INV_SLOT_HEAD              =  504,      //  46 of them
    PLAYER_FIELD_PACK_SLOT_1                =  550,      //  32 of them
    PLAYER_FIELD_BANK_SLOT_1                =  582,      //  48 of them
    PLAYER_FIELD_BANKBAG_SLOT_1             =  630,      //  12 of them
    PLAYER_FIELD_VENDORBUYBACK_SLOT_1       =  642,      //  24 of them
    PLAYER_FARSIGHT                         =  666,
    PLAYER__FIELD_COMBO_TARGET              =  668,
    PLAYER_XP                               =  670,
    PLAYER_NEXT_LEVEL_XP                    =  671,
    PLAYER_SKILL_INFO_1_1                   =  672,         //  384 of them
    PLAYER_SKILL_INFO_1_1_381  = PLAYER_SKILL_INFO_1_1+381, //  384 of them
    PLAYER_CHARACTER_POINTS1                = 1056,
    PLAYER_CHARACTER_POINTS2                = 1057,
    PLAYER_TRACK_CREATURES                  = 1058,
    PLAYER_TRACK_RESOURCES                  = 1059,
    PLAYER_BLOCK_PERCENTAGE                 = 1060,
    PLAYER_DODGE_PERCENTAGE                 = 1061,
    PLAYER_PARRY_PERCENTAGE                 = 1062,
    PLAYER_CRIT_PERCENTAGE                  = 1063,
    PLAYER_RANGED_CRIT_PERCENTAGE           = 1064,
    PLAYER_EXPLORED_ZONES_1                 = 1065,         //  64 of them
    PLAYER_REST_STATE_EXPERIENCE            = 1129,
    PLAYER_FIELD_COINAGE                    = 1130,
    PLAYER_FIELD_POSSTAT0                   = 1131,
    PLAYER_FIELD_POSSTAT1                   = 1132,
    PLAYER_FIELD_POSSTAT2                   = 1133,
    PLAYER_FIELD_POSSTAT3                   = 1134,
    PLAYER_FIELD_POSSTAT4                   = 1135,
    PLAYER_FIELD_NEGSTAT0                   = 1136,
    PLAYER_FIELD_NEGSTAT1                   = 1137,
    PLAYER_FIELD_NEGSTAT2                   = 1138,
    PLAYER_FIELD_NEGSTAT3                   = 1139,
    PLAYER_FIELD_NEGSTAT4                   = 1140,
    PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE = 1141,
    PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_01 = 1142,
    PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_02 = 1143,
    PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_03 = 1144,
    PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_04 = 1145,
    PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_05 = 1146,
    PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_06 = 1147,
    PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE = 1148,
    PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_01 = 1149,
    PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_02 = 1150,
    PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_03 = 1151,
    PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_04 = 1152,
    PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_05 = 1153,
    PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_06 = 1154,
    PLAYER_FIELD_MOD_DAMAGE_DONE_POS        = 1155,
    PLAYER_FIELD_MOD_DAMAGE_DONE_NEG        = 1162,
    PLAYER_FIELD_MOD_DAMAGE_DONE_PCT        = 1169,
    PLAYER_FIELD_BYTES                      = 1176,
    PLAYER_AMMO_ID                          = 1177,
    PLAYER_SELF_RES_SPELL                   = 1178,
    PLAYER_FIELD_PVP_MEDALS                 = 1179,
    PLAYER_FIELD_BUYBACK_PRICE_1            = 1180,
    PLAYER_FIELD_BUYBACK_TIMESTAMP_1        = 1181,
    PLAYER_FIELD_SESSION_KILLS              = 1182,
    PLAYER_FIELD_YESTERDAY_KILLS            = 1183,
    PLAYER_FIELD_LAST_WEEK_KILLS            = 1184,
    PLAYER_FIELD_THIS_WEEK_KILLS            = 1185,
    PLAYER_FIELD_THIS_WEEK_CONTRIBUTION     = 1186,
    PLAYER_FIELD_LIFETIME_HONORBALE_KILLS   = 1187,
    PLAYER_FIELD_LIFETIME_DISHONORBALE_KILLS= 1188,
    PLAYER_FIELD_YESTERDAY_CONTRIBUTION     = 1189,
    PLAYER_FIELD_LAST_WEEK_CONTRIBUTION     = 1190,
    PLAYER_FIELD_LAST_WEEK_RANK             = 1191,
    PLAYER_FIELD_BYTES2                     = 1192,
    PLAYER_FIELD_PADDING                    = 1193,
    PLAYER_FIELD_UNKNOWN180                 = 1194,      //  44 of them
    PLAYER_END                              = 1238,
};

enum EGameObjectFields
{
    OBJECT_FIELD_CREATED_BY                 =    6,
    GAMEOBJECT_DISPLAYID                    =    8,
    GAMEOBJECT_FLAGS                        =    9,
    GAMEOBJECT_ROTATION                     =   10,      //  4 of them
    GAMEOBJECT_STATE                        =   14,
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
    CORPSE_FIELD_ITEM                       =   13,      //  19 of them
    CORPSE_FIELD_BYTES_1                    =   32,
    CORPSE_FIELD_BYTES_2                    =   33,
    CORPSE_FIELD_GUILD                      =   34,
    CORPSE_FIELD_FLAGS                      =   35,
    CORPSE_FIELD_DYNAMIC_FLAGS              =   36,
    CORPSE_FIELD_PAD                        =   37,
    CORPSE_END                              =   38,
};

#endif

#endif //_VERSION_1_8_0_

