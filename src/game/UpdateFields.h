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

#ifndef _UPDATEFIELDS_AUTO_H
#define _UPDATEFIELDS_AUTO_H

// Auto generated for version 2.1.3, build 6898

enum EObjectFields
{
    OBJECT_FIELD_GUID                         = 0x0000,     // 2 4 1
    OBJECT_FIELD_TYPE                         = 0x0002,     // 1 1 1
    OBJECT_FIELD_ENTRY                        = 0x0003,     // 1 1 1
    OBJECT_FIELD_SCALE_X                      = 0x0004,     // 1 3 1
    OBJECT_FIELD_PADDING                      = 0x0005,     // 1 1 0
    OBJECT_END                                = 0x0006,
};

enum EItemFields
{
    ITEM_FIELD_OWNER                          = 0x0006,     // 2 4 1
    ITEM_FIELD_CONTAINED                      = 0x0008,     // 2 4 1
    ITEM_FIELD_CREATOR                        = 0x000A,     // 2 4 1
    ITEM_FIELD_GIFTCREATOR                    = 0x000C,     // 2 4 1
    ITEM_FIELD_STACK_COUNT                    = 0x000E,     // 1 1 20
    ITEM_FIELD_DURATION                       = 0x000F,     // 1 1 20
    ITEM_FIELD_SPELL_CHARGES                  = 0x0010,     // 5 1 20
    ITEM_FIELD_FLAGS                          = 0x0015,     // 1 1 1
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
    ITEM_FIELD_ENCHANTMENT                    = 0x0016,     // 33 1 1
    ITEM_FIELD_PROPERTY_SEED                  = 0x0037,     // 1 1 1
                                                            // custom
    ITEM_FIELD_SUFFIX_FACTOR                  = ITEM_FIELD_PROPERTY_SEED,
    ITEM_FIELD_RANDOM_PROPERTIES_ID           = 0x0038,     // 1 1 1
    ITEM_FIELD_ITEM_TEXT_ID                   = 0x0039,     // 1 1 4
    ITEM_FIELD_DURABILITY                     = 0x003A,     // 1 1 20
    ITEM_FIELD_MAXDURABILITY                  = 0x003B,     // 1 1 20
    ITEM_END                                  = 0x003C,
};

enum EContainerFields
{
    CONTAINER_FIELD_NUM_SLOTS                 = 0x003C,     // 1 1 1
    CONTAINER_ALIGN_PAD                       = 0x003D,     // 1 5 0
    CONTAINER_FIELD_SLOT_1                    = 0x003E,     // 72 4 1
    CONTAINER_END                             = 0x0086,
};

enum EUnitFields
{
    UNIT_FIELD_CHARM                          = 0x0006,     // 2 4 1
    UNIT_FIELD_SUMMON                         = 0x0008,     // 2 4 1
    UNIT_FIELD_CHARMEDBY                      = 0x000A,     // 2 4 1
    UNIT_FIELD_SUMMONEDBY                     = 0x000C,     // 2 4 1
    UNIT_FIELD_CREATEDBY                      = 0x000E,     // 2 4 1
    UNIT_FIELD_TARGET                         = 0x0010,     // 2 4 1
    UNIT_FIELD_PERSUADED                      = 0x0012,     // 2 4 1
    UNIT_FIELD_CHANNEL_OBJECT                 = 0x0014,     // 2 4 1
    UNIT_FIELD_HEALTH                         = 0x0016,     // 1 1 256
    UNIT_FIELD_POWER1                         = 0x0017,     // 1 1 1
    UNIT_FIELD_POWER2                         = 0x0018,     // 1 1 1
    UNIT_FIELD_POWER3                         = 0x0019,     // 1 1 1
    UNIT_FIELD_POWER4                         = 0x001A,     // 1 1 1
    UNIT_FIELD_POWER5                         = 0x001B,     // 1 1 1
    UNIT_FIELD_MAXHEALTH                      = 0x001C,     // 1 1 256
    UNIT_FIELD_MAXPOWER1                      = 0x001D,     // 1 1 1
    UNIT_FIELD_MAXPOWER2                      = 0x001E,     // 1 1 1
    UNIT_FIELD_MAXPOWER3                      = 0x001F,     // 1 1 1
    UNIT_FIELD_MAXPOWER4                      = 0x0020,     // 1 1 1
    UNIT_FIELD_MAXPOWER5                      = 0x0021,     // 1 1 1
    UNIT_FIELD_LEVEL                          = 0x0022,     // 1 1 1
    UNIT_FIELD_FACTIONTEMPLATE                = 0x0023,     // 1 1 1
    UNIT_FIELD_BYTES_0                        = 0x0024,     // 1 5 1
    UNIT_VIRTUAL_ITEM_SLOT_DISPLAY            = 0x0025,     // 3 1 1
    UNIT_VIRTUAL_ITEM_INFO                    = 0x0028,     // 6 5 1
    UNIT_FIELD_FLAGS                          = 0x002E,     // 1 1 1
    UNIT_FIELD_FLAGS_2                        = 0x002F,     // 1 1 1
    UNIT_FIELD_AURA                           = 0x0030,     // 56 1 1
    UNIT_FIELD_AURAFLAGS                      = 0x0068,     // 7 5 1
    UNIT_FIELD_AURALEVELS                     = 0x006F,     // 14 5 1
    UNIT_FIELD_AURAAPPLICATIONS               = 0x007D,     // 14 5 1
    UNIT_FIELD_AURASTATE                      = 0x008B,     // 1 1 1
    UNIT_FIELD_BASEATTACKTIME                 = 0x008C,     // 2 1 1
                                                            // custom
    UNIT_FIELD_OFFHANDATTACKTIME              = UNIT_FIELD_BASEATTACKTIME + 1,
    UNIT_FIELD_RANGEDATTACKTIME               = 0x008E,     // 1 1 2
    UNIT_FIELD_BOUNDINGRADIUS                 = 0x008F,     // 1 3 1
    UNIT_FIELD_COMBATREACH                    = 0x0090,     // 1 3 1
    UNIT_FIELD_DISPLAYID                      = 0x0091,     // 1 1 1
    UNIT_FIELD_NATIVEDISPLAYID                = 0x0092,     // 1 1 1
    UNIT_FIELD_MOUNTDISPLAYID                 = 0x0093,     // 1 1 1
    UNIT_FIELD_MINDAMAGE                      = 0x0094,     // 1 3 38
    UNIT_FIELD_MAXDAMAGE                      = 0x0095,     // 1 3 38
    UNIT_FIELD_MINOFFHANDDAMAGE               = 0x0096,     // 1 3 38
    UNIT_FIELD_MAXOFFHANDDAMAGE               = 0x0097,     // 1 3 38
    UNIT_FIELD_BYTES_1                        = 0x0098,     // 1 5 1
    UNIT_FIELD_PETNUMBER                      = 0x0099,     // 1 1 1
    UNIT_FIELD_PET_NAME_TIMESTAMP             = 0x009A,     // 1 1 1
    UNIT_FIELD_PETEXPERIENCE                  = 0x009B,     // 1 1 4
    UNIT_FIELD_PETNEXTLEVELEXP                = 0x009C,     // 1 1 4
    UNIT_DYNAMIC_FLAGS                        = 0x009D,     // 1 1 256
    UNIT_CHANNEL_SPELL                        = 0x009E,     // 1 1 1
    UNIT_MOD_CAST_SPEED                       = 0x009F,     // 1 3 1
    UNIT_CREATED_BY_SPELL                     = 0x00A0,     // 1 1 1
    UNIT_NPC_FLAGS                            = 0x00A1,     // 1 1 1
    UNIT_NPC_EMOTESTATE                       = 0x00A2,     // 1 1 1
    UNIT_TRAINING_POINTS                      = 0x00A3,     // 1 2 4
    UNIT_FIELD_STAT0                          = 0x00A4,     // 1 1 6
    UNIT_FIELD_STAT1                          = 0x00A5,     // 1 1 6
    UNIT_FIELD_STAT2                          = 0x00A6,     // 1 1 6
    UNIT_FIELD_STAT3                          = 0x00A7,     // 1 1 6
    UNIT_FIELD_STAT4                          = 0x00A8,     // 1 1 6
    UNIT_FIELD_POSSTAT0                       = 0x00A9,     // 1 1 6
    UNIT_FIELD_POSSTAT1                       = 0x00AA,     // 1 1 6
    UNIT_FIELD_POSSTAT2                       = 0x00AB,     // 1 1 6
    UNIT_FIELD_POSSTAT3                       = 0x00AC,     // 1 1 6
    UNIT_FIELD_POSSTAT4                       = 0x00AD,     // 1 1 6
    UNIT_FIELD_NEGSTAT0                       = 0x00AE,     // 1 1 6
    UNIT_FIELD_NEGSTAT1                       = 0x00AF,     // 1 1 6
    UNIT_FIELD_NEGSTAT2                       = 0x00B0,     // 1 1 6
    UNIT_FIELD_NEGSTAT3                       = 0x00B1,     // 1 1 6
    UNIT_FIELD_NEGSTAT4                       = 0x00B2,     // 1 1 6
    UNIT_FIELD_RESISTANCES                    = 0x00B3,     // 7 1 38
    UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE     = 0x00BA,     // 7 1 6
    UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE     = 0x00C1,     // 7 1 6
    UNIT_FIELD_BASE_MANA                      = 0x00C8,     // 1 1 6
    UNIT_FIELD_BASE_HEALTH                    = 0x00C9,     // 1 1 6
    UNIT_FIELD_BYTES_2                        = 0x00CA,     // 1 5 1
    UNIT_FIELD_ATTACK_POWER                   = 0x00CB,     // 1 1 6
    UNIT_FIELD_ATTACK_POWER_MODS              = 0x00CC,     // 1 2 6
    UNIT_FIELD_ATTACK_POWER_MULTIPLIER        = 0x00CD,     // 1 3 6
    UNIT_FIELD_RANGED_ATTACK_POWER            = 0x00CE,     // 1 1 6
    UNIT_FIELD_RANGED_ATTACK_POWER_MODS       = 0x00CF,     // 1 2 6
    UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER = 0x00D0,     // 1 3 6
    UNIT_FIELD_MINRANGEDDAMAGE                = 0x00D1,     // 1 3 6
    UNIT_FIELD_MAXRANGEDDAMAGE                = 0x00D2,     // 1 3 6
    UNIT_FIELD_POWER_COST_MODIFIER            = 0x00D3,     // 7 1 6
    UNIT_FIELD_POWER_COST_MULTIPLIER          = 0x00DA,     // 7 3 6
    UNIT_FIELD_PADDING                        = 0x00E1,     // 1 1 0
    UNIT_END                                  = 0x00E2,

    PLAYER_DUEL_ARBITER                       = 0x00E2,     // 2 4 1
    PLAYER_FLAGS                              = 0x00E4,     // 1 1 1
    PLAYER_GUILDID                            = 0x00E5,     // 1 1 1
    PLAYER_GUILDRANK                          = 0x00E6,     // 1 1 1
    PLAYER_BYTES                              = 0x00E7,     // 1 5 1
    PLAYER_BYTES_2                            = 0x00E8,     // 1 5 1
    PLAYER_BYTES_3                            = 0x00E9,     // 1 5 1
    PLAYER_DUEL_TEAM                          = 0x00EA,     // 1 1 1
    PLAYER_GUILD_TIMESTAMP                    = 0x00EB,     // 1 1 1
    PLAYER_QUEST_LOG_1_1                      = 0x00EC,     // 1 1 64
    PLAYER_QUEST_LOG_1_2                      = 0x00ED,     // 2 1 2
    PLAYER_QUEST_LOG_2_1                      = 0x00EF,     // 1 1 64
    PLAYER_QUEST_LOG_2_2                      = 0x00F0,     // 2 1 2
    PLAYER_QUEST_LOG_3_1                      = 0x00F2,     // 1 1 64
    PLAYER_QUEST_LOG_3_2                      = 0x00F3,     // 2 1 2
    PLAYER_QUEST_LOG_4_1                      = 0x00F5,     // 1 1 64
    PLAYER_QUEST_LOG_4_2                      = 0x00F6,     // 2 1 2
    PLAYER_QUEST_LOG_5_1                      = 0x00F8,     // 1 1 64
    PLAYER_QUEST_LOG_5_2                      = 0x00F9,     // 2 1 2
    PLAYER_QUEST_LOG_6_1                      = 0x00FB,     // 1 1 64
    PLAYER_QUEST_LOG_6_2                      = 0x00FC,     // 2 1 2
    PLAYER_QUEST_LOG_7_1                      = 0x00FE,     // 1 1 64
    PLAYER_QUEST_LOG_7_2                      = 0x00FF,     // 2 1 2
    PLAYER_QUEST_LOG_8_1                      = 0x0101,     // 1 1 64
    PLAYER_QUEST_LOG_8_2                      = 0x0102,     // 2 1 2
    PLAYER_QUEST_LOG_9_1                      = 0x0104,     // 1 1 64
    PLAYER_QUEST_LOG_9_2                      = 0x0105,     // 2 1 2
    PLAYER_QUEST_LOG_10_1                     = 0x0107,     // 1 1 64
    PLAYER_QUEST_LOG_10_2                     = 0x0108,     // 2 1 2
    PLAYER_QUEST_LOG_11_1                     = 0x010A,     // 1 1 64
    PLAYER_QUEST_LOG_11_2                     = 0x010B,     // 2 1 2
    PLAYER_QUEST_LOG_12_1                     = 0x010D,     // 1 1 64
    PLAYER_QUEST_LOG_12_2                     = 0x010E,     // 2 1 2
    PLAYER_QUEST_LOG_13_1                     = 0x0110,     // 1 1 64
    PLAYER_QUEST_LOG_13_2                     = 0x0111,     // 2 1 2
    PLAYER_QUEST_LOG_14_1                     = 0x0113,     // 1 1 64
    PLAYER_QUEST_LOG_14_2                     = 0x0114,     // 2 1 2
    PLAYER_QUEST_LOG_15_1                     = 0x0116,     // 1 1 64
    PLAYER_QUEST_LOG_15_2                     = 0x0117,     // 2 1 2
    PLAYER_QUEST_LOG_16_1                     = 0x0119,     // 1 1 64
    PLAYER_QUEST_LOG_16_2                     = 0x011A,     // 2 1 2
    PLAYER_QUEST_LOG_17_1                     = 0x011C,     // 1 1 64
    PLAYER_QUEST_LOG_17_2                     = 0x011D,     // 2 1 2
    PLAYER_QUEST_LOG_18_1                     = 0x011F,     // 1 1 64
    PLAYER_QUEST_LOG_18_2                     = 0x0120,     // 2 1 2
    PLAYER_QUEST_LOG_19_1                     = 0x0122,     // 1 1 64
    PLAYER_QUEST_LOG_19_2                     = 0x0123,     // 2 1 2
    PLAYER_QUEST_LOG_20_1                     = 0x0125,     // 1 1 64
    PLAYER_QUEST_LOG_20_2                     = 0x0126,     // 2 1 2
    PLAYER_QUEST_LOG_21_1                     = 0x0128,     // 1 1 64
    PLAYER_QUEST_LOG_21_2                     = 0x0129,     // 2 1 2
    PLAYER_QUEST_LOG_22_1                     = 0x012B,     // 1 1 64
    PLAYER_QUEST_LOG_22_2                     = 0x012C,     // 2 1 2
    PLAYER_QUEST_LOG_23_1                     = 0x012E,     // 1 1 64
    PLAYER_QUEST_LOG_23_2                     = 0x012F,     // 2 1 2
    PLAYER_QUEST_LOG_24_1                     = 0x0131,     // 1 1 64
    PLAYER_QUEST_LOG_24_2                     = 0x0132,     // 2 1 2
    PLAYER_QUEST_LOG_25_1                     = 0x0134,     // 1 1 64
    PLAYER_QUEST_LOG_25_2                     = 0x0135,     // 2 1 2
    PLAYER_VISIBLE_ITEM_1_CREATOR             = 0x0137,     // 2 4 1
    PLAYER_VISIBLE_ITEM_1_0                   = 0x0139,     // 12 1 1
    PLAYER_VISIBLE_ITEM_1_PROPERTIES          = 0x0145,     // 1 2 1
    PLAYER_VISIBLE_ITEM_1_PAD                 = 0x0146,     // 1 1 1
    PLAYER_VISIBLE_ITEM_2_CREATOR             = 0x0147,     // 2 4 1
    PLAYER_VISIBLE_ITEM_2_0                   = 0x0149,     // 12 1 1
    PLAYER_VISIBLE_ITEM_2_PROPERTIES          = 0x0155,     // 1 2 1
    PLAYER_VISIBLE_ITEM_2_PAD                 = 0x0156,     // 1 1 1
    PLAYER_VISIBLE_ITEM_3_CREATOR             = 0x0157,     // 2 4 1
    PLAYER_VISIBLE_ITEM_3_0                   = 0x0159,     // 12 1 1
    PLAYER_VISIBLE_ITEM_3_PROPERTIES          = 0x0165,     // 1 2 1
    PLAYER_VISIBLE_ITEM_3_PAD                 = 0x0166,     // 1 1 1
    PLAYER_VISIBLE_ITEM_4_CREATOR             = 0x0167,     // 2 4 1
    PLAYER_VISIBLE_ITEM_4_0                   = 0x0169,     // 12 1 1
    PLAYER_VISIBLE_ITEM_4_PROPERTIES          = 0x0175,     // 1 2 1
    PLAYER_VISIBLE_ITEM_4_PAD                 = 0x0176,     // 1 1 1
    PLAYER_VISIBLE_ITEM_5_CREATOR             = 0x0177,     // 2 4 1
    PLAYER_VISIBLE_ITEM_5_0                   = 0x0179,     // 12 1 1
    PLAYER_VISIBLE_ITEM_5_PROPERTIES          = 0x0185,     // 1 2 1
    PLAYER_VISIBLE_ITEM_5_PAD                 = 0x0186,     // 1 1 1
    PLAYER_VISIBLE_ITEM_6_CREATOR             = 0x0187,     // 2 4 1
    PLAYER_VISIBLE_ITEM_6_0                   = 0x0189,     // 12 1 1
    PLAYER_VISIBLE_ITEM_6_PROPERTIES          = 0x0195,     // 1 2 1
    PLAYER_VISIBLE_ITEM_6_PAD                 = 0x0196,     // 1 1 1
    PLAYER_VISIBLE_ITEM_7_CREATOR             = 0x0197,     // 2 4 1
    PLAYER_VISIBLE_ITEM_7_0                   = 0x0199,     // 12 1 1
    PLAYER_VISIBLE_ITEM_7_PROPERTIES          = 0x01A5,     // 1 2 1
    PLAYER_VISIBLE_ITEM_7_PAD                 = 0x01A6,     // 1 1 1
    PLAYER_VISIBLE_ITEM_8_CREATOR             = 0x01A7,     // 2 4 1
    PLAYER_VISIBLE_ITEM_8_0                   = 0x01A9,     // 12 1 1
    PLAYER_VISIBLE_ITEM_8_PROPERTIES          = 0x01B5,     // 1 2 1
    PLAYER_VISIBLE_ITEM_8_PAD                 = 0x01B6,     // 1 1 1
    PLAYER_VISIBLE_ITEM_9_CREATOR             = 0x01B7,     // 2 4 1
    PLAYER_VISIBLE_ITEM_9_0                   = 0x01B9,     // 12 1 1
    PLAYER_VISIBLE_ITEM_9_PROPERTIES          = 0x01C5,     // 1 2 1
    PLAYER_VISIBLE_ITEM_9_PAD                 = 0x01C6,     // 1 1 1
    PLAYER_VISIBLE_ITEM_10_CREATOR            = 0x01C7,     // 2 4 1
    PLAYER_VISIBLE_ITEM_10_0                  = 0x01C9,     // 12 1 1
    PLAYER_VISIBLE_ITEM_10_PROPERTIES         = 0x01D5,     // 1 2 1
    PLAYER_VISIBLE_ITEM_10_PAD                = 0x01D6,     // 1 1 1
    PLAYER_VISIBLE_ITEM_11_CREATOR            = 0x01D7,     // 2 4 1
    PLAYER_VISIBLE_ITEM_11_0                  = 0x01D9,     // 12 1 1
    PLAYER_VISIBLE_ITEM_11_PROPERTIES         = 0x01E5,     // 1 2 1
    PLAYER_VISIBLE_ITEM_11_PAD                = 0x01E6,     // 1 1 1
    PLAYER_VISIBLE_ITEM_12_CREATOR            = 0x01E7,     // 2 4 1
    PLAYER_VISIBLE_ITEM_12_0                  = 0x01E9,     // 12 1 1
    PLAYER_VISIBLE_ITEM_12_PROPERTIES         = 0x01F5,     // 1 2 1
    PLAYER_VISIBLE_ITEM_12_PAD                = 0x01F6,     // 1 1 1
    PLAYER_VISIBLE_ITEM_13_CREATOR            = 0x01F7,     // 2 4 1
    PLAYER_VISIBLE_ITEM_13_0                  = 0x01F9,     // 12 1 1
    PLAYER_VISIBLE_ITEM_13_PROPERTIES         = 0x0205,     // 1 2 1
    PLAYER_VISIBLE_ITEM_13_PAD                = 0x0206,     // 1 1 1
    PLAYER_VISIBLE_ITEM_14_CREATOR            = 0x0207,     // 2 4 1
    PLAYER_VISIBLE_ITEM_14_0                  = 0x0209,     // 12 1 1
    PLAYER_VISIBLE_ITEM_14_PROPERTIES         = 0x0215,     // 1 2 1
    PLAYER_VISIBLE_ITEM_14_PAD                = 0x0216,     // 1 1 1
    PLAYER_VISIBLE_ITEM_15_CREATOR            = 0x0217,     // 2 4 1
    PLAYER_VISIBLE_ITEM_15_0                  = 0x0219,     // 12 1 1
    PLAYER_VISIBLE_ITEM_15_PROPERTIES         = 0x0225,     // 1 2 1
    PLAYER_VISIBLE_ITEM_15_PAD                = 0x0226,     // 1 1 1
    PLAYER_VISIBLE_ITEM_16_CREATOR            = 0x0227,     // 2 4 1
    PLAYER_VISIBLE_ITEM_16_0                  = 0x0229,     // 12 1 1
    PLAYER_VISIBLE_ITEM_16_PROPERTIES         = 0x0235,     // 1 2 1
    PLAYER_VISIBLE_ITEM_16_PAD                = 0x0236,     // 1 1 1
    PLAYER_VISIBLE_ITEM_17_CREATOR            = 0x0237,     // 2 4 1
    PLAYER_VISIBLE_ITEM_17_0                  = 0x0239,     // 12 1 1
    PLAYER_VISIBLE_ITEM_17_PROPERTIES         = 0x0245,     // 1 2 1
    PLAYER_VISIBLE_ITEM_17_PAD                = 0x0246,     // 1 1 1
    PLAYER_VISIBLE_ITEM_18_CREATOR            = 0x0247,     // 2 4 1
    PLAYER_VISIBLE_ITEM_18_0                  = 0x0249,     // 12 1 1
    PLAYER_VISIBLE_ITEM_18_PROPERTIES         = 0x0255,     // 1 2 1
    PLAYER_VISIBLE_ITEM_18_PAD                = 0x0256,     // 1 1 1
    PLAYER_VISIBLE_ITEM_19_CREATOR            = 0x0257,     // 2 4 1
    PLAYER_VISIBLE_ITEM_19_0                  = 0x0259,     // 12 1 1
    PLAYER_VISIBLE_ITEM_19_PROPERTIES         = 0x0265,     // 1 2 1
    PLAYER_VISIBLE_ITEM_19_PAD                = 0x0266,     // 1 1 1
    PLAYER_CHOSEN_TITLE                       = 0x0267,     // 1 1 1
    PLAYER_FIELD_INV_SLOT_HEAD                = 0x0268,     // 46 4 2
    PLAYER_FIELD_PACK_SLOT_1                  = 0x0296,     // 32 4 2
    PLAYER_FIELD_BANK_SLOT_1                  = 0x02B6,     // 56 4 2
    PLAYER_FIELD_BANKBAG_SLOT_1               = 0x02EE,     // 14 4 2
    PLAYER_FIELD_VENDORBUYBACK_SLOT_1         = 0x02FC,     // 24 4 2
    PLAYER_FIELD_KEYRING_SLOT_1               = 0x0314,     // 64 4 2
    PLAYER_FARSIGHT                           = 0x0354,     // 2 4 2
    PLAYER__FIELD_KNOWN_TITLES                = 0x0356,     // 2 4 2
    PLAYER_XP                                 = 0x0358,     // 1 1 2
    PLAYER_NEXT_LEVEL_XP                      = 0x0359,     // 1 1 2
    PLAYER_SKILL_INFO_1_1                     = 0x035A,     // 384 2 2
    PLAYER_CHARACTER_POINTS1                  = 0x04DA,     // 1 1 2
    PLAYER_CHARACTER_POINTS2                  = 0x04DB,     // 1 1 2
    PLAYER_TRACK_CREATURES                    = 0x04DC,     // 1 1 2
    PLAYER_TRACK_RESOURCES                    = 0x04DD,     // 1 1 2
    PLAYER_BLOCK_PERCENTAGE                   = 0x04DE,     // 1 3 2
    PLAYER_DODGE_PERCENTAGE                   = 0x04DF,     // 1 3 2
    PLAYER_PARRY_PERCENTAGE                   = 0x04E0,     // 1 3 2
    PLAYER_CRIT_PERCENTAGE                    = 0x04E1,     // 1 3 2
    PLAYER_RANGED_CRIT_PERCENTAGE             = 0x04E2,     // 1 3 2
    PLAYER_OFFHAND_CRIT_PERCENTAGE            = 0x04E3,     // 1 3 2
    PLAYER_SPELL_CRIT_PERCENTAGE1             = 0x04E4,     // 7 3 2

    // custom
    PLAYER_HOLY_SPELL_CRIT_PERCENTAGE         = PLAYER_SPELL_CRIT_PERCENTAGE1+1,
    PLAYER_FIRE_SPELL_CRIT_PERCENTAGE         = PLAYER_SPELL_CRIT_PERCENTAGE1+2,
    PLAYER_NATURE_SPELL_CRIT_PERCENTAGE       = PLAYER_SPELL_CRIT_PERCENTAGE1+3,
    PLAYER_FROST_SPELL_CRIT_PERCENTAGE        = PLAYER_SPELL_CRIT_PERCENTAGE1+4,
    PLAYER_SHADOW_SPELL_CRIT_PERCENTAGE       = PLAYER_SPELL_CRIT_PERCENTAGE1+5,
    PLAYER_ARCANE_SPELL_CRIT_PERCENTAGE       = PLAYER_SPELL_CRIT_PERCENTAGE1+6,

    PLAYER_EXPLORED_ZONES_1                   = 0x04EB,     // 64 5 2
    PLAYER_REST_STATE_EXPERIENCE              = 0x052B,     // 1 1 2
    PLAYER_FIELD_COINAGE                      = 0x052C,     // 1 1 2
    PLAYER_FIELD_MOD_DAMAGE_DONE_POS          = 0x052D,     // 7 1 2
    PLAYER_FIELD_MOD_DAMAGE_DONE_NEG          = 0x0534,     // 7 1 2
    PLAYER_FIELD_MOD_DAMAGE_DONE_PCT          = 0x053B,     // 7 1 2
    PLAYER_FIELD_MOD_HEALING_DONE_POS         = 0x0542,     // 1 1 2
    PLAYER_FIELD_MOD_TARGET_RESISTANCE        = 0x0543,     // 1 1 2
    PLAYER_FIELD_BYTES                        = 0x0544,     // 1 5 2
    PLAYER_AMMO_ID                            = 0x0545,     // 1 1 2
    PLAYER_SELF_RES_SPELL                     = 0x0546,     // 1 1 2
    PLAYER_FIELD_PVP_MEDALS                   = 0x0547,     // 1 1 2
    PLAYER_FIELD_BUYBACK_PRICE_1              = 0x0548,     // 12 1 2
    PLAYER_FIELD_BUYBACK_TIMESTAMP_1          = 0x0554,     // 12 1 2
    PLAYER_FIELD_KILLS                        = 0x0560,     // 1 2 2
    PLAYER_FIELD_TODAY_CONTRIBUTION           = 0x0561,     // 1 1 2
    PLAYER_FIELD_YESTERDAY_CONTRIBUTION       = 0x0562,     // 1 1 2
    PLAYER_FIELD_LIFETIME_HONORABLE_KILLS     = 0x0563,     // 1 1 2
    PLAYER_FIELD_BYTES2                       = 0x0564,     // 1 5 2
    PLAYER_FIELD_WATCHED_FACTION_INDEX        = 0x0565,     // 1 1 2
    PLAYER_FIELD_COMBAT_RATING_1              = 0x0566,     // 23 1 2

    // custom                                                                   // client names:
                                                            // CR_WEAPON_SKILL
    PLAYER_FIELD_ALL_WEAPONS_SKILL_RATING     = PLAYER_FIELD_COMBAT_RATING_1,
                                                            // CR_DEFENSE_SKILL
    PLAYER_FIELD_DEFENCE_RATING               = PLAYER_FIELD_COMBAT_RATING_1+1,
                                                            // CR_DODGE
    PLAYER_FIELD_DODGE_RATING                 = PLAYER_FIELD_COMBAT_RATING_1+2,
                                                            // CR_PARRY
    PLAYER_FIELD_PARRY_RATING                 = PLAYER_FIELD_COMBAT_RATING_1+3,
                                                            // CR_BLOCK
    PLAYER_FIELD_BLOCK_RATING                 = PLAYER_FIELD_COMBAT_RATING_1+4,
                                                            // CR_HIT_MELEE
    PLAYER_FIELD_MELEE_HIT_RATING             = PLAYER_FIELD_COMBAT_RATING_1+5,
                                                            // CR_HIT_RANGED
    PLAYER_FIELD_RANGED_HIT_RATING            = PLAYER_FIELD_COMBAT_RATING_1+6,
                                                            // CR_HIT_SPELL
    PLAYER_FIELD_SPELL_HIT_RATING             = PLAYER_FIELD_COMBAT_RATING_1+7,
                                                            // CR_CRIT_MELEE
    PLAYER_FIELD_MELEE_CRIT_RATING            = PLAYER_FIELD_COMBAT_RATING_1+8,
                                                            // CR_CRIT_RANGED
    PLAYER_FIELD_RANGED_CRIT_RATING           = PLAYER_FIELD_COMBAT_RATING_1+9,
                                                            // CR_CRIT_SPELL
    PLAYER_FIELD_SPELL_CRIT_RATING            = PLAYER_FIELD_COMBAT_RATING_1+10,
                                                            // CR_HIT_TAKEN_MELEE
    PLAYER_FIELD_HIT_RATING                   = PLAYER_FIELD_COMBAT_RATING_1+11,
                                                            // CR_HIT_TAKEN_RANGED
    PLAYER_FIELD_CRIT_RATING                  = PLAYER_FIELD_COMBAT_RATING_1+12,
                                                            // CR_HIT_TAKEN_SPELL
    PLAYER_FIELD_UNK3_RATING                  = PLAYER_FIELD_COMBAT_RATING_1+13,
                                                            // CR_CRIT_TAKEN_MELEE
    PLAYER_FIELD_UNK4_RATING                  = PLAYER_FIELD_COMBAT_RATING_1+14,
                                                            // CR_CRIT_TAKEN_RANGED
    PLAYER_FIELD_UNK5_RATING                  = PLAYER_FIELD_COMBAT_RATING_1+15,
                                                            // CR_CRIT_TAKEN_SPELL
    PLAYER_FIELD_RESILIENCE_RATING            = PLAYER_FIELD_COMBAT_RATING_1+16,
                                                            // CR_HASTE_MELEE
    PLAYER_FIELD_MELEE_HASTE_RATING           = PLAYER_FIELD_COMBAT_RATING_1+17,
                                                            // CR_HASTE_RANGED
    PLAYER_FIELD_RANGED_HASTE_RATING          = PLAYER_FIELD_COMBAT_RATING_1+19,
                                                            // CR_HASTE_SPELL
    PLAYER_FIELD_SPELL_HASTE_RATING           = PLAYER_FIELD_COMBAT_RATING_1+20,
                                                            // CR_WEAPON_SKILL_MAINHAND
    PLAYER_FIELD_MELEE_WEAPON_SKILL_RATING    = PLAYER_FIELD_COMBAT_RATING_1+21,
                                                            // CR_WEAPON_SKILL_OFFHAND
    PLAYER_FIELD_OFFHAND_WEAPON_SKILL_RATING  = PLAYER_FIELD_COMBAT_RATING_1+22,
                                                            // CR_WEAPON_SKILL_RANGED
    PLAYER_FIELD_RANGED_WEAPON_SKILL_RATING   = PLAYER_FIELD_COMBAT_RATING_1+23,

    PLAYER_FIELD_ARENA_TEAM_INFO_1_1          = 0x057D,     // 15 1 2

    // custom
    PLAYER_FIELD_ARENA_TEAM_ID_2v2            = PLAYER_FIELD_ARENA_TEAM_INFO_1_1,
    PLAYER_FIELD_ARENA_TEAM_ID_3v3            = PLAYER_FIELD_ARENA_TEAM_INFO_1_1+5,
    PLAYER_FIELD_ARENA_TEAM_ID_5v5            = PLAYER_FIELD_ARENA_TEAM_INFO_1_1+10,

    PLAYER_FIELD_HONOR_CURRENCY               = 0x058C,     // 1 1 2
    PLAYER_FIELD_ARENA_CURRENCY               = 0x058D,     // 1 1 2
    PLAYER_FIELD_MOD_MANA_REGEN               = 0x058E,     // 1 3 2
    PLAYER_FIELD_MOD_MANA_REGEN_INTERRUPT     = 0x058F,     // 1 3 2
    PLAYER_FIELD_MAX_LEVEL                    = 0x0590,     // 1 1 2
    PLAYER_FIELD_DAILY_QUESTS_1               = 0x0591,     // 10 1 2
    PLAYER_FIELD_PADDING                      = 0x059B,     // 1 1 0
    PLAYER_END                                = 0x059C,
};

enum EGameObjectFields
{
    OBJECT_FIELD_CREATED_BY                   = 0x0006,     // 2 4 1
    GAMEOBJECT_DISPLAYID                      = 0x0008,     // 1 1 1
    GAMEOBJECT_FLAGS                          = 0x0009,     // 1 1 1
    GAMEOBJECT_ROTATION                       = 0x000A,     // 4 3 1
    GAMEOBJECT_STATE                          = 0x000E,     // 1 1 1
    GAMEOBJECT_POS_X                          = 0x000F,     // 1 3 1
    GAMEOBJECT_POS_Y                          = 0x0010,     // 1 3 1
    GAMEOBJECT_POS_Z                          = 0x0011,     // 1 3 1
    GAMEOBJECT_FACING                         = 0x0012,     // 1 3 1
    GAMEOBJECT_DYN_FLAGS                      = 0x0013,     // 1 1 256
    GAMEOBJECT_FACTION                        = 0x0014,     // 1 1 1
    GAMEOBJECT_TYPE_ID                        = 0x0015,     // 1 1 1
    GAMEOBJECT_LEVEL                          = 0x0016,     // 1 1 1
    GAMEOBJECT_ARTKIT                         = 0x0017,     // 1 1 1
    GAMEOBJECT_ANIMPROGRESS                   = 0x0018,     // 1 1 256
    GAMEOBJECT_PADDING                        = 0x0019,     // 1 1 0
    GAMEOBJECT_END                            = 0x001A,
};

enum EDynamicObjectFields
{
    DYNAMICOBJECT_CASTER                      = 0x0006,     // 2 4 1
    DYNAMICOBJECT_BYTES                       = 0x0008,     // 1 5 1
    DYNAMICOBJECT_SPELLID                     = 0x0009,     // 1 1 1
    DYNAMICOBJECT_RADIUS                      = 0x000A,     // 1 3 1
    DYNAMICOBJECT_POS_X                       = 0x000B,     // 1 3 1
    DYNAMICOBJECT_POS_Y                       = 0x000C,     // 1 3 1
    DYNAMICOBJECT_POS_Z                       = 0x000D,     // 1 3 1
    DYNAMICOBJECT_FACING                      = 0x000E,     // 1 3 1
    DYNAMICOBJECT_PAD                         = 0x000F,     // 1 5 1
    DYNAMICOBJECT_END                         = 0x0010,
};

enum ECorpseFields
{
    CORPSE_FIELD_OWNER                        = 0x0006,     // 2 4 1
    CORPSE_FIELD_FACING                       = 0x0008,     // 1 3 1
    CORPSE_FIELD_POS_X                        = 0x0009,     // 1 3 1
    CORPSE_FIELD_POS_Y                        = 0x000A,     // 1 3 1
    CORPSE_FIELD_POS_Z                        = 0x000B,     // 1 3 1
    CORPSE_FIELD_DISPLAY_ID                   = 0x000C,     // 1 1 1
    CORPSE_FIELD_ITEM                         = 0x000D,     // 19 1 1
    CORPSE_FIELD_BYTES_1                      = 0x0020,     // 1 5 1
    CORPSE_FIELD_BYTES_2                      = 0x0021,     // 1 5 1
    CORPSE_FIELD_GUILD                        = 0x0022,     // 1 1 1
    CORPSE_FIELD_FLAGS                        = 0x0023,     // 1 1 1
    CORPSE_FIELD_DYNAMIC_FLAGS                = 0x0024,     // 1 1 256
    CORPSE_FIELD_PAD                          = 0x0025,     // 1 1 0
    CORPSE_END                                = 0x0026,
};
#endif
