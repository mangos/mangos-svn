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

#pragma once

// Object
#define OBJECT_FIELD_GUID               0x000     // GUID
#define OBJECT_FIELD_GUID_01            0x001     // GUID
#define OBJECT_FIELD_TYPE                   0x002 // Int32
#define OBJECT_FIELD_ENTRY                  0x003 // Int32
#define OBJECT_FIELD_SCALE_X                0x004 // Float
#define OBJECT_FIELD_PADDING                0x005 // Int32
#define OBJECT_END                          0x006

// Item
// GUID
#define ITEM_FIELD_OWNER                OBJECT_END + 0x000
// GUID
#define ITEM_FIELD_OWNER_01             OBJECT_END + 0x001
// GUID
#define ITEM_FIELD_CONTAINED                OBJECT_END + 0x002
// GUID
#define ITEM_FIELD_CONTAINED_01             OBJECT_END + 0x003
// GUID
#define ITEM_FIELD_CREATOR              OBJECT_END + 0x004
// GUID
#define ITEM_FIELD_CREATOR_01           OBJECT_END + 0x005
// GUID
#define ITEM_FIELD_GIFTCREATOR              OBJECT_END + 0x006
// GUID
#define ITEM_FIELD_GIFTCREATOR_01           OBJECT_END + 0x007
// Int32
#define ITEM_FIELD_STACK_COUNT              OBJECT_END + 0x008
// Int32
#define ITEM_FIELD_DURATION                 OBJECT_END + 0x009
// Int32
#define ITEM_FIELD_SPELL_CHARGES                OBJECT_END + 0x00A
// Int32
#define ITEM_FIELD_SPELL_CHARGES_01             OBJECT_END + 0x00B
// Int32
#define ITEM_FIELD_SPELL_CHARGES_02             OBJECT_END + 0x00C
// Int32
#define ITEM_FIELD_SPELL_CHARGES_03             OBJECT_END + 0x00D
// Int32
#define ITEM_FIELD_SPELL_CHARGES_04             OBJECT_END + 0x00E
// Chars?
#define ITEM_FIELD_FLAGS                    OBJECT_END + 0x00F
// Int32
#define ITEM_FIELD_ENCHANTMENT              OBJECT_END + 0x010
// Int32
#define ITEM_FIELD_ENCHANTMENT_01           OBJECT_END + 0x011
// Int32
#define ITEM_FIELD_ENCHANTMENT_02           OBJECT_END + 0x012
// Int32
#define ITEM_FIELD_ENCHANTMENT_03           OBJECT_END + 0x013
// Int32
#define ITEM_FIELD_ENCHANTMENT_04           OBJECT_END + 0x014
// Int32
#define ITEM_FIELD_ENCHANTMENT_05           OBJECT_END + 0x015
// Int32
#define ITEM_FIELD_ENCHANTMENT_06           OBJECT_END + 0x016
// Int32
#define ITEM_FIELD_ENCHANTMENT_07           OBJECT_END + 0x017
// Int32
#define ITEM_FIELD_ENCHANTMENT_08           OBJECT_END + 0x018
// Int32
#define ITEM_FIELD_ENCHANTMENT_09           OBJECT_END + 0x019
// Int32
#define ITEM_FIELD_ENCHANTMENT_10           OBJECT_END + 0x01A
// Int32
#define ITEM_FIELD_ENCHANTMENT_11           OBJECT_END + 0x01B
// Int32
#define ITEM_FIELD_ENCHANTMENT_12           OBJECT_END + 0x01C
// Int32
#define ITEM_FIELD_ENCHANTMENT_13           OBJECT_END + 0x01D
// Int32
#define ITEM_FIELD_ENCHANTMENT_14           OBJECT_END + 0x01E
// Int32
#define ITEM_FIELD_ENCHANTMENT_15           OBJECT_END + 0x01F
// Int32
#define ITEM_FIELD_ENCHANTMENT_16           OBJECT_END + 0x020
// Int32
#define ITEM_FIELD_ENCHANTMENT_17           OBJECT_END + 0x021
// Int32
#define ITEM_FIELD_ENCHANTMENT_18           OBJECT_END + 0x022
// Int32
#define ITEM_FIELD_ENCHANTMENT_19           OBJECT_END + 0x023
// Int32
#define ITEM_FIELD_ENCHANTMENT_20           OBJECT_END + 0x024
// Int32
#define ITEM_FIELD_PROPERTY_SEED            OBJECT_END + 0x025
// Int32
#define ITEM_FIELD_RANDOM_PROPERTIES_ID     OBJECT_END + 0x026
// Int32
#define ITEM_FIELD_ITEM_TEXT_ID             OBJECT_END + 0x027
// Int32
#define ITEM_FIELD_DURABILITY               OBJECT_END + 0x028
// Int32
#define ITEM_FIELD_MAXDURABILITY            OBJECT_END + 0x029
#define ITEM_END                            OBJECT_END + 0x02A

// Container
// Int32
#define CONTAINER_FIELD_NUM_SLOTS           ITEM_END + 0x000
// Bytes
#define CONTAINER_ALIGN_PAD                 ITEM_END + 0x001
// GUID
#define CONTAINER_FIELD_SLOT_1              ITEM_END + 0x002
// GUID
#define CONTAINER_FIELD_SLOT_1_01           ITEM_END + 0x003
// GUID
#define CONTAINER_FIELD_SLOT_1_02           ITEM_END + 0x004
// GUID
#define CONTAINER_FIELD_SLOT_1_03           ITEM_END + 0x005
// GUID
#define CONTAINER_FIELD_SLOT_1_04           ITEM_END + 0x006
// GUID
#define CONTAINER_FIELD_SLOT_1_05           ITEM_END + 0x007
// GUID
#define CONTAINER_FIELD_SLOT_1_06           ITEM_END + 0x008
// GUID
#define CONTAINER_FIELD_SLOT_1_07           ITEM_END + 0x009
// GUID
#define CONTAINER_FIELD_SLOT_1_08           ITEM_END + 0x00A
// GUID
#define CONTAINER_FIELD_SLOT_1_09           ITEM_END + 0x00B
// GUID
#define CONTAINER_FIELD_SLOT_1_10           ITEM_END + 0x00C
// GUID
#define CONTAINER_FIELD_SLOT_1_11           ITEM_END + 0x00D
// GUID
#define CONTAINER_FIELD_SLOT_1_12           ITEM_END + 0x00E
// GUID
#define CONTAINER_FIELD_SLOT_1_13           ITEM_END + 0x00F
// GUID
#define CONTAINER_FIELD_SLOT_1_14           ITEM_END + 0x010
// GUID
#define CONTAINER_FIELD_SLOT_1_15           ITEM_END + 0x011
// GUID
#define CONTAINER_FIELD_SLOT_1_16           ITEM_END + 0x012
// GUID
#define CONTAINER_FIELD_SLOT_1_17           ITEM_END + 0x013
// GUID
#define CONTAINER_FIELD_SLOT_1_18           ITEM_END + 0x014
// GUID
#define CONTAINER_FIELD_SLOT_1_19           ITEM_END + 0x015
// GUID
#define CONTAINER_FIELD_SLOT_1_20           ITEM_END + 0x016
// GUID
#define CONTAINER_FIELD_SLOT_1_21           ITEM_END + 0x017
// GUID
#define CONTAINER_FIELD_SLOT_1_22           ITEM_END + 0x018
// GUID
#define CONTAINER_FIELD_SLOT_1_23           ITEM_END + 0x019
// GUID
#define CONTAINER_FIELD_SLOT_1_24           ITEM_END + 0x01A
// GUID
#define CONTAINER_FIELD_SLOT_1_25           ITEM_END + 0x01B
// GUID
#define CONTAINER_FIELD_SLOT_1_26           ITEM_END + 0x01C
// GUID
#define CONTAINER_FIELD_SLOT_1_27           ITEM_END + 0x01D
// GUID
#define CONTAINER_FIELD_SLOT_1_28           ITEM_END + 0x01E
// GUID
#define CONTAINER_FIELD_SLOT_1_29           ITEM_END + 0x01F
// GUID
#define CONTAINER_FIELD_SLOT_1_30           ITEM_END + 0x020
// GUID
#define CONTAINER_FIELD_SLOT_1_31           ITEM_END + 0x021
// GUID
#define CONTAINER_FIELD_SLOT_1_32           ITEM_END + 0x022
// GUID
#define CONTAINER_FIELD_SLOT_1_33           ITEM_END + 0x023
// GUID
#define CONTAINER_FIELD_SLOT_1_34           ITEM_END + 0x024
// GUID
#define CONTAINER_FIELD_SLOT_1_35           ITEM_END + 0x025
// GUID
#define CONTAINER_FIELD_SLOT_1_36           ITEM_END + 0x026
// GUID
#define CONTAINER_FIELD_SLOT_1_37           ITEM_END + 0x027
// GUID
#define CONTAINER_FIELD_SLOT_1_38           ITEM_END + 0x028
// GUID
#define CONTAINER_FIELD_SLOT_1_39           ITEM_END + 0x029
#define CONTAINER_END                       ITEM_END + 0x02A

// Unit
// GUID
#define UNIT_FIELD_CHARM                OBJECT_END + 0x000
// GUID
#define UNIT_FIELD_CHARM_01             OBJECT_END + 0x001
// GUID
#define UNIT_FIELD_SUMMON               OBJECT_END + 0x002
// GUID
#define UNIT_FIELD_SUMMON_01            OBJECT_END + 0x003
// GUID
#define UNIT_FIELD_CHARMEDBY            OBJECT_END + 0x004
// GUID
#define UNIT_FIELD_CHARMEDBY_01             OBJECT_END + 0x005
// GUID
#define UNIT_FIELD_SUMMONEDBY               OBJECT_END + 0x006
// GUID
#define UNIT_FIELD_SUMMONEDBY_01            OBJECT_END + 0x007
// GUID
#define UNIT_FIELD_CREATEDBY                OBJECT_END + 0x008
// GUID
#define UNIT_FIELD_CREATEDBY_01             OBJECT_END + 0x009
// GUID
#define UNIT_FIELD_TARGET               OBJECT_END + 0x00A
// GUID
#define UNIT_FIELD_TARGET_01            OBJECT_END + 0x00B
// GUID
#define UNIT_FIELD_PERSUADED                OBJECT_END + 0x00C
// GUID
#define UNIT_FIELD_PERSUADED_01             OBJECT_END + 0x00D
// GUID
#define UNIT_FIELD_CHANNEL_OBJECT               OBJECT_END + 0x00E
// GUID
#define UNIT_FIELD_CHANNEL_OBJECT_01            OBJECT_END + 0x00F
// Int32
#define UNIT_FIELD_HEALTH                   OBJECT_END + 0x010
// Int32
#define UNIT_FIELD_POWER1                   OBJECT_END + 0x011
// Int32
#define UNIT_FIELD_POWER2                   OBJECT_END + 0x012
// Int32
#define UNIT_FIELD_POWER3                   OBJECT_END + 0x013
// Int32
#define UNIT_FIELD_POWER4                   OBJECT_END + 0x014
// Int32
#define UNIT_FIELD_POWER5                   OBJECT_END + 0x015
// Int32
#define UNIT_FIELD_MAXHEALTH                OBJECT_END + 0x016
// Int32
#define UNIT_FIELD_MAXPOWER1                OBJECT_END + 0x017
// Int32
#define UNIT_FIELD_MAXPOWER2                OBJECT_END + 0x018
// Int32
#define UNIT_FIELD_MAXPOWER3                OBJECT_END + 0x019
// Int32
#define UNIT_FIELD_MAXPOWER4                OBJECT_END + 0x01A
// Int32
#define UNIT_FIELD_MAXPOWER5                OBJECT_END + 0x01B
// Int32
#define UNIT_FIELD_LEVEL                    OBJECT_END + 0x01C
// Int32
#define UNIT_FIELD_FACTIONTEMPLATE          OBJECT_END + 0x01D
// Bytes
#define UNIT_FIELD_BYTES_0                  OBJECT_END + 0x01E
// Int32
#define UNIT_VIRTUAL_ITEM_SLOT_DISPLAY              OBJECT_END + 0x01F
// Int32
#define UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_01           OBJECT_END + 0x020
// Int32
#define UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_02           OBJECT_END + 0x021
// Bytes
#define UNIT_VIRTUAL_ITEM_INFO              OBJECT_END + 0x022
// Bytes
#define UNIT_VIRTUAL_ITEM_INFO_01           OBJECT_END + 0x023
// Bytes
#define UNIT_VIRTUAL_ITEM_INFO_02           OBJECT_END + 0x024
// Bytes
#define UNIT_VIRTUAL_ITEM_INFO_03           OBJECT_END + 0x025
// Bytes
#define UNIT_VIRTUAL_ITEM_INFO_04           OBJECT_END + 0x026
// Bytes
#define UNIT_VIRTUAL_ITEM_INFO_05           OBJECT_END + 0x027
// Int32
#define UNIT_FIELD_FLAGS                    OBJECT_END + 0x028
// Int32
#define UNIT_FIELD_AURA             OBJECT_END + 0x029
// Int32
#define UNIT_FIELD_AURA_01          OBJECT_END + 0x02A
// Int32
#define UNIT_FIELD_AURA_02          OBJECT_END + 0x02B
// Int32
#define UNIT_FIELD_AURA_03          OBJECT_END + 0x02C
// Int32
#define UNIT_FIELD_AURA_04          OBJECT_END + 0x02D
// Int32
#define UNIT_FIELD_AURA_05          OBJECT_END + 0x02E
// Int32
#define UNIT_FIELD_AURA_06          OBJECT_END + 0x02F
// Int32
#define UNIT_FIELD_AURA_07          OBJECT_END + 0x030
// Int32
#define UNIT_FIELD_AURA_08          OBJECT_END + 0x031
// Int32
#define UNIT_FIELD_AURA_09          OBJECT_END + 0x032
// Int32
#define UNIT_FIELD_AURA_10          OBJECT_END + 0x033
// Int32
#define UNIT_FIELD_AURA_11          OBJECT_END + 0x034
// Int32
#define UNIT_FIELD_AURA_12          OBJECT_END + 0x035
// Int32
#define UNIT_FIELD_AURA_13          OBJECT_END + 0x036
// Int32
#define UNIT_FIELD_AURA_14          OBJECT_END + 0x037
// Int32
#define UNIT_FIELD_AURA_15          OBJECT_END + 0x038
// Int32
#define UNIT_FIELD_AURA_16          OBJECT_END + 0x039
// Int32
#define UNIT_FIELD_AURA_17          OBJECT_END + 0x03A
// Int32
#define UNIT_FIELD_AURA_18          OBJECT_END + 0x03B
// Int32
#define UNIT_FIELD_AURA_19          OBJECT_END + 0x03C
// Int32
#define UNIT_FIELD_AURA_20          OBJECT_END + 0x03D
// Int32
#define UNIT_FIELD_AURA_21          OBJECT_END + 0x03E
// Int32
#define UNIT_FIELD_AURA_22          OBJECT_END + 0x03F
// Int32
#define UNIT_FIELD_AURA_23          OBJECT_END + 0x040
// Int32
#define UNIT_FIELD_AURA_24          OBJECT_END + 0x041
// Int32
#define UNIT_FIELD_AURA_25          OBJECT_END + 0x042
// Int32
#define UNIT_FIELD_AURA_26          OBJECT_END + 0x043
// Int32
#define UNIT_FIELD_AURA_27          OBJECT_END + 0x044
// Int32
#define UNIT_FIELD_AURA_28          OBJECT_END + 0x045
// Int32
#define UNIT_FIELD_AURA_29          OBJECT_END + 0x046
// Int32
#define UNIT_FIELD_AURA_30          OBJECT_END + 0x047
// Int32
#define UNIT_FIELD_AURA_31          OBJECT_END + 0x048
// Int32
#define UNIT_FIELD_AURA_32          OBJECT_END + 0x049
// Int32
#define UNIT_FIELD_AURA_33          OBJECT_END + 0x04A
// Int32
#define UNIT_FIELD_AURA_34          OBJECT_END + 0x04B
// Int32
#define UNIT_FIELD_AURA_35          OBJECT_END + 0x04C
// Int32
#define UNIT_FIELD_AURA_36          OBJECT_END + 0x04D
// Int32
#define UNIT_FIELD_AURA_37          OBJECT_END + 0x04E
// Int32
#define UNIT_FIELD_AURA_38          OBJECT_END + 0x04F
// Int32
#define UNIT_FIELD_AURA_39          OBJECT_END + 0x050
// Int32
#define UNIT_FIELD_AURA_40          OBJECT_END + 0x051
// Int32
#define UNIT_FIELD_AURA_41          OBJECT_END + 0x052
// Int32
#define UNIT_FIELD_AURA_42          OBJECT_END + 0x053
// Int32
#define UNIT_FIELD_AURA_43          OBJECT_END + 0x054
// Int32
#define UNIT_FIELD_AURA_44          OBJECT_END + 0x055
// Int32
#define UNIT_FIELD_AURA_45          OBJECT_END + 0x056
// Int32
#define UNIT_FIELD_AURA_46          OBJECT_END + 0x057
// Int32
#define UNIT_FIELD_AURA_47          OBJECT_END + 0x058
// Int32
#define UNIT_FIELD_AURA_48          OBJECT_END + 0x059
// Int32
#define UNIT_FIELD_AURA_49          OBJECT_END + 0x05A
// Int32
#define UNIT_FIELD_AURA_50          OBJECT_END + 0x05B
// Int32
#define UNIT_FIELD_AURA_51          OBJECT_END + 0x05C
// Int32
#define UNIT_FIELD_AURA_52          OBJECT_END + 0x05D
// Int32
#define UNIT_FIELD_AURA_53          OBJECT_END + 0x05E
// Int32
#define UNIT_FIELD_AURA_54          OBJECT_END + 0x05F
// Int32
#define UNIT_FIELD_AURA_55          OBJECT_END + 0x060
// Bytes
#define UNIT_FIELD_AURALEVELS               OBJECT_END + 0x061
// Bytes
#define UNIT_FIELD_AURALEVELS_01            OBJECT_END + 0x062
// Bytes
#define UNIT_FIELD_AURALEVELS_02            OBJECT_END + 0x063
// Bytes
#define UNIT_FIELD_AURALEVELS_03            OBJECT_END + 0x064
// Bytes
#define UNIT_FIELD_AURALEVELS_04            OBJECT_END + 0x065
// Bytes
#define UNIT_FIELD_AURALEVELS_05            OBJECT_END + 0x066
// Bytes
#define UNIT_FIELD_AURALEVELS_06            OBJECT_END + 0x067
// Bytes
#define UNIT_FIELD_AURALEVELS_07            OBJECT_END + 0x068
// Bytes
#define UNIT_FIELD_AURALEVELS_08            OBJECT_END + 0x069
// Bytes
#define UNIT_FIELD_AURALEVELS_09            OBJECT_END + 0x06A
// Bytes
#define UNIT_FIELD_AURAAPPLICATIONS             OBJECT_END + 0x06B
// Bytes
#define UNIT_FIELD_AURAAPPLICATIONS_01          OBJECT_END + 0x06C
// Bytes
#define UNIT_FIELD_AURAAPPLICATIONS_02          OBJECT_END + 0x06D
// Bytes
#define UNIT_FIELD_AURAAPPLICATIONS_03          OBJECT_END + 0x06E
// Bytes
#define UNIT_FIELD_AURAAPPLICATIONS_04          OBJECT_END + 0x06F
// Bytes
#define UNIT_FIELD_AURAAPPLICATIONS_05          OBJECT_END + 0x070
// Bytes
#define UNIT_FIELD_AURAAPPLICATIONS_06          OBJECT_END + 0x071
// Bytes
#define UNIT_FIELD_AURAAPPLICATIONS_07          OBJECT_END + 0x072
// Bytes
#define UNIT_FIELD_AURAAPPLICATIONS_08          OBJECT_END + 0x073
// Bytes
#define UNIT_FIELD_AURAAPPLICATIONS_09          OBJECT_END + 0x074
// Bytes
#define UNIT_FIELD_AURAFLAGS                OBJECT_END + 0x075
// Bytes
#define UNIT_FIELD_AURAFLAGS_01             OBJECT_END + 0x076
// Bytes
#define UNIT_FIELD_AURAFLAGS_02             OBJECT_END + 0x077
// Bytes
#define UNIT_FIELD_AURAFLAGS_03             OBJECT_END + 0x078
// Bytes
#define UNIT_FIELD_AURAFLAGS_04             OBJECT_END + 0x079
// Bytes
#define UNIT_FIELD_AURAFLAGS_05             OBJECT_END + 0x07A
// Bytes
#define UNIT_FIELD_AURAFLAGS_06             OBJECT_END + 0x07B
// Int32
#define UNIT_FIELD_AURASTATE               OBJECT_END +  0x07C
// Int32
#define UNIT_FIELD_BASEATTACKTIME               OBJECT_END + 0x07D
// Int32
#define UNIT_FIELD_BASEATTACKTIME_01            OBJECT_END + 0x07E
// Int32
#define UNIT_FIELD_RANGEDATTACKTIME         OBJECT_END + 0x07F
// Float
#define UNIT_FIELD_BOUNDINGRADIUS          OBJECT_END +  0x080
// Float
#define UNIT_FIELD_COMBATREACH              OBJECT_END + 0x081
// Int32
#define UNIT_FIELD_DISPLAYID                OBJECT_END + 0x082
// Int32
#define UNIT_FIELD_NATIVEDISPLAYID          OBJECT_END + 0x083
// Int32
#define UNIT_FIELD_MOUNTDISPLAYID           OBJECT_END + 0x084
// Float
#define UNIT_FIELD_MINDAMAGE                OBJECT_END + 0x085
// Float
#define UNIT_FIELD_MAXDAMAGE                OBJECT_END + 0x086
// Float
#define UNIT_FIELD_MINOFFHANDDAMAGE         OBJECT_END + 0x087
// Float
#define UNIT_FIELD_MAXOFFHANDDAMAGE         OBJECT_END + 0x088
// Bytes
#define UNIT_FIELD_BYTES_1                  OBJECT_END + 0x089
// Int32
#define UNIT_FIELD_PETNUMBER                OBJECT_END + 0x08A
// Int32
#define UNIT_FIELD_PET_NAME_TIMESTAMP       OBJECT_END + 0x08B
// Int32
#define UNIT_FIELD_PETEXPERIENCE            OBJECT_END + 0x08C
// Int32
#define UNIT_FIELD_PETNEXTLEVELEXP          OBJECT_END + 0x08D
// Int32
#define UNIT_DYNAMIC_FLAGS                  OBJECT_END + 0x08E
// Int32
#define UNIT_CHANNEL_SPELL                  OBJECT_END + 0x08F
// Int32
#define UNIT_MOD_CAST_SPEED                 OBJECT_END + 0x090
// Int32
#define UNIT_CREATED_BY_SPELL               OBJECT_END + 0x091
// Int32
#define UNIT_NPC_FLAGS                      OBJECT_END + 0x092
// Int32
#define UNIT_NPC_EMOTESTATE                 OBJECT_END + 0x093
// Chars?
#define UNIT_TRAINING_POINTS                OBJECT_END + 0x094
// Int32
#define UNIT_FIELD_STAT0                    OBJECT_END + 0x095
// Int32
#define UNIT_FIELD_STAT1                    OBJECT_END + 0x096
// Int32
#define UNIT_FIELD_STAT2                    OBJECT_END + 0x097
// Int32
#define UNIT_FIELD_STAT3                    OBJECT_END + 0x098
// Int32
#define UNIT_FIELD_STAT4                    OBJECT_END + 0x099
// Int32
#define UNIT_FIELD_RESISTANCES              OBJECT_END + 0x09A
// Int32
#define UNIT_FIELD_RESISTANCES_01           OBJECT_END + 0x09B
// Int32
#define UNIT_FIELD_RESISTANCES_02           OBJECT_END + 0x09C
// Int32
#define UNIT_FIELD_RESISTANCES_03           OBJECT_END + 0x09D
// Int32
#define UNIT_FIELD_RESISTANCES_04           OBJECT_END + 0x09E
// Int32
#define UNIT_FIELD_RESISTANCES_05           OBJECT_END + 0x09F
// Int32
#define UNIT_FIELD_RESISTANCES_06           OBJECT_END + 0x0A0
// Int32
#define UNIT_FIELD_ATTACKPOWER              OBJECT_END + 0x0A1
// Int32
#define UNIT_FIELD_BASE_MANA                OBJECT_END + 0x0A2
// Chars?
#define UNIT_FIELD_ATTACK_POWER_MODS        OBJECT_END + 0x0A3
// Bytes
#define UNIT_FIELD_BYTES_2                  OBJECT_END + 0x0A4
// Int32
#define UNIT_FIELD_RANGEDATTACKPOWER        OBJECT_END + 0x0A5
// Chars?
#define UNIT_FIELD_RANGED_ATTACK_POWER_MODS OBJECT_END + 0x0A6
// Float
#define UNIT_FIELD_MINRANGEDDAMAGE          OBJECT_END + 0x0A7
// Float
#define UNIT_FIELD_MAXRANGEDDAMAGE          OBJECT_END + 0x0A8
// Int32
#define UNIT_FIELD_PADDING                  OBJECT_END + 0x0A9
#define UNIT_END                            OBJECT_END + 0x0AA

// Player
// GUID
#define PLAYER_SELECTION                UNIT_END + 0x000
// GUID
#define PLAYER_SELECTION_01             UNIT_END + 0x001
// GUID
#define PLAYER_DUEL_ARBITER             UNIT_END + 0x002
// GUID
#define PLAYER_DUEL_ARBITER_01          UNIT_END + 0x003
// Int32
#define PLAYER_FLAGS                        UNIT_END + 0x004
// Int32
#define PLAYER_GUILDID                      UNIT_END + 0x005
// Int32
#define PLAYER_GUILDRANK                    UNIT_END + 0x006
// Bytes
#define PLAYER_BYTES                        UNIT_END + 0x007
// Bytes
#define PLAYER_BYTES_2                      UNIT_END + 0x008
// Bytes
#define PLAYER_BYTES_3                      UNIT_END + 0x009
// Int32
#define PLAYER_DUEL_TEAM                    UNIT_END + 0x00A
// Int32
#define PLAYER_GUILD_TIMESTAMP              UNIT_END + 0x00B
// Int32
#define PLAYER_QUEST_LOG_1_1                UNIT_END + 0x00C
// Int32
#define PLAYER_QUEST_LOG_1_2                UNIT_END + 0x00D
// Int32
#define PLAYER_QUEST_LOG_1_2_01             UNIT_END + 0x00E
// Int32
#define PLAYER_QUEST_LOG_2_1                UNIT_END + 0x00F
// Int32
#define PLAYER_QUEST_LOG_2_2                UNIT_END + 0x010
// Int32
#define PLAYER_QUEST_LOG_2_2_01             UNIT_END + 0x011
// Int32
#define PLAYER_QUEST_LOG_3_1                UNIT_END + 0x012
// Int32
#define PLAYER_QUEST_LOG_3_2                UNIT_END + 0x013
// Int32
#define PLAYER_QUEST_LOG_3_2_01             UNIT_END + 0x014
// Int32
#define PLAYER_QUEST_LOG_4_1                UNIT_END + 0x015
// Int32
#define PLAYER_QUEST_LOG_4_2                UNIT_END + 0x016
// Int32
#define PLAYER_QUEST_LOG_4_2_01             UNIT_END + 0x017
// Int32
#define PLAYER_QUEST_LOG_5_1               UNIT_END +  0x018
// Int32
#define PLAYER_QUEST_LOG_5_2                UNIT_END + 0x019
// Int32
#define PLAYER_QUEST_LOG_5_2_01             UNIT_END + 0x01A
// Int32
#define PLAYER_QUEST_LOG_6_1                UNIT_END + 0x01B
// Int32
#define PLAYER_QUEST_LOG_6_2                UNIT_END + 0x01C
// Int32
#define PLAYER_QUEST_LOG_6_2_01             UNIT_END + 0x01D
// Int32
#define PLAYER_QUEST_LOG_7_1                UNIT_END + 0x01E
// Int32
#define PLAYER_QUEST_LOG_7_2                UNIT_END + 0x01F
// Int32
#define PLAYER_QUEST_LOG_7_2_01             UNIT_END + 0x020
// Int32
#define PLAYER_QUEST_LOG_8_1                UNIT_END + 0x021
// Int32
#define PLAYER_QUEST_LOG_8_2                UNIT_END + 0x022
// Int32
#define PLAYER_QUEST_LOG_8_2_01             UNIT_END + 0x023
// Int32
#define PLAYER_QUEST_LOG_9_1                UNIT_END + 0x024
// Int32
#define PLAYER_QUEST_LOG_9_2                UNIT_END + 0x025
// Int32
#define PLAYER_QUEST_LOG_9_2_01             UNIT_END + 0x026
// Int32
#define PLAYER_QUEST_LOG_10_1               UNIT_END + 0x027
// Int32
#define PLAYER_QUEST_LOG_10_2               UNIT_END + 0x028
// Int32
#define PLAYER_QUEST_LOG_10_2_01            UNIT_END + 0x029
// Int32
#define PLAYER_QUEST_LOG_11_1               UNIT_END + 0x02A
// Int32
#define PLAYER_QUEST_LOG_11_2               UNIT_END + 0x02B
// Int32
#define PLAYER_QUEST_LOG_11_2_01            UNIT_END + 0x02C
// Int32
#define PLAYER_QUEST_LOG_12_1               UNIT_END + 0x02D
// Int32
#define PLAYER_QUEST_LOG_12_2               UNIT_END + 0x02E
// Int32
#define PLAYER_QUEST_LOG_12_2_01            UNIT_END + 0x02F
// Int32
#define PLAYER_QUEST_LOG_13_1               UNIT_END + 0x030
// Int32
#define PLAYER_QUEST_LOG_13_2               UNIT_END + 0x031
// Int32
#define PLAYER_QUEST_LOG_13_2_01            UNIT_END + 0x032
// Int32
#define PLAYER_QUEST_LOG_14_1               UNIT_END + 0x033
// Int32
#define PLAYER_QUEST_LOG_14_2               UNIT_END + 0x034
// Int32
#define PLAYER_QUEST_LOG_14_2_01            UNIT_END + 0x035
// Int32
#define PLAYER_QUEST_LOG_15_1              UNIT_END + 0x036
// Int32
#define PLAYER_QUEST_LOG_15_2               UNIT_END + 0x037
// Int32
#define PLAYER_QUEST_LOG_15_2_01            UNIT_END + 0x038
// Int32
#define PLAYER_QUEST_LOG_16_1               UNIT_END + 0x039
// Int32
#define PLAYER_QUEST_LOG_16_2               UNIT_END + 0x03A
// Int32
#define PLAYER_QUEST_LOG_16_2_01            UNIT_END + 0x03B
// Int32
#define PLAYER_QUEST_LOG_17_1               UNIT_END + 0x03C
// Int32
#define PLAYER_QUEST_LOG_17_2               UNIT_END + 0x03D
// Int32
#define PLAYER_QUEST_LOG_17_2_01            UNIT_END + 0x03E
// Int32
#define PLAYER_QUEST_LOG_18_1               UNIT_END + 0x03F
// Int32
#define PLAYER_QUEST_LOG_18_2               UNIT_END + 0x040
// Int32
#define PLAYER_QUEST_LOG_18_2_01            UNIT_END + 0x041
// Int32
#define PLAYER_QUEST_LOG_19_1               UNIT_END + 0x042
// Int32
#define PLAYER_QUEST_LOG_19_2               UNIT_END + 0x043
// Int32
#define PLAYER_QUEST_LOG_19_2_01            UNIT_END + 0x044
// Int32
#define PLAYER_QUEST_LOG_20_1               UNIT_END + 0x045
// Int32
#define PLAYER_QUEST_LOG_20_2               UNIT_END + 0x046
// Int32
#define PLAYER_QUEST_LOG_20_2_01            UNIT_END + 0x047
// GUID
#define PLAYER_VISIBLE_ITEM_1_CREATOR               UNIT_END + 0x048
// GUID
#define PLAYER_VISIBLE_ITEM_1_CREATOR_01            UNIT_END + 0x049
// Int32
#define PLAYER_VISIBLE_ITEM_1_0             UNIT_END + 0x04A
// Int32
#define PLAYER_VISIBLE_ITEM_1_0_01          UNIT_END + 0x04B
// Int32
#define PLAYER_VISIBLE_ITEM_1_0_02          UNIT_END + 0x04C
// Int32
#define PLAYER_VISIBLE_ITEM_1_0_03          UNIT_END + 0x04D
// Int32
#define PLAYER_VISIBLE_ITEM_1_0_04          UNIT_END + 0x04E
// Int32
#define PLAYER_VISIBLE_ITEM_1_0_05          UNIT_END + 0x04F
// Int32
#define PLAYER_VISIBLE_ITEM_1_0_06          UNIT_END + 0x050
// Int32
#define PLAYER_VISIBLE_ITEM_1_0_07          UNIT_END + 0x051
// Chars?
#define PLAYER_VISIBLE_ITEM_1_PROPERTIES    UNIT_END + 0x052
// Int32
#define PLAYER_VISIBLE_ITEM_1_PAD          UNIT_END +  0x053
// GUID
#define PLAYER_VISIBLE_ITEM_2_CREATOR               UNIT_END + 0x054
// GUID
#define PLAYER_VISIBLE_ITEM_2_CREATOR_01            UNIT_END + 0x055
// Int32
#define PLAYER_VISIBLE_ITEM_2_0             UNIT_END + 0x056
// Int32
#define PLAYER_VISIBLE_ITEM_2_0_01          UNIT_END + 0x057
// Int32
#define PLAYER_VISIBLE_ITEM_2_0_02          UNIT_END + 0x058
// Int32
#define PLAYER_VISIBLE_ITEM_2_0_03          UNIT_END + 0x059
// Int32
#define PLAYER_VISIBLE_ITEM_2_0_04          UNIT_END + 0x05A
// Int32
#define PLAYER_VISIBLE_ITEM_2_0_05          UNIT_END + 0x05B
// Int32
#define PLAYER_VISIBLE_ITEM_2_0_06          UNIT_END + 0x05C
// Int32
#define PLAYER_VISIBLE_ITEM_2_0_07          UNIT_END + 0x05D
// Chars?
#define PLAYER_VISIBLE_ITEM_2_PROPERTIES    UNIT_END + 0x05E
// Int32
#define PLAYER_VISIBLE_ITEM_2_PAD           UNIT_END + 0x05F
// GUID
#define PLAYER_VISIBLE_ITEM_3_CREATOR               UNIT_END + 0x060
// GUID
#define PLAYER_VISIBLE_ITEM_3_CREATOR_01            UNIT_END + 0x061
// Int32
#define PLAYER_VISIBLE_ITEM_3_0             UNIT_END + 0x062
// Int32
#define PLAYER_VISIBLE_ITEM_3_0_01          UNIT_END + 0x063
// Int32
#define PLAYER_VISIBLE_ITEM_3_0_02          UNIT_END + 0x064
// Int32
#define PLAYER_VISIBLE_ITEM_3_0_03          UNIT_END + 0x065
// Int32
#define PLAYER_VISIBLE_ITEM_3_0_04          UNIT_END + 0x066
// Int32
#define PLAYER_VISIBLE_ITEM_3_0_05          UNIT_END + 0x067
// Int32
#define PLAYER_VISIBLE_ITEM_3_0_06          UNIT_END + 0x068
// Int32
#define PLAYER_VISIBLE_ITEM_3_0_07          UNIT_END + 0x069
// Chars?
#define PLAYER_VISIBLE_ITEM_3_PROPERTIES    UNIT_END + 0x06A
// Int32
#define PLAYER_VISIBLE_ITEM_3_PAD           UNIT_END + 0x06B
// GUID
#define PLAYER_VISIBLE_ITEM_4_CREATOR               UNIT_END + 0x06C
// GUID
#define PLAYER_VISIBLE_ITEM_4_CREATOR_01            UNIT_END + 0x06D
// Int32
#define PLAYER_VISIBLE_ITEM_4_0             UNIT_END + 0x06E
// Int32
#define PLAYER_VISIBLE_ITEM_4_0_01          UNIT_END + 0x06F
// Int32
#define PLAYER_VISIBLE_ITEM_4_0_02          UNIT_END + 0x070
// Int32
#define PLAYER_VISIBLE_ITEM_4_0_03          UNIT_END + 0x071
// Int32
#define PLAYER_VISIBLE_ITEM_4_0_04          UNIT_END + 0x072
// Int32
#define PLAYER_VISIBLE_ITEM_4_0_05          UNIT_END + 0x073
// Int32
#define PLAYER_VISIBLE_ITEM_4_0_06          UNIT_END + 0x074
// Int32
#define PLAYER_VISIBLE_ITEM_4_0_07          UNIT_END + 0x075
// Chars?
#define PLAYER_VISIBLE_ITEM_4_PROPERTIES   UNIT_END +  0x076
// Int32
#define PLAYER_VISIBLE_ITEM_4_PAD           UNIT_END + 0x077
// GUID
#define PLAYER_VISIBLE_ITEM_5_CREATOR               UNIT_END + 0x078
// GUID
#define PLAYER_VISIBLE_ITEM_5_CREATOR_01            0x079
// Int32
#define PLAYER_VISIBLE_ITEM_5_0             UNIT_END + 0x07A
// Int32
#define PLAYER_VISIBLE_ITEM_5_0_01          UNIT_END + 0x07B
// Int32
#define PLAYER_VISIBLE_ITEM_5_0_02          UNIT_END + 0x07C
// Int32
#define PLAYER_VISIBLE_ITEM_5_0_03          UNIT_END + 0x07D
// Int32
#define PLAYER_VISIBLE_ITEM_5_0_04          UNIT_END + 0x07E
// Int32
#define PLAYER_VISIBLE_ITEM_5_0_05          UNIT_END + 0x07F
// Int32
#define PLAYER_VISIBLE_ITEM_5_0_06          UNIT_END + 0x080
// Int32
#define PLAYER_VISIBLE_ITEM_5_0_07          UNIT_END + 0x081
// Chars?
#define PLAYER_VISIBLE_ITEM_5_PROPERTIES    UNIT_END + 0x082
// Int32
#define PLAYER_VISIBLE_ITEM_5_PAD           UNIT_END + 0x083
// GUID
#define PLAYER_VISIBLE_ITEM_6_CREATOR               UNIT_END + 0x084
// GUID
#define PLAYER_VISIBLE_ITEM_6_CREATOR_01            UNIT_END + 0x085
// Int32
#define PLAYER_VISIBLE_ITEM_6_0             UNIT_END + 0x086
// Int32
#define PLAYER_VISIBLE_ITEM_6_0_01          UNIT_END + 0x087
// Int32
#define PLAYER_VISIBLE_ITEM_6_0_02          UNIT_END + 0x088
// Int32
#define PLAYER_VISIBLE_ITEM_6_0_03          UNIT_END + 0x089
// Int32
#define PLAYER_VISIBLE_ITEM_6_0_04          UNIT_END + 0x08A
// Int32
#define PLAYER_VISIBLE_ITEM_6_0_05          UNIT_END + 0x08B
// Int32
#define PLAYER_VISIBLE_ITEM_6_0_06          UNIT_END + 0x08C
// Int32
#define PLAYER_VISIBLE_ITEM_6_0_07          UNIT_END + 0x08D
// Chars?
#define PLAYER_VISIBLE_ITEM_6_PROPERTIES    UNIT_END + 0x08E
// Int32
#define PLAYER_VISIBLE_ITEM_6_PAD           UNIT_END + 0x08F
// GUID
#define PLAYER_VISIBLE_ITEM_7_CREATOR               UNIT_END + 0x090
// GUID
#define PLAYER_VISIBLE_ITEM_7_CREATOR_01            UNIT_END + 0x091
// Int32
#define PLAYER_VISIBLE_ITEM_7_0             UNIT_END + 0x092
// Int32
#define PLAYER_VISIBLE_ITEM_7_0_01          UNIT_END + 0x093
// Int32
#define PLAYER_VISIBLE_ITEM_7_0_02          UNIT_END + 0x094
// Int32
#define PLAYER_VISIBLE_ITEM_7_0_03          UNIT_END + 0x095
// Int32
#define PLAYER_VISIBLE_ITEM_7_0_04          UNIT_END + 0x096
// Int32
#define PLAYER_VISIBLE_ITEM_7_0_05          UNIT_END + 0x097
// Int32
#define PLAYER_VISIBLE_ITEM_7_0_06          UNIT_END + 0x098
// Int32
#define PLAYER_VISIBLE_ITEM_7_0_07          UNIT_END + 0x099
// Chars?
#define PLAYER_VISIBLE_ITEM_7_PROPERTIES    UNIT_END + 0x09A
// Int32
#define PLAYER_VISIBLE_ITEM_7_PAD           UNIT_END + 0x09B
// GUID
#define PLAYER_VISIBLE_ITEM_8_CREATOR               UNIT_END + 0x09C
// GUID
#define PLAYER_VISIBLE_ITEM_8_CREATOR_01            UNIT_END + 0x09D
// Int32
#define PLAYER_VISIBLE_ITEM_8_0             UNIT_END + 0x09E
// Int32
#define PLAYER_VISIBLE_ITEM_8_0_01          UNIT_END + 0x09F
// Int32
#define PLAYER_VISIBLE_ITEM_8_0_02          UNIT_END + 0x0A0
// Int32
#define PLAYER_VISIBLE_ITEM_8_0_03          UNIT_END + 0x0A1
// Int32
#define PLAYER_VISIBLE_ITEM_8_0_04          UNIT_END + 0x0A2
// Int32
#define PLAYER_VISIBLE_ITEM_8_0_05          UNIT_END + 0x0A3
// Int32
#define PLAYER_VISIBLE_ITEM_8_0_06          UNIT_END + 0x0A4
// Int32
#define PLAYER_VISIBLE_ITEM_8_0_07          UNIT_END + 0x0A5
// Chars?
#define PLAYER_VISIBLE_ITEM_8_PROPERTIES    UNIT_END + 0x0A6
// Int32
#define PLAYER_VISIBLE_ITEM_8_PAD           UNIT_END + 0x0A7
// GUID
#define PLAYER_VISIBLE_ITEM_9_CREATOR               UNIT_END + 0x0A8
// GUID
#define PLAYER_VISIBLE_ITEM_9_CREATOR_01            UNIT_END + 0x0A9
// Int32
#define PLAYER_VISIBLE_ITEM_9_0             UNIT_END + 0x0AA
// Int32
#define PLAYER_VISIBLE_ITEM_9_0_01          UNIT_END + 0x0AB
// Int32
#define PLAYER_VISIBLE_ITEM_9_0_02          UNIT_END + 0x0AC
// Int32
#define PLAYER_VISIBLE_ITEM_9_0_03          UNIT_END + 0x0AD
// Int32
#define PLAYER_VISIBLE_ITEM_9_0_04          UNIT_END + 0x0AE
// Int32
#define PLAYER_VISIBLE_ITEM_9_0_05          UNIT_END + 0x0AF
// Int32
#define PLAYER_VISIBLE_ITEM_9_0_06          UNIT_END + 0x0B0
// Int32
#define PLAYER_VISIBLE_ITEM_9_0_07          UNIT_END + 0x0B1
// Chars?
#define PLAYER_VISIBLE_ITEM_9_PROPERTIES    UNIT_END + 0x0B2
// Int32
#define PLAYER_VISIBLE_ITEM_9_PAD           UNIT_END + 0x0B3
// GUID
#define PLAYER_VISIBLE_ITEM_10_CREATOR              UNIT_END + 0x0B4
// GUID
#define PLAYER_VISIBLE_ITEM_10_CREATOR_01           UNIT_END + 0x0B5
// Int32
#define PLAYER_VISIBLE_ITEM_10_0                UNIT_END + 0x0B6
// Int32
#define PLAYER_VISIBLE_ITEM_10_0_01             UNIT_END + 0x0B7
// Int32
#define PLAYER_VISIBLE_ITEM_10_0_02             UNIT_END + 0x0B8
// Int32
#define PLAYER_VISIBLE_ITEM_10_0_03             UNIT_END + 0x0B9
// Int32
#define PLAYER_VISIBLE_ITEM_10_0_04             UNIT_END + 0x0BA
// Int32
#define PLAYER_VISIBLE_ITEM_10_0_05             UNIT_END + 0x0BB
// Int32
#define PLAYER_VISIBLE_ITEM_10_0_06             UNIT_END + 0x0BC
// Int32
#define PLAYER_VISIBLE_ITEM_10_0_07             UNIT_END + 0x0BD
// Chars?
#define PLAYER_VISIBLE_ITEM_10_PROPERTIES   UNIT_END + 0x0BE
// Int32
#define PLAYER_VISIBLE_ITEM_10_PAD          UNIT_END + 0x0BF
// GUID
#define PLAYER_VISIBLE_ITEM_11_CREATOR              UNIT_END + 0x0C0
// GUID
#define PLAYER_VISIBLE_ITEM_11_CREATOR_01           UNIT_END + 0x0C1
// Int32
#define PLAYER_VISIBLE_ITEM_11_0                UNIT_END + 0x0C2
// Int32
#define PLAYER_VISIBLE_ITEM_11_0_01             UNIT_END + 0x0C3
// Int32
#define PLAYER_VISIBLE_ITEM_11_0_02             UNIT_END + 0x0C4
// Int32
#define PLAYER_VISIBLE_ITEM_11_0_03             UNIT_END + 0x0C5
// Int32
#define PLAYER_VISIBLE_ITEM_11_0_04             UNIT_END + 0x0C6
// Int32
#define PLAYER_VISIBLE_ITEM_11_0_05             UNIT_END + 0x0C7
// Int32
#define PLAYER_VISIBLE_ITEM_11_0_06             UNIT_END + 0x0C8
// Int32
#define PLAYER_VISIBLE_ITEM_11_0_07             UNIT_END + 0x0C9
// Chars?
#define PLAYER_VISIBLE_ITEM_11_PROPERTIES   UNIT_END + 0x0CA
// Int32
#define PLAYER_VISIBLE_ITEM_11_PAD          UNIT_END + 0x0CB
// GUID
#define PLAYER_VISIBLE_ITEM_12_CREATOR              UNIT_END + 0x0CC
// GUID
#define PLAYER_VISIBLE_ITEM_12_CREATOR_01           UNIT_END + 0x0CD
// Int32
#define PLAYER_VISIBLE_ITEM_12_0                UNIT_END + 0x0CE
// Int32
#define PLAYER_VISIBLE_ITEM_12_0_01             UNIT_END + 0x0CF
// Int32
#define PLAYER_VISIBLE_ITEM_12_0_02             UNIT_END + 0x0D0
// Int32
#define PLAYER_VISIBLE_ITEM_12_0_03             UNIT_END + 0x0D1
// Int32
#define PLAYER_VISIBLE_ITEM_12_0_04             UNIT_END + 0x0D2
// Int32
#define PLAYER_VISIBLE_ITEM_12_0_05             UNIT_END + 0x0D3
// Int32
#define PLAYER_VISIBLE_ITEM_12_0_06             UNIT_END + 0x0D4
// Int32
#define PLAYER_VISIBLE_ITEM_12_0_07             UNIT_END + 0x0D5
// Chars?
#define PLAYER_VISIBLE_ITEM_12_PROPERTIES   UNIT_END + 0x0D6
// Int32
#define PLAYER_VISIBLE_ITEM_12_PAD          UNIT_END + 0x0D7
// GUID
#define PLAYER_VISIBLE_ITEM_13_CREATOR              UNIT_END + 0x0D8
// GUID
#define PLAYER_VISIBLE_ITEM_13_CREATOR_01           UNIT_END + 0x0D9
// Int32
#define PLAYER_VISIBLE_ITEM_13_0                UNIT_END + 0x0DA
// Int32
#define PLAYER_VISIBLE_ITEM_13_0_01             UNIT_END + 0x0DB
// Int32
#define PLAYER_VISIBLE_ITEM_13_0_02             UNIT_END + 0x0DC
// Int32
#define PLAYER_VISIBLE_ITEM_13_0_03             UNIT_END + 0x0DD
// Int32
#define PLAYER_VISIBLE_ITEM_13_0_04             UNIT_END + 0x0DE
// Int32
#define PLAYER_VISIBLE_ITEM_13_0_05             UNIT_END + 0x0DF
// Int32
#define PLAYER_VISIBLE_ITEM_13_0_06             UNIT_END + 0x0E0
// Int32
#define PLAYER_VISIBLE_ITEM_13_0_07             UNIT_END + 0x0E1
// Chars?
#define PLAYER_VISIBLE_ITEM_13_PROPERTIES   UNIT_END + 0x0E2
// Int32
#define PLAYER_VISIBLE_ITEM_13_PAD          UNIT_END + 0x0E3
// GUID
#define PLAYER_VISIBLE_ITEM_14_CREATOR              UNIT_END + 0x0E4
// GUID
#define PLAYER_VISIBLE_ITEM_14_CREATOR_01           UNIT_END + 0x0E5
// Int32
#define PLAYER_VISIBLE_ITEM_14_0                UNIT_END + 0x0E6
// Int32
#define PLAYER_VISIBLE_ITEM_14_0_01             UNIT_END + 0x0E7
// Int32
#define PLAYER_VISIBLE_ITEM_14_0_02             UNIT_END + 0x0E8
// Int32
#define PLAYER_VISIBLE_ITEM_14_0_03             UNIT_END + 0x0E9
// Int32
#define PLAYER_VISIBLE_ITEM_14_0_04             UNIT_END + 0x0EA
// Int32
#define PLAYER_VISIBLE_ITEM_14_0_05             UNIT_END + 0x0EB
// Int32
#define PLAYER_VISIBLE_ITEM_14_0_06             UNIT_END + 0x0EC
// Int32
#define PLAYER_VISIBLE_ITEM_14_0_07             UNIT_END + 0x0ED
// Chars?
#define PLAYER_VISIBLE_ITEM_14_PROPERTIES   UNIT_END + 0x0EE
// Int32
#define PLAYER_VISIBLE_ITEM_14_PAD          UNIT_END + 0x0EF
// GUID
#define PLAYER_VISIBLE_ITEM_15_CREATOR              UNIT_END + 0x0F0
// GUID
#define PLAYER_VISIBLE_ITEM_15_CREATOR_01           UNIT_END + 0x0F1
// Int32
#define PLAYER_VISIBLE_ITEM_15_0                UNIT_END + 0x0F2
// Int32
#define PLAYER_VISIBLE_ITEM_15_0_01             UNIT_END + 0x0F3
// Int32
#define PLAYER_VISIBLE_ITEM_15_0_02             UNIT_END + 0x0F4
// Int32
#define PLAYER_VISIBLE_ITEM_15_0_03             UNIT_END + 0x0F5
// Int32
#define PLAYER_VISIBLE_ITEM_15_0_04             UNIT_END + 0x0F6
// Int32
#define PLAYER_VISIBLE_ITEM_15_0_05             UNIT_END + 0x0F7
// Int32
#define PLAYER_VISIBLE_ITEM_15_0_06             UNIT_END + 0x0F8
// Int32
#define PLAYER_VISIBLE_ITEM_15_0_07             UNIT_END + 0x0F9
// Chars?
#define PLAYER_VISIBLE_ITEM_15_PROPERTIES   UNIT_END + 0x0FA
// Int32
#define PLAYER_VISIBLE_ITEM_15_PAD          UNIT_END + 0x0FB
// GUID
#define PLAYER_VISIBLE_ITEM_16_CREATOR              UNIT_END + 0x0FC
// GUID
#define PLAYER_VISIBLE_ITEM_16_CREATOR_01           UNIT_END + 0x0FD
// Int32
#define PLAYER_VISIBLE_ITEM_16_0                UNIT_END + 0x0FE
// Int32
#define PLAYER_VISIBLE_ITEM_16_0_01             UNIT_END + 0x0FF
// Int32
#define PLAYER_VISIBLE_ITEM_16_0_02             UNIT_END + 0x100
// Int32
#define PLAYER_VISIBLE_ITEM_16_0_03             UNIT_END + 0x101
// Int32
#define PLAYER_VISIBLE_ITEM_16_0_04             UNIT_END + 0x102
// Int32
#define PLAYER_VISIBLE_ITEM_16_0_05             UNIT_END + 0x103
// Int32
#define PLAYER_VISIBLE_ITEM_16_0_06             UNIT_END + 0x104
// Int32
#define PLAYER_VISIBLE_ITEM_16_0_07             UNIT_END + 0x105
// Chars?
#define PLAYER_VISIBLE_ITEM_16_PROPERTIES   UNIT_END + 0x106
// Int32
#define PLAYER_VISIBLE_ITEM_16_PAD          UNIT_END + 0x107
// GUID
#define PLAYER_VISIBLE_ITEM_17_CREATOR              UNIT_END + 0x108
// GUID
#define PLAYER_VISIBLE_ITEM_17_CREATOR_01           UNIT_END + 0x109
// Int32
#define PLAYER_VISIBLE_ITEM_17_0                UNIT_END + 0x10A
// Int32
#define PLAYER_VISIBLE_ITEM_17_0_01             UNIT_END + 0x10B
// Int32
#define PLAYER_VISIBLE_ITEM_17_0_02             UNIT_END + 0x10C
// Int32
#define PLAYER_VISIBLE_ITEM_17_0_03             UNIT_END + 0x10D
// Int32
#define PLAYER_VISIBLE_ITEM_17_0_04             UNIT_END + 0x10E
// Int32
#define PLAYER_VISIBLE_ITEM_17_0_05             UNIT_END + 0x10F
// Int32
#define PLAYER_VISIBLE_ITEM_17_0_06             UNIT_END + 0x110
// Int32
#define PLAYER_VISIBLE_ITEM_17_0_07             UNIT_END + 0x111
// Chars?
#define PLAYER_VISIBLE_ITEM_17_PROPERTIES   UNIT_END + 0x112
// Int32
#define PLAYER_VISIBLE_ITEM_17_PAD          UNIT_END + 0x113
// GUID
#define PLAYER_VISIBLE_ITEM_18_CREATOR              UNIT_END + 0x114
// GUID
#define PLAYER_VISIBLE_ITEM_18_CREATOR_01           UNIT_END + 0x115
// Int32
#define PLAYER_VISIBLE_ITEM_18_0                UNIT_END + 0x116
// Int32
#define PLAYER_VISIBLE_ITEM_18_0_01             UNIT_END + 0x117
// Int32
#define PLAYER_VISIBLE_ITEM_18_0_02             UNIT_END + 0x118
// Int32
#define PLAYER_VISIBLE_ITEM_18_0_03             UNIT_END + 0x119
// Int32
#define PLAYER_VISIBLE_ITEM_18_0_04             UNIT_END + 0x11A
// Int32
#define PLAYER_VISIBLE_ITEM_18_0_05             UNIT_END + 0x11B
// Int32
#define PLAYER_VISIBLE_ITEM_18_0_06             UNIT_END + 0x11C
// Int32
#define PLAYER_VISIBLE_ITEM_18_0_07             UNIT_END + 0x11D
// Chars?
#define PLAYER_VISIBLE_ITEM_18_PROPERTIES   UNIT_END + 0x11E
// Int32
#define PLAYER_VISIBLE_ITEM_18_PAD          UNIT_END + 0x11F
// GUID
#define PLAYER_VISIBLE_ITEM_19_CREATOR              UNIT_END + 0x120
// GUID
#define PLAYER_VISIBLE_ITEM_19_CREATOR_01           UNIT_END + 0x121
// Int32
#define PLAYER_VISIBLE_ITEM_19_0                UNIT_END + 0x122
// Int32
#define PLAYER_VISIBLE_ITEM_19_0_01             UNIT_END + 0x123
// Int32
#define PLAYER_VISIBLE_ITEM_19_0_02             UNIT_END + 0x124
// Int32
#define PLAYER_VISIBLE_ITEM_19_0_03             UNIT_END + 0x125
// Int32
#define PLAYER_VISIBLE_ITEM_19_0_04             UNIT_END + 0x126
// Int32
#define PLAYER_VISIBLE_ITEM_19_0_05             UNIT_END + 0x127
// Int32
#define PLAYER_VISIBLE_ITEM_19_0_06             UNIT_END + 0x128
// Int32
#define PLAYER_VISIBLE_ITEM_19_0_07             UNIT_END + 0x129
// Chars?
#define PLAYER_VISIBLE_ITEM_19_PROPERTIES   UNIT_END + 0x12A
// Int32
#define PLAYER_VISIBLE_ITEM_19_PAD         UNIT_END +  0x12B
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD              UNIT_END + 0x12C
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_01           UNIT_END + 0x12D
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_02           UNIT_END + 0x12E
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_03           UNIT_END + 0x12F
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_04           UNIT_END + 0x130
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_05           UNIT_END + 0x131
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_06           UNIT_END + 0x132
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_07           UNIT_END + 0x133
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_08           UNIT_END + 0x134
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_09           UNIT_END + 0x135
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_10           UNIT_END + 0x136
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_11           UNIT_END + 0x137
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_12           UNIT_END + 0x138
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_13           UNIT_END + 0x139
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_14           UNIT_END + 0x13A
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_15           UNIT_END + 0x13B
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_16           UNIT_END + 0x13C
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_17           UNIT_END + 0x13D
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_18           UNIT_END + 0x13E
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_19           UNIT_END + 0x13F
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_20           UNIT_END + 0x140
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_21           UNIT_END + 0x141
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_22           UNIT_END + 0x142
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_23           UNIT_END + 0x143
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_24           UNIT_END + 0x144
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_25           UNIT_END + 0x145
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_26           UNIT_END + 0x146
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_27           UNIT_END + 0x147
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_28           UNIT_END + 0x148
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_29           UNIT_END + 0x149
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_30           UNIT_END + 0x14A
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_31           UNIT_END + 0x14B
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_32           UNIT_END + 0x14C
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_33           UNIT_END + 0x14D
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_34           UNIT_END + 0x14E
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_35           UNIT_END + 0x14F
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_36           UNIT_END + 0x150
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_37           UNIT_END + 0x151
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_38           UNIT_END + 0x152
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_39           UNIT_END + 0x153
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_40           UNIT_END + 0x154
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_41           UNIT_END + 0x155
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_42           UNIT_END + 0x156
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_43           UNIT_END + 0x157
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_44           UNIT_END + 0x158
// GUID
#define PLAYER_FIELD_INV_SLOT_HEAD_45           UNIT_END + 0x159
// GUID
#define PLAYER_FIELD_PACK_SLOT_1                UNIT_END + 0x15A
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_01             UNIT_END + 0x15B
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_02             UNIT_END + 0x15C
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_03             UNIT_END + 0x15D
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_04             UNIT_END + 0x15E
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_05             UNIT_END + 0x15F
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_06             UNIT_END + 0x160
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_07             UNIT_END + 0x161
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_08             UNIT_END + 0x162
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_09             UNIT_END + 0x163
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_10             UNIT_END + 0x164
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_11             UNIT_END + 0x165
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_12             UNIT_END + 0x166
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_13             UNIT_END + 0x167
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_14             UNIT_END + 0x168
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_15             UNIT_END + 0x169
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_16             UNIT_END + 0x16A
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_17             UNIT_END + 0x16B
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_18             UNIT_END + 0x16C
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_19             UNIT_END + 0x16D
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_20             UNIT_END + 0x16E
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_21             UNIT_END + 0x16F
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_22             UNIT_END + 0x170
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_23             UNIT_END + 0x171
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_24             UNIT_END + 0x172
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_25             UNIT_END + 0x173
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_26             UNIT_END + 0x174
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_27             UNIT_END + 0x175
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_28             UNIT_END + 0x176
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_29             UNIT_END + 0x177
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_30             UNIT_END + 0x178
// GUID
#define PLAYER_FIELD_PACK_SLOT_1_31             UNIT_END + 0x179
// GUID
#define PLAYER_FIELD_BANK_SLOT_1                UNIT_END + 0x17A
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_01             UNIT_END + 0x17B
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_02             UNIT_END + 0x17C
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_03             UNIT_END + 0x17D
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_04             UNIT_END + 0x17E
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_05             UNIT_END + 0x17F
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_06             UNIT_END + 0x180
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_07             UNIT_END + 0x181
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_08             UNIT_END + 0x182
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_09             UNIT_END + 0x183
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_10             UNIT_END + 0x184
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_11             UNIT_END + 0x185
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_12             UNIT_END + 0x186
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_13             UNIT_END + 0x187
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_14             UNIT_END + 0x188
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_15             UNIT_END + 0x189
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_16             UNIT_END + 0x18A
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_17             UNIT_END + 0x18B
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_18             UNIT_END + 0x18C
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_19             UNIT_END + 0x18D
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_20             UNIT_END + 0x18E
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_21             UNIT_END + 0x18F
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_22             UNIT_END + 0x190
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_23             UNIT_END + 0x191
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_24             UNIT_END + 0x192
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_25             UNIT_END + 0x193
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_26             UNIT_END + 0x194
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_27             UNIT_END + 0x195
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_28             UNIT_END + 0x196
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_29             UNIT_END + 0x197
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_30             UNIT_END + 0x198
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_31             UNIT_END + 0x199
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_32             UNIT_END + 0x19A
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_33             UNIT_END + 0x19B
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_34             UNIT_END + 0x19C
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_35             UNIT_END + 0x19D
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_36             UNIT_END + 0x19E
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_37             UNIT_END + 0x19F
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_38             UNIT_END + 0x1A0
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_39             UNIT_END + 0x1A1
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_40             UNIT_END + 0x1A2
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_41             UNIT_END + 0x1A3
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_42             UNIT_END + 0x1A4
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_43             UNIT_END + 0x1A5
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_44             UNIT_END + 0x1A6
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_45             UNIT_END + 0x1A7
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_46             UNIT_END + 0x1A8
// GUID
#define PLAYER_FIELD_BANK_SLOT_1_47             UNIT_END + 0x1A9
// GUID
#define PLAYER_FIELD_BANKBAG_SLOT_1             UNIT_END + 0x1AA
// GUID
#define PLAYER_FIELD_BANKBAG_SLOT_1_01          UNIT_END + 0x1AB
// GUID
#define PLAYER_FIELD_BANKBAG_SLOT_1_02          UNIT_END + 0x1AC
// GUID
#define PLAYER_FIELD_BANKBAG_SLOT_1_03          UNIT_END + 0x1AD
// GUID
#define PLAYER_FIELD_BANKBAG_SLOT_1_04          UNIT_END + 0x1AE
// GUID
#define PLAYER_FIELD_BANKBAG_SLOT_1_05          UNIT_END + 0x1AF
// GUID
#define PLAYER_FIELD_BANKBAG_SLOT_1_06          UNIT_END + 0x1B0
// GUID
#define PLAYER_FIELD_BANKBAG_SLOT_1_07          UNIT_END + 0x1B1
// GUID
#define PLAYER_FIELD_BANKBAG_SLOT_1_08          UNIT_END + 0x1B2
// GUID
#define PLAYER_FIELD_BANKBAG_SLOT_1_09          UNIT_END + 0x1B3
// GUID
#define PLAYER_FIELD_BANKBAG_SLOT_1_10          UNIT_END + 0x1B4
// GUID
#define PLAYER_FIELD_BANKBAG_SLOT_1_11          UNIT_END + 0x1B5
// GUID
#define PLAYER_FIELD_VENDORBUYBACK_SLOT             UNIT_END + 0x1B6
// GUID
#define PLAYER_FIELD_VENDORBUYBACK_SLOT_01          UNIT_END + 0x1B7
// GUID
#define PLAYER_FARSIGHT             UNIT_END + 0x1B8
// GUID
#define PLAYER_FARSIGHT_01          UNIT_END + 0x1B9
// GUID
#define PLAYER__FIELD_COMBO_TARGET              UNIT_END + 0x1BA
// GUID
#define PLAYER__FIELD_COMBO_TARGET_01           UNIT_END + 0x1BB
// GUID
#define PLAYER_FIELD_BUYBACK_NPC                UNIT_END + 0x1BC
// GUID
#define PLAYER_FIELD_BUYBACK_NPC_01             UNIT_END + 0x1BD
// Int32
#define PLAYER_XP                           UNIT_END + 0x1BE
// Int32
#define PLAYER_NEXT_LEVEL_XP                UNIT_END + 0x1BF
// Chars?
#define PLAYER_SKILL_INFO_1_1               UNIT_END + 0x1C0
// Chars?
#define PLAYER_SKILL_INFO_1_1_01            UNIT_END + 0x1C1
// Chars?
#define PLAYER_SKILL_INFO_1_1_02            UNIT_END + 0x1C2
// Chars?
#define PLAYER_SKILL_INFO_1_1_03            UNIT_END + 0x1C3
// Chars?
#define PLAYER_SKILL_INFO_1_1_04            UNIT_END + 0x1C4
// Chars?
#define PLAYER_SKILL_INFO_1_1_05            UNIT_END + 0x1C5
// Chars?
#define PLAYER_SKILL_INFO_1_1_06            UNIT_END + 0x1C6
// Chars?
#define PLAYER_SKILL_INFO_1_1_07            UNIT_END + 0x1C7
// Chars?
#define PLAYER_SKILL_INFO_1_1_08            UNIT_END + 0x1C8
// Chars?
#define PLAYER_SKILL_INFO_1_1_09            UNIT_END + 0x1C9
// Chars?
#define PLAYER_SKILL_INFO_1_1_10            UNIT_END + 0x1CA
// Chars?
#define PLAYER_SKILL_INFO_1_1_11            UNIT_END + 0x1CB
// Chars?
#define PLAYER_SKILL_INFO_1_1_12            UNIT_END + 0x1CC
// Chars?
#define PLAYER_SKILL_INFO_1_1_13            UNIT_END + 0x1CD
// Chars?
#define PLAYER_SKILL_INFO_1_1_14            UNIT_END + 0x1CE
// Chars?
#define PLAYER_SKILL_INFO_1_1_15            UNIT_END + 0x1CF
// Chars?
#define PLAYER_SKILL_INFO_1_1_16            UNIT_END + 0x1D0
// Chars?
#define PLAYER_SKILL_INFO_1_1_17            UNIT_END + 0x1D1
// Chars?
#define PLAYER_SKILL_INFO_1_1_18            UNIT_END + 0x1D2
// Chars?
#define PLAYER_SKILL_INFO_1_1_19            UNIT_END + 0x1D3
// Chars?
#define PLAYER_SKILL_INFO_1_1_20            UNIT_END + 0x1D4
// Chars?
#define PLAYER_SKILL_INFO_1_1_21            UNIT_END + 0x1D5
// Chars?
#define PLAYER_SKILL_INFO_1_1_22            UNIT_END + 0x1D6
// Chars?
#define PLAYER_SKILL_INFO_1_1_23            UNIT_END + 0x1D7
// Chars?
#define PLAYER_SKILL_INFO_1_1_24            UNIT_END + 0x1D8
// Chars?
#define PLAYER_SKILL_INFO_1_1_25            UNIT_END + 0x1D9
// Chars?
#define PLAYER_SKILL_INFO_1_1_26            UNIT_END + 0x1DA
// Chars?
#define PLAYER_SKILL_INFO_1_1_27            UNIT_END + 0x1DB
// Chars?
#define PLAYER_SKILL_INFO_1_1_28            UNIT_END + 0x1DC
// Chars?
#define PLAYER_SKILL_INFO_1_1_29            UNIT_END + 0x1DD
// Chars?
#define PLAYER_SKILL_INFO_1_1_30            UNIT_END + 0x1DE
// Chars?
#define PLAYER_SKILL_INFO_1_1_31            UNIT_END + 0x1DF
// Chars?
#define PLAYER_SKILL_INFO_1_1_32            UNIT_END + 0x1E0
// Chars?
#define PLAYER_SKILL_INFO_1_1_33            UNIT_END + 0x1E1
// Chars?
#define PLAYER_SKILL_INFO_1_1_34            UNIT_END + 0x1E2
// Chars?
#define PLAYER_SKILL_INFO_1_1_35            UNIT_END + 0x1E3
// Chars?
#define PLAYER_SKILL_INFO_1_1_36            UNIT_END + 0x1E4
// Chars?
#define PLAYER_SKILL_INFO_1_1_37            UNIT_END + 0x1E5
// Chars?
#define PLAYER_SKILL_INFO_1_1_38            UNIT_END + 0x1E6
// Chars?
#define PLAYER_SKILL_INFO_1_1_39            UNIT_END + 0x1E7
// Chars?
#define PLAYER_SKILL_INFO_1_1_40            UNIT_END + 0x1E8
// Chars?
#define PLAYER_SKILL_INFO_1_1_41            UNIT_END + 0x1E9
// Chars?
#define PLAYER_SKILL_INFO_1_1_42            UNIT_END + 0x1EA
// Chars?
#define PLAYER_SKILL_INFO_1_1_43            UNIT_END + 0x1EB
// Chars?
#define PLAYER_SKILL_INFO_1_1_44            UNIT_END + 0x1EC
// Chars?
#define PLAYER_SKILL_INFO_1_1_45            UNIT_END + 0x1ED
// Chars?
#define PLAYER_SKILL_INFO_1_1_46            UNIT_END + 0x1EE
// Chars?
#define PLAYER_SKILL_INFO_1_1_47            UNIT_END + 0x1EF
// Chars?
#define PLAYER_SKILL_INFO_1_1_48            UNIT_END + 0x1F0
// Chars?
#define PLAYER_SKILL_INFO_1_1_49            UNIT_END + 0x1F1
// Chars?
#define PLAYER_SKILL_INFO_1_1_50            UNIT_END + 0x1F2
// Chars?
#define PLAYER_SKILL_INFO_1_1_51            UNIT_END + 0x1F3
// Chars?
#define PLAYER_SKILL_INFO_1_1_52            UNIT_END + 0x1F4
// Chars?
#define PLAYER_SKILL_INFO_1_1_53            UNIT_END + 0x1F5
// Chars?
#define PLAYER_SKILL_INFO_1_1_54            UNIT_END + 0x1F6
// Chars?
#define PLAYER_SKILL_INFO_1_1_55            UNIT_END + 0x1F7
// Chars?
#define PLAYER_SKILL_INFO_1_1_56            UNIT_END + 0x1F8
// Chars?
#define PLAYER_SKILL_INFO_1_1_57            UNIT_END + 0x1F9
// Chars?
#define PLAYER_SKILL_INFO_1_1_58            UNIT_END + 0x1FA
// Chars?
#define PLAYER_SKILL_INFO_1_1_59            UNIT_END + 0x1FB
// Chars?
#define PLAYER_SKILL_INFO_1_1_60            UNIT_END + 0x1FC
// Chars?
#define PLAYER_SKILL_INFO_1_1_61            UNIT_END + 0x1FD
// Chars?
#define PLAYER_SKILL_INFO_1_1_62            UNIT_END + 0x1FE
// Chars?
#define PLAYER_SKILL_INFO_1_1_63            UNIT_END + 0x1FF
// Chars?
#define PLAYER_SKILL_INFO_1_1_64            UNIT_END + 0x200
// Chars?
#define PLAYER_SKILL_INFO_1_1_65            UNIT_END + 0x201
// Chars?
#define PLAYER_SKILL_INFO_1_1_66            UNIT_END + 0x202
// Chars?
#define PLAYER_SKILL_INFO_1_1_67            UNIT_END + 0x203
// Chars?
#define PLAYER_SKILL_INFO_1_1_68            UNIT_END + 0x204
// Chars?
#define PLAYER_SKILL_INFO_1_1_69            UNIT_END + 0x205
// Chars?
#define PLAYER_SKILL_INFO_1_1_70            UNIT_END + 0x206
// Chars?
#define PLAYER_SKILL_INFO_1_1_71            UNIT_END + 0x207
// Chars?
#define PLAYER_SKILL_INFO_1_1_72            UNIT_END + 0x208
// Chars?
#define PLAYER_SKILL_INFO_1_1_73            UNIT_END + 0x209
// Chars?
#define PLAYER_SKILL_INFO_1_1_74            UNIT_END + 0x20A
// Chars?
#define PLAYER_SKILL_INFO_1_1_75            UNIT_END + 0x20B
// Chars?
#define PLAYER_SKILL_INFO_1_1_76            UNIT_END + 0x20C
// Chars?
#define PLAYER_SKILL_INFO_1_1_77            UNIT_END + 0x20D
// Chars?
#define PLAYER_SKILL_INFO_1_1_78            UNIT_END + 0x20E
// Chars?
#define PLAYER_SKILL_INFO_1_1_79            UNIT_END + 0x20F
// Chars?
#define PLAYER_SKILL_INFO_1_1_80            UNIT_END + 0x210
// Chars?
#define PLAYER_SKILL_INFO_1_1_81            UNIT_END + 0x211
// Chars?
#define PLAYER_SKILL_INFO_1_1_82            UNIT_END + 0x212
// Chars?
#define PLAYER_SKILL_INFO_1_1_83            UNIT_END + 0x213
// Chars?
#define PLAYER_SKILL_INFO_1_1_84            UNIT_END + 0x214
// Chars?
#define PLAYER_SKILL_INFO_1_1_85            UNIT_END + 0x215
// Chars?
#define PLAYER_SKILL_INFO_1_1_86            UNIT_END + 0x216
// Chars?
#define PLAYER_SKILL_INFO_1_1_87            UNIT_END + 0x217
// Chars?
#define PLAYER_SKILL_INFO_1_1_88            UNIT_END + 0x218
// Chars?
#define PLAYER_SKILL_INFO_1_1_89            UNIT_END + 0x219
// Chars?
#define PLAYER_SKILL_INFO_1_1_90            UNIT_END + 0x21A
// Chars?
#define PLAYER_SKILL_INFO_1_1_91            UNIT_END + 0x21B
// Chars?
#define PLAYER_SKILL_INFO_1_1_92            UNIT_END + 0x21C
// Chars?
#define PLAYER_SKILL_INFO_1_1_93            UNIT_END + 0x21D
// Chars?
#define PLAYER_SKILL_INFO_1_1_94            UNIT_END + 0x21E
// Chars?
#define PLAYER_SKILL_INFO_1_1_95            UNIT_END + 0x21F
// Chars?
#define PLAYER_SKILL_INFO_1_1_96            UNIT_END + 0x220
// Chars?
#define PLAYER_SKILL_INFO_1_1_97            UNIT_END + 0x221
// Chars?
#define PLAYER_SKILL_INFO_1_1_98            UNIT_END + 0x222
// Chars?
#define PLAYER_SKILL_INFO_1_1_99            UNIT_END + 0x223
// Chars?
#define PLAYER_SKILL_INFO_1_1_100           UNIT_END + 0x224
// Chars?
#define PLAYER_SKILL_INFO_1_1_101           UNIT_END + 0x225
// Chars?
#define PLAYER_SKILL_INFO_1_1_102           UNIT_END + 0x226
// Chars?
#define PLAYER_SKILL_INFO_1_1_103           UNIT_END + 0x227
// Chars?
#define PLAYER_SKILL_INFO_1_1_104           UNIT_END + 0x228
// Chars?
#define PLAYER_SKILL_INFO_1_1_105           UNIT_END + 0x229
// Chars?
#define PLAYER_SKILL_INFO_1_1_106           UNIT_END + 0x22A
// Chars?
#define PLAYER_SKILL_INFO_1_1_107           UNIT_END + 0x22B
// Chars?
#define PLAYER_SKILL_INFO_1_1_108           UNIT_END + 0x22C
// Chars?
#define PLAYER_SKILL_INFO_1_1_109           UNIT_END + 0x22D
// Chars?
#define PLAYER_SKILL_INFO_1_1_110           UNIT_END + 0x22E
// Chars?
#define PLAYER_SKILL_INFO_1_1_111           UNIT_END + 0x22F
// Chars?
#define PLAYER_SKILL_INFO_1_1_112           UNIT_END + 0x230
// Chars?
#define PLAYER_SKILL_INFO_1_1_113           UNIT_END + 0x231
// Chars?
#define PLAYER_SKILL_INFO_1_1_114           UNIT_END + 0x232
// Chars?
#define PLAYER_SKILL_INFO_1_1_115           UNIT_END + 0x233
// Chars?
#define PLAYER_SKILL_INFO_1_1_116           UNIT_END + 0x234
// Chars?
#define PLAYER_SKILL_INFO_1_1_117           UNIT_END + 0x235
// Chars?
#define PLAYER_SKILL_INFO_1_1_118           UNIT_END + 0x236
// Chars?
#define PLAYER_SKILL_INFO_1_1_119           UNIT_END + 0x237
// Chars?
#define PLAYER_SKILL_INFO_1_1_120           UNIT_END + 0x238
// Chars?
#define PLAYER_SKILL_INFO_1_1_121           UNIT_END + 0x239
// Chars?
#define PLAYER_SKILL_INFO_1_1_122           UNIT_END + 0x23A
// Chars?
#define PLAYER_SKILL_INFO_1_1_123           UNIT_END + 0x23B
// Chars?
#define PLAYER_SKILL_INFO_1_1_124           UNIT_END + 0x23C
// Chars?
#define PLAYER_SKILL_INFO_1_1_125           UNIT_END + 0x23D
// Chars?
#define PLAYER_SKILL_INFO_1_1_126           UNIT_END + 0x23E
// Chars?
#define PLAYER_SKILL_INFO_1_1_127           UNIT_END + 0x23F
// Chars?
#define PLAYER_SKILL_INFO_1_1_128           UNIT_END + 0x240
// Chars?
#define PLAYER_SKILL_INFO_1_1_129           UNIT_END + 0x241
// Chars?
#define PLAYER_SKILL_INFO_1_1_130           UNIT_END + 0x242
// Chars?
#define PLAYER_SKILL_INFO_1_1_131           UNIT_END + 0x243
// Chars?
#define PLAYER_SKILL_INFO_1_1_132           UNIT_END + 0x244
// Chars?
#define PLAYER_SKILL_INFO_1_1_133           UNIT_END + 0x245
// Chars?
#define PLAYER_SKILL_INFO_1_1_134           UNIT_END + 0x246
// Chars?
#define PLAYER_SKILL_INFO_1_1_135           UNIT_END + 0x247
// Chars?
#define PLAYER_SKILL_INFO_1_1_136           UNIT_END + 0x248
// Chars?
#define PLAYER_SKILL_INFO_1_1_137           UNIT_END + 0x249
// Chars?
#define PLAYER_SKILL_INFO_1_1_138           UNIT_END + 0x24A
// Chars?
#define PLAYER_SKILL_INFO_1_1_139           UNIT_END + 0x24B
// Chars?
#define PLAYER_SKILL_INFO_1_1_140           UNIT_END + 0x24C
// Chars?
#define PLAYER_SKILL_INFO_1_1_141           UNIT_END + 0x24D
// Chars?
#define PLAYER_SKILL_INFO_1_1_142           UNIT_END + 0x24E
// Chars?
#define PLAYER_SKILL_INFO_1_1_143           UNIT_END + 0x24F
// Chars?
#define PLAYER_SKILL_INFO_1_1_144           UNIT_END + 0x250
// Chars?
#define PLAYER_SKILL_INFO_1_1_145           UNIT_END + 0x251
// Chars?
#define PLAYER_SKILL_INFO_1_1_146           UNIT_END + 0x252
// Chars?
#define PLAYER_SKILL_INFO_1_1_147           UNIT_END + 0x253
// Chars?
#define PLAYER_SKILL_INFO_1_1_148           UNIT_END + 0x254
// Chars?
#define PLAYER_SKILL_INFO_1_1_149           UNIT_END + 0x255
// Chars?
#define PLAYER_SKILL_INFO_1_1_150           UNIT_END + 0x256
// Chars?
#define PLAYER_SKILL_INFO_1_1_151           UNIT_END + 0x257
// Chars?
#define PLAYER_SKILL_INFO_1_1_152           UNIT_END + 0x258
// Chars?
#define PLAYER_SKILL_INFO_1_1_153           UNIT_END + 0x259
// Chars?
#define PLAYER_SKILL_INFO_1_1_154           UNIT_END + 0x25A
// Chars?
#define PLAYER_SKILL_INFO_1_1_155           UNIT_END + 0x25B
// Chars?
#define PLAYER_SKILL_INFO_1_1_156           UNIT_END + 0x25C
// Chars?
#define PLAYER_SKILL_INFO_1_1_157           UNIT_END + 0x25D
// Chars?
#define PLAYER_SKILL_INFO_1_1_158           UNIT_END + 0x25E
// Chars?
#define PLAYER_SKILL_INFO_1_1_159           UNIT_END + 0x25F
// Chars?
#define PLAYER_SKILL_INFO_1_1_160           UNIT_END + 0x260
// Chars?
#define PLAYER_SKILL_INFO_1_1_161           UNIT_END + 0x261
// Chars?
#define PLAYER_SKILL_INFO_1_1_162           UNIT_END + 0x262
// Chars?
#define PLAYER_SKILL_INFO_1_1_163           UNIT_END + 0x263
// Chars?
#define PLAYER_SKILL_INFO_1_1_164           UNIT_END + 0x264
// Chars?
#define PLAYER_SKILL_INFO_1_1_165           UNIT_END + 0x265
// Chars?
#define PLAYER_SKILL_INFO_1_1_166           UNIT_END + 0x266
// Chars?
#define PLAYER_SKILL_INFO_1_1_167           UNIT_END + 0x267
// Chars?
#define PLAYER_SKILL_INFO_1_1_168           UNIT_END + 0x268
// Chars?
#define PLAYER_SKILL_INFO_1_1_169           UNIT_END + 0x269
// Chars?
#define PLAYER_SKILL_INFO_1_1_170           UNIT_END + 0x26A
// Chars?
#define PLAYER_SKILL_INFO_1_1_171           UNIT_END + 0x26B
// Chars?
#define PLAYER_SKILL_INFO_1_1_172           UNIT_END + 0x26C
// Chars?
#define PLAYER_SKILL_INFO_1_1_173           UNIT_END + 0x26D
// Chars?
#define PLAYER_SKILL_INFO_1_1_174           UNIT_END + 0x26E
// Chars?
#define PLAYER_SKILL_INFO_1_1_175           UNIT_END + 0x26F
// Chars?
#define PLAYER_SKILL_INFO_1_1_176           UNIT_END + 0x270
// Chars?
#define PLAYER_SKILL_INFO_1_1_177           UNIT_END + 0x271
// Chars?
#define PLAYER_SKILL_INFO_1_1_178           UNIT_END + 0x272
// Chars?
#define PLAYER_SKILL_INFO_1_1_179           UNIT_END + 0x273
// Chars?
#define PLAYER_SKILL_INFO_1_1_180         UNIT_END + 0x274
// Chars?
#define PLAYER_SKILL_INFO_1_1_181           UNIT_END + 0x275
// Chars?
#define PLAYER_SKILL_INFO_1_1_182           UNIT_END + 0x276
// Chars?
#define PLAYER_SKILL_INFO_1_1_183           UNIT_END + 0x277
// Chars?
#define PLAYER_SKILL_INFO_1_1_184           UNIT_END + 0x278
// Chars?
#define PLAYER_SKILL_INFO_1_1_185           UNIT_END + 0x279
// Chars?
#define PLAYER_SKILL_INFO_1_1_186           UNIT_END + 0x27A
// Chars?
#define PLAYER_SKILL_INFO_1_1_187           UNIT_END + 0x27B
// Chars?
#define PLAYER_SKILL_INFO_1_1_188           UNIT_END + 0x27C
// Chars?
#define PLAYER_SKILL_INFO_1_1_189           UNIT_END + 0x27D
// Chars?
#define PLAYER_SKILL_INFO_1_1_190           UNIT_END + 0x27E
// Chars?
#define PLAYER_SKILL_INFO_1_1_191           UNIT_END + 0x27F
// Chars?
#define PLAYER_SKILL_INFO_1_1_192           UNIT_END + 0x280
// Chars?
#define PLAYER_SKILL_INFO_1_1_193           UNIT_END + 0x281
// Chars?
#define PLAYER_SKILL_INFO_1_1_194           UNIT_END + 0x282
// Chars?
#define PLAYER_SKILL_INFO_1_1_195           UNIT_END + 0x283
// Chars?
#define PLAYER_SKILL_INFO_1_1_196           UNIT_END + 0x284
// Chars?
#define PLAYER_SKILL_INFO_1_1_197           UNIT_END + 0x285
// Chars?
#define PLAYER_SKILL_INFO_1_1_198           UNIT_END + 0x286
// Chars?
#define PLAYER_SKILL_INFO_1_1_199           UNIT_END + 0x287
// Chars?
#define PLAYER_SKILL_INFO_1_1_200           UNIT_END + 0x288
// Chars?
#define PLAYER_SKILL_INFO_1_1_201           UNIT_END + 0x289
// Chars?
#define PLAYER_SKILL_INFO_1_1_202           UNIT_END + 0x28A
// Chars?
#define PLAYER_SKILL_INFO_1_1_203           UNIT_END + 0x28B
// Chars?
#define PLAYER_SKILL_INFO_1_1_204           UNIT_END + 0x28C
// Chars?
#define PLAYER_SKILL_INFO_1_1_205           UNIT_END + 0x28D
// Chars?
#define PLAYER_SKILL_INFO_1_1_206           UNIT_END + 0x28E
// Chars?
#define PLAYER_SKILL_INFO_1_1_207           UNIT_END + 0x28F
// Chars?
#define PLAYER_SKILL_INFO_1_1_208           UNIT_END + 0x290
// Chars?
#define PLAYER_SKILL_INFO_1_1_209           UNIT_END + 0x291
// Chars?
#define PLAYER_SKILL_INFO_1_1_210           UNIT_END + 0x292
// Chars?
#define PLAYER_SKILL_INFO_1_1_211           UNIT_END + 0x293
// Chars?
#define PLAYER_SKILL_INFO_1_1_212           UNIT_END + 0x294
// Chars?
#define PLAYER_SKILL_INFO_1_1_213           UNIT_END + 0x295
// Chars?
#define PLAYER_SKILL_INFO_1_1_214           UNIT_END + 0x296
// Chars?
#define PLAYER_SKILL_INFO_1_1_215           UNIT_END + 0x297
// Chars?
#define PLAYER_SKILL_INFO_1_1_216           UNIT_END + 0x298
// Chars?
#define PLAYER_SKILL_INFO_1_1_217           UNIT_END + 0x299
// Chars?
#define PLAYER_SKILL_INFO_1_1_218           UNIT_END + 0x29A
// Chars?
#define PLAYER_SKILL_INFO_1_1_219           UNIT_END + 0x29B
// Chars?
#define PLAYER_SKILL_INFO_1_1_220           UNIT_END + 0x29C
// Chars?
#define PLAYER_SKILL_INFO_1_1_221           UNIT_END + 0x29D
// Chars?
#define PLAYER_SKILL_INFO_1_1_222           UNIT_END + 0x29E
// Chars?
#define PLAYER_SKILL_INFO_1_1_223           UNIT_END + 0x29F
// Chars?
#define PLAYER_SKILL_INFO_1_1_224           UNIT_END + 0x2A0
// Chars?
#define PLAYER_SKILL_INFO_1_1_225           UNIT_END + 0x2A1
// Chars?
#define PLAYER_SKILL_INFO_1_1_226           UNIT_END + 0x2A2
// Chars?
#define PLAYER_SKILL_INFO_1_1_227           UNIT_END + 0x2A3
// Chars?
#define PLAYER_SKILL_INFO_1_1_228           UNIT_END + 0x2A4
// Chars?
#define PLAYER_SKILL_INFO_1_1_229           UNIT_END + 0x2A5
// Chars?
#define PLAYER_SKILL_INFO_1_1_230           UNIT_END + 0x2A6
// Chars?
#define PLAYER_SKILL_INFO_1_1_231           UNIT_END + 0x2A7
// Chars?
#define PLAYER_SKILL_INFO_1_1_232           UNIT_END + 0x2A8
// Chars?
#define PLAYER_SKILL_INFO_1_1_233           UNIT_END + 0x2A9
// Chars?
#define PLAYER_SKILL_INFO_1_1_234           UNIT_END + 0x2AA
// Chars?
#define PLAYER_SKILL_INFO_1_1_235           UNIT_END + 0x2AB
// Chars?
#define PLAYER_SKILL_INFO_1_1_236           UNIT_END + 0x2AC
// Chars?
#define PLAYER_SKILL_INFO_1_1_237           UNIT_END + 0x2AD
// Chars?
#define PLAYER_SKILL_INFO_1_1_238           UNIT_END + 0x2AE
// Chars?
#define PLAYER_SKILL_INFO_1_1_239           UNIT_END + 0x2AF
// Chars?
#define PLAYER_SKILL_INFO_1_1_240           UNIT_END + 0x2B0
// Chars?
#define PLAYER_SKILL_INFO_1_1_241           UNIT_END + 0x2B1
// Chars?
#define PLAYER_SKILL_INFO_1_1_242           UNIT_END + 0x2B2
// Chars?
#define PLAYER_SKILL_INFO_1_1_243           UNIT_END + 0x2B3
// Chars?
#define PLAYER_SKILL_INFO_1_1_244           UNIT_END + 0x2B4
// Chars?
#define PLAYER_SKILL_INFO_1_1_245           UNIT_END + 0x2B5
// Chars?
#define PLAYER_SKILL_INFO_1_1_246           UNIT_END + 0x2B6
// Chars?
#define PLAYER_SKILL_INFO_1_1_247           UNIT_END + 0x2B7
// Chars?
#define PLAYER_SKILL_INFO_1_1_248           UNIT_END + 0x2B8
// Chars?
#define PLAYER_SKILL_INFO_1_1_249           UNIT_END + 0x2B9
// Chars?
#define PLAYER_SKILL_INFO_1_1_250           UNIT_END + 0x2BA
// Chars?
#define PLAYER_SKILL_INFO_1_1_251           UNIT_END + 0x2BB
// Chars?
#define PLAYER_SKILL_INFO_1_1_252           UNIT_END + 0x2BC
// Chars?
#define PLAYER_SKILL_INFO_1_1_253           UNIT_END + 0x2BD
// Chars?
#define PLAYER_SKILL_INFO_1_1_254           UNIT_END + 0x2BE
// Chars?
#define PLAYER_SKILL_INFO_1_1_255           UNIT_END + 0x2BF
// Chars?
#define PLAYER_SKILL_INFO_1_1_256           UNIT_END + 0x2C0
// Chars?
#define PLAYER_SKILL_INFO_1_1_257           UNIT_END + 0x2C1
// Chars?
#define PLAYER_SKILL_INFO_1_1_258           UNIT_END + 0x2C2
// Chars?
#define PLAYER_SKILL_INFO_1_1_259           UNIT_END + 0x2C3
// Chars?
#define PLAYER_SKILL_INFO_1_1_260           UNIT_END + 0x2C4
// Chars?
#define PLAYER_SKILL_INFO_1_1_261           UNIT_END + 0x2C5
// Chars?
#define PLAYER_SKILL_INFO_1_1_262           UNIT_END + 0x2C6
// Chars?
#define PLAYER_SKILL_INFO_1_1_263           UNIT_END + 0x2C7
// Chars?
#define PLAYER_SKILL_INFO_1_1_264           UNIT_END + 0x2C8
// Chars?
#define PLAYER_SKILL_INFO_1_1_265           UNIT_END + 0x2C9
// Chars?
#define PLAYER_SKILL_INFO_1_1_266           UNIT_END + 0x2CA
// Chars?
#define PLAYER_SKILL_INFO_1_1_267           UNIT_END + 0x2CB
// Chars?
#define PLAYER_SKILL_INFO_1_1_268           UNIT_END + 0x2CC
// Chars?
#define PLAYER_SKILL_INFO_1_1_269           UNIT_END + 0x2CD
// Chars?
#define PLAYER_SKILL_INFO_1_1_270           UNIT_END + 0x2CE
// Chars?
#define PLAYER_SKILL_INFO_1_1_271           UNIT_END + 0x2CF
// Chars?
#define PLAYER_SKILL_INFO_1_1_272           UNIT_END + 0x2D0
// Chars?
#define PLAYER_SKILL_INFO_1_1_273           UNIT_END + 0x2D1
// Chars?
#define PLAYER_SKILL_INFO_1_1_274           UNIT_END + 0x2D2
// Chars?
#define PLAYER_SKILL_INFO_1_1_275           UNIT_END + 0x2D3
// Chars?
#define PLAYER_SKILL_INFO_1_1_276           UNIT_END + 0x2D4
// Chars?
#define PLAYER_SKILL_INFO_1_1_277           UNIT_END + 0x2D5
// Chars?
#define PLAYER_SKILL_INFO_1_1_278           UNIT_END + 0x2D6
// Chars?
#define PLAYER_SKILL_INFO_1_1_279           UNIT_END + 0x2D7
// Chars?
#define PLAYER_SKILL_INFO_1_1_280           UNIT_END + 0x2D8
// Chars?
#define PLAYER_SKILL_INFO_1_1_281           UNIT_END + 0x2D9
// Chars?
#define PLAYER_SKILL_INFO_1_1_282           UNIT_END + 0x2DA
// Chars?
#define PLAYER_SKILL_INFO_1_1_283           UNIT_END + 0x2DB
// Chars?
#define PLAYER_SKILL_INFO_1_1_284           UNIT_END + 0x2DC
// Chars?
#define PLAYER_SKILL_INFO_1_1_285           UNIT_END + 0x2DD
// Chars?
#define PLAYER_SKILL_INFO_1_1_286           UNIT_END + 0x2DE
// Chars?
#define PLAYER_SKILL_INFO_1_1_287           UNIT_END + 0x2DF
// Chars?
#define PLAYER_SKILL_INFO_1_1_288           UNIT_END + 0x2E0
// Chars?
#define PLAYER_SKILL_INFO_1_1_289           UNIT_END + 0x2E1
// Chars?
#define PLAYER_SKILL_INFO_1_1_290           UNIT_END + 0x2E2
// Chars?
#define PLAYER_SKILL_INFO_1_1_291           UNIT_END + 0x2E3
// Chars?
#define PLAYER_SKILL_INFO_1_1_292           UNIT_END + 0x2E4
// Chars?
#define PLAYER_SKILL_INFO_1_1_293           UNIT_END + 0x2E5
// Chars?
#define PLAYER_SKILL_INFO_1_1_294           UNIT_END + 0x2E6
// Chars?
#define PLAYER_SKILL_INFO_1_1_295           UNIT_END + 0x2E7
// Chars?
#define PLAYER_SKILL_INFO_1_1_296           UNIT_END + 0x2E8
// Chars?
#define PLAYER_SKILL_INFO_1_1_297           UNIT_END + 0x2E9
// Chars?
#define PLAYER_SKILL_INFO_1_1_298           UNIT_END + 0x2EA
// Chars?
#define PLAYER_SKILL_INFO_1_1_299           UNIT_END + 0x2EB
// Chars?
#define PLAYER_SKILL_INFO_1_1_300           UNIT_END + 0x2EC
// Chars?
#define PLAYER_SKILL_INFO_1_1_301           UNIT_END + 0x2ED
// Chars?
#define PLAYER_SKILL_INFO_1_1_302           UNIT_END + 0x2EE
// Chars?
#define PLAYER_SKILL_INFO_1_1_303           UNIT_END + 0x2EF
// Chars?
#define PLAYER_SKILL_INFO_1_1_304           UNIT_END + 0x2F0
// Chars?
#define PLAYER_SKILL_INFO_1_1_305           UNIT_END + 0x2F1
// Chars?
#define PLAYER_SKILL_INFO_1_1_306           UNIT_END + 0x2F2
// Chars?
#define PLAYER_SKILL_INFO_1_1_307           UNIT_END + 0x2F3
// Chars?
#define PLAYER_SKILL_INFO_1_1_308           UNIT_END + 0x2F4
// Chars?
#define PLAYER_SKILL_INFO_1_1_309           UNIT_END + 0x2F5
// Chars?
#define PLAYER_SKILL_INFO_1_1_310           UNIT_END + 0x2F6
// Chars?
#define PLAYER_SKILL_INFO_1_1_311           UNIT_END + 0x2F7
// Chars?
#define PLAYER_SKILL_INFO_1_1_312           UNIT_END + 0x2F8
// Chars?
#define PLAYER_SKILL_INFO_1_1_313           UNIT_END + 0x2F9
// Chars?
#define PLAYER_SKILL_INFO_1_1_314           UNIT_END + 0x2FA
// Chars?
#define PLAYER_SKILL_INFO_1_1_315           UNIT_END + 0x2FB
// Chars?
#define PLAYER_SKILL_INFO_1_1_316           UNIT_END + 0x2FC
// Chars?
#define PLAYER_SKILL_INFO_1_1_317           UNIT_END + 0x2FD
// Chars?
#define PLAYER_SKILL_INFO_1_1_318           UNIT_END + 0x2FE
// Chars?
#define PLAYER_SKILL_INFO_1_1_319           UNIT_END + 0x2FF
// Chars?
#define PLAYER_SKILL_INFO_1_1_320           UNIT_END + 0x300
// Chars?
#define PLAYER_SKILL_INFO_1_1_321           UNIT_END + 0x301
// Chars?
#define PLAYER_SKILL_INFO_1_1_322           UNIT_END + 0x302
// Chars?
#define PLAYER_SKILL_INFO_1_1_323           UNIT_END + 0x303
// Chars?
#define PLAYER_SKILL_INFO_1_1_324           UNIT_END + 0x304
// Chars?
#define PLAYER_SKILL_INFO_1_1_325           UNIT_END + 0x305
// Chars?
#define PLAYER_SKILL_INFO_1_1_326           UNIT_END + 0x306
// Chars?
#define PLAYER_SKILL_INFO_1_1_327           UNIT_END + 0x307
// Chars?
#define PLAYER_SKILL_INFO_1_1_328           UNIT_END + 0x308
// Chars?
#define PLAYER_SKILL_INFO_1_1_329           UNIT_END + 0x309
// Chars?
#define PLAYER_SKILL_INFO_1_1_330           UNIT_END + 0x30A
// Chars?
#define PLAYER_SKILL_INFO_1_1_331           UNIT_END + 0x30B
// Chars?
#define PLAYER_SKILL_INFO_1_1_332           UNIT_END + 0x30C
// Chars?
#define PLAYER_SKILL_INFO_1_1_333           UNIT_END + 0x30D
// Chars?
#define PLAYER_SKILL_INFO_1_1_334           UNIT_END + 0x30E
// Chars?
#define PLAYER_SKILL_INFO_1_1_335           UNIT_END + 0x30F
// Chars?
#define PLAYER_SKILL_INFO_1_1_336           UNIT_END + 0x310
// Chars?
#define PLAYER_SKILL_INFO_1_1_337           UNIT_END + 0x311
// Chars?
#define PLAYER_SKILL_INFO_1_1_338           UNIT_END + 0x312
// Chars?
#define PLAYER_SKILL_INFO_1_1_339           UNIT_END + 0x313
// Chars?
#define PLAYER_SKILL_INFO_1_1_340           UNIT_END + 0x314
// Chars?
#define PLAYER_SKILL_INFO_1_1_341           UNIT_END + 0x315
// Chars?
#define PLAYER_SKILL_INFO_1_1_342           UNIT_END + 0x316
// Chars?
#define PLAYER_SKILL_INFO_1_1_343           UNIT_END + 0x317
// Chars?
#define PLAYER_SKILL_INFO_1_1_344           UNIT_END + 0x318
// Chars?
#define PLAYER_SKILL_INFO_1_1_345           UNIT_END + 0x319
// Chars?
#define PLAYER_SKILL_INFO_1_1_346           UNIT_END + 0x31A
// Chars?
#define PLAYER_SKILL_INFO_1_1_347           UNIT_END + 0x31B
// Chars?
#define PLAYER_SKILL_INFO_1_1_348           UNIT_END + 0x31C
// Chars?
#define PLAYER_SKILL_INFO_1_1_349           UNIT_END + 0x31D
// Chars?
#define PLAYER_SKILL_INFO_1_1_350           UNIT_END + 0x31E
// Chars?
#define PLAYER_SKILL_INFO_1_1_351           UNIT_END + 0x31F
// Chars?
#define PLAYER_SKILL_INFO_1_1_352           UNIT_END + 0x320
// Chars?
#define PLAYER_SKILL_INFO_1_1_353           UNIT_END + 0x321
// Chars?
#define PLAYER_SKILL_INFO_1_1_354           UNIT_END + 0x322
// Chars?
#define PLAYER_SKILL_INFO_1_1_355           UNIT_END + 0x323
// Chars?
#define PLAYER_SKILL_INFO_1_1_356           UNIT_END + 0x324
// Chars?
#define PLAYER_SKILL_INFO_1_1_357           UNIT_END + 0x325
// Chars?
#define PLAYER_SKILL_INFO_1_1_358           UNIT_END + 0x326
// Chars?
#define PLAYER_SKILL_INFO_1_1_359           UNIT_END + 0x327
// Chars?
#define PLAYER_SKILL_INFO_1_1_360           UNIT_END + 0x328
// Chars?
#define PLAYER_SKILL_INFO_1_1_361           UNIT_END + 0x329
// Chars?
#define PLAYER_SKILL_INFO_1_1_362           UNIT_END + 0x32A
// Chars?
#define PLAYER_SKILL_INFO_1_1_363           UNIT_END + 0x32B
// Chars?
#define PLAYER_SKILL_INFO_1_1_364           UNIT_END + 0x32C
// Chars?
#define PLAYER_SKILL_INFO_1_1_365           UNIT_END + 0x32D
// Chars?
#define PLAYER_SKILL_INFO_1_1_366           UNIT_END + 0x32E
// Chars?
#define PLAYER_SKILL_INFO_1_1_367           UNIT_END + 0x32F
// Chars?
#define PLAYER_SKILL_INFO_1_1_368           UNIT_END + 0x330
// Chars?
#define PLAYER_SKILL_INFO_1_1_369           UNIT_END + 0x331
// Chars?
#define PLAYER_SKILL_INFO_1_1_370           UNIT_END + 0x332
// Chars?
#define PLAYER_SKILL_INFO_1_1_371           UNIT_END + 0x333
// Chars?
#define PLAYER_SKILL_INFO_1_1_372           UNIT_END + 0x334
// Chars?
#define PLAYER_SKILL_INFO_1_1_373           UNIT_END + 0x335
// Chars?
#define PLAYER_SKILL_INFO_1_1_374           UNIT_END + 0x336
// Chars?
#define PLAYER_SKILL_INFO_1_1_375           UNIT_END + 0x337
// Chars?
#define PLAYER_SKILL_INFO_1_1_376           UNIT_END + 0x338
// Chars?
#define PLAYER_SKILL_INFO_1_1_377           UNIT_END + 0x339
// Chars?
#define PLAYER_SKILL_INFO_1_1_378           UNIT_END + 0x33A
// Chars?
#define PLAYER_SKILL_INFO_1_1_379           UNIT_END + 0x33B
// Chars?
#define PLAYER_SKILL_INFO_1_1_380           UNIT_END + 0x33C
// Chars?
#define PLAYER_SKILL_INFO_1_1_381           UNIT_END + 0x33D
// Chars?
#define PLAYER_SKILL_INFO_1_1_382           UNIT_END + 0x33E
// Chars?
#define PLAYER_SKILL_INFO_1_1_383           UNIT_END + 0x33F
// Int32
#define PLAYER_CHARACTER_POINTS1            UNIT_END + 0x340
// Int32
#define PLAYER_CHARACTER_POINTS2            UNIT_END + 0x341
// Int32
#define PLAYER_TRACK_CREATURES              UNIT_END + 0x342
// Int32
#define PLAYER_TRACK_RESOURCES              UNIT_END + 0x343
// Float
#define PLAYER_BLOCK_PERCENTAGE             UNIT_END + 0x344
// Float
#define PLAYER_DODGE_PERCENTAGE             UNIT_END + 0x345
// Float
#define PLAYER_PARRY_PERCENTAGE             UNIT_END + 0x346
// Float
#define PLAYER_CRIT_PERCENTAGE              UNIT_END + 0x347
// Float
#define PLAYER_RANGED_CRIT_PERCENTAGE       UNIT_END + 0x348
// Bytes
#define PLAYER_EXPLORED_ZONES_1             UNIT_END + 0x349
// Bytes
#define PLAYER_EXPLORED_ZONES_1_01          UNIT_END + 0x34A
// Bytes
#define PLAYER_EXPLORED_ZONES_1_02          UNIT_END + 0x34B
// Bytes
#define PLAYER_EXPLORED_ZONES_1_03          UNIT_END + 0x34C
// Bytes
#define PLAYER_EXPLORED_ZONES_1_04          UNIT_END + 0x34D
// Bytes
#define PLAYER_EXPLORED_ZONES_1_05          UNIT_END + 0x34E
// Bytes
#define PLAYER_EXPLORED_ZONES_1_06          UNIT_END + 0x34F
// Bytes
#define PLAYER_EXPLORED_ZONES_1_07          UNIT_END + 0x350
// Bytes
#define PLAYER_EXPLORED_ZONES_1_08          UNIT_END + 0x351
// Bytes
#define PLAYER_EXPLORED_ZONES_1_09          UNIT_END + 0x352
// Bytes
#define PLAYER_EXPLORED_ZONES_1_10          UNIT_END + 0x353
// Bytes
#define PLAYER_EXPLORED_ZONES_1_11          UNIT_END + 0x354
// Bytes
#define PLAYER_EXPLORED_ZONES_1_12          UNIT_END + 0x355
// Bytes
#define PLAYER_EXPLORED_ZONES_1_13          UNIT_END + 0x356
// Bytes
#define PLAYER_EXPLORED_ZONES_1_14          UNIT_END + 0x357
// Bytes
#define PLAYER_EXPLORED_ZONES_1_15          UNIT_END + 0x358
// Bytes
#define PLAYER_EXPLORED_ZONES_1_16          UNIT_END + 0x359
// Bytes
#define PLAYER_EXPLORED_ZONES_1_17          UNIT_END + 0x35A
// Bytes
#define PLAYER_EXPLORED_ZONES_1_18          UNIT_END + 0x35B
// Bytes
#define PLAYER_EXPLORED_ZONES_1_19          UNIT_END + 0x35C
// Bytes
#define PLAYER_EXPLORED_ZONES_1_20          UNIT_END + 0x35D
// Bytes
#define PLAYER_EXPLORED_ZONES_1_21          UNIT_END + 0x35E
// Bytes
#define PLAYER_EXPLORED_ZONES_1_22          UNIT_END + 0x35F
// Bytes
#define PLAYER_EXPLORED_ZONES_1_23          UNIT_END + 0x360
// Bytes
#define PLAYER_EXPLORED_ZONES_1_24          UNIT_END + 0x361
// Bytes
#define PLAYER_EXPLORED_ZONES_1_25          UNIT_END + 0x362
// Bytes
#define PLAYER_EXPLORED_ZONES_1_26          UNIT_END + 0x363
// Bytes
#define PLAYER_EXPLORED_ZONES_1_27          UNIT_END + 0x364
// Bytes
#define PLAYER_EXPLORED_ZONES_1_28          UNIT_END + 0x365
// Bytes
#define PLAYER_EXPLORED_ZONES_1_29          UNIT_END + 0x366
// Bytes
#define PLAYER_EXPLORED_ZONES_1_30          UNIT_END + 0x367
// Bytes
#define PLAYER_EXPLORED_ZONES_1_31          UNIT_END + 0x368
// Bytes
#define PLAYER_EXPLORED_ZONES_1_32          UNIT_END + 0x369
// Bytes
#define PLAYER_EXPLORED_ZONES_1_33          UNIT_END + 0x36A
// Bytes
#define PLAYER_EXPLORED_ZONES_1_34          UNIT_END + 0x36B
// Bytes
#define PLAYER_EXPLORED_ZONES_1_35          UNIT_END + 0x36C
// Bytes
#define PLAYER_EXPLORED_ZONES_1_36          UNIT_END + 0x36D
// Bytes
#define PLAYER_EXPLORED_ZONES_1_37          UNIT_END + 0x36E
// Bytes
#define PLAYER_EXPLORED_ZONES_1_38          UNIT_END + 0x36F
// Bytes
#define PLAYER_EXPLORED_ZONES_1_39          UNIT_END + 0x370
// Bytes
#define PLAYER_EXPLORED_ZONES_1_40          UNIT_END + 0x371
// Bytes
#define PLAYER_EXPLORED_ZONES_1_41          UNIT_END + 0x372
// Bytes
#define PLAYER_EXPLORED_ZONES_1_42          UNIT_END + 0x373
// Bytes
#define PLAYER_EXPLORED_ZONES_1_43          UNIT_END + 0x374
// Bytes
#define PLAYER_EXPLORED_ZONES_1_44          UNIT_END + 0x375
// Bytes
#define PLAYER_EXPLORED_ZONES_1_45          UNIT_END + 0x376
// Bytes
#define PLAYER_EXPLORED_ZONES_1_46          UNIT_END + 0x377
// Bytes
#define PLAYER_EXPLORED_ZONES_1_47          UNIT_END + 0x378
// Bytes
#define PLAYER_EXPLORED_ZONES_1_48          UNIT_END + 0x379
// Bytes
#define PLAYER_EXPLORED_ZONES_1_49          UNIT_END + 0x37A
// Bytes
#define PLAYER_EXPLORED_ZONES_1_50          UNIT_END + 0x37B
// Bytes
#define PLAYER_EXPLORED_ZONES_1_51          UNIT_END + 0x37C
// Bytes
#define PLAYER_EXPLORED_ZONES_1_52          UNIT_END + 0x37D
// Bytes
#define PLAYER_EXPLORED_ZONES_1_53          UNIT_END + 0x37E
// Bytes
#define PLAYER_EXPLORED_ZONES_1_54          UNIT_END + 0x37F
// Bytes
#define PLAYER_EXPLORED_ZONES_1_55          UNIT_END + 0x380
// Bytes
#define PLAYER_EXPLORED_ZONES_1_56          UNIT_END + 0x381
// Bytes
#define PLAYER_EXPLORED_ZONES_1_57          UNIT_END + 0x382
// Bytes
#define PLAYER_EXPLORED_ZONES_1_58          UNIT_END + 0x383
// Bytes
#define PLAYER_EXPLORED_ZONES_1_59          UNIT_END + 0x384
// Bytes
#define PLAYER_EXPLORED_ZONES_1_60          UNIT_END + 0x385
// Bytes
#define PLAYER_EXPLORED_ZONES_1_61          UNIT_END + 0x386
// Bytes
#define PLAYER_EXPLORED_ZONES_1_62          UNIT_END + 0x387
// Bytes
#define PLAYER_EXPLORED_ZONES_1_63          UNIT_END + 0x388
// Int32
#define PLAYER_REST_STATE_EXPERIENCE        UNIT_END + 0x389
// Int32
#define PLAYER_FIELD_COINAGE               UNIT_END +  0x38A
// Int32
#define PLAYER_FIELD_POSSTAT0               UNIT_END + 0x38B
// Int32
#define PLAYER_FIELD_POSSTAT1               UNIT_END + 0x38C
// Int32
#define PLAYER_FIELD_POSSTAT2               UNIT_END + 0x38D
// Int32
#define PLAYER_FIELD_POSSTAT3               UNIT_END + 0x38E
// Int32
#define PLAYER_FIELD_POSSTAT4               UNIT_END + 0x38F
// Int32
#define PLAYER_FIELD_NEGSTAT0               UNIT_END + 0x390
// Int32
#define PLAYER_FIELD_NEGSTAT1               UNIT_END + 0x391
// Int32
#define PLAYER_FIELD_NEGSTAT2               UNIT_END + 0x392
// Int32
#define PLAYER_FIELD_NEGSTAT3               UNIT_END + 0x393
// Int32
#define PLAYER_FIELD_NEGSTAT4               UNIT_END + 0x394
// Int32
#define PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE             UNIT_END + 0x395
// Int32
#define PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_01          UNIT_END + 0x396
// Int32
#define PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_02          UNIT_END + 0x397
// Int32
#define PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_03          UNIT_END + 0x398
// Int32
#define PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_04          UNIT_END + 0x399
// Int32
#define PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_05          UNIT_END + 0x39A
// Int32
#define PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_06          UNIT_END + 0x39B
// Int32
#define PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE             UNIT_END + 0x39C
// Int32
#define PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_01          UNIT_END + 0x39D
// Int32
#define PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_02          UNIT_END + 0x39E
// Int32
#define PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_03          UNIT_END + 0x39F
// Int32
#define PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_04          UNIT_END + 0x3A0
// Int32
#define PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_05          UNIT_END + 0x3A1
// Int32
#define PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_06          UNIT_END + 0x3A2
// Int32
#define PLAYER_FIELD_MOD_DAMAGE_DONE_POS                UNIT_END + 0x3A3
// Int32
#define PLAYER_FIELD_MOD_DAMAGE_DONE_POS_01             UNIT_END + 0x3A4
// Int32
#define PLAYER_FIELD_MOD_DAMAGE_DONE_POS_02             UNIT_END + 0x3A5
// Int32
#define PLAYER_FIELD_MOD_DAMAGE_DONE_POS_03             UNIT_END + 0x3A6
// Int32
#define PLAYER_FIELD_MOD_DAMAGE_DONE_POS_04             UNIT_END + 0x3A7
// Int32
#define PLAYER_FIELD_MOD_DAMAGE_DONE_POS_05             UNIT_END + 0x3A8
// Int32
#define PLAYER_FIELD_MOD_DAMAGE_DONE_POS_06             UNIT_END + 0x3A9
// Int32
#define PLAYER_FIELD_MOD_DAMAGE_DONE_NEG                UNIT_END + 0x3AA
// Int32
#define PLAYER_FIELD_MOD_DAMAGE_DONE_NEG_01             UNIT_END + 0x3AB
// Int32
#define PLAYER_FIELD_MOD_DAMAGE_DONE_NEG_02             UNIT_END + 0x3AC
// Int32
#define PLAYER_FIELD_MOD_DAMAGE_DONE_NEG_03             UNIT_END + 0x3AD
// Int32
#define PLAYER_FIELD_MOD_DAMAGE_DONE_NEG_04             UNIT_END + 0x3AE
// Int32
#define PLAYER_FIELD_MOD_DAMAGE_DONE_NEG_05             UNIT_END + 0x3AF
// Int32
#define PLAYER_FIELD_MOD_DAMAGE_DONE_NEG_06             UNIT_END + 0x3B0
// Int32
#define PLAYER_FIELD_MOD_DAMAGE_DONE_PCT                UNIT_END + 0x3B1
// Int32
#define PLAYER_FIELD_MOD_DAMAGE_DONE_PCT_01             UNIT_END + 0x3B2
// Int32
#define PLAYER_FIELD_MOD_DAMAGE_DONE_PCT_02             UNIT_END + 0x3B3
// Int32
#define PLAYER_FIELD_MOD_DAMAGE_DONE_PCT_03             UNIT_END + 0x3B4
// Int32
#define PLAYER_FIELD_MOD_DAMAGE_DONE_PCT_04             UNIT_END + 0x3B5
// Int32
#define PLAYER_FIELD_MOD_DAMAGE_DONE_PCT_05             UNIT_END + 0x3B6
// Int32
#define PLAYER_FIELD_MOD_DAMAGE_DONE_PCT_06             UNIT_END + 0x3B7
// Bytes
#define PLAYER_FIELD_BYTES                  UNIT_END + 0x3B8
// Int32
#define PLAYER_AMMO_ID                      UNIT_END + 0x3B9
// Int32
#define PLAYER_SELF_RES_SPELL               UNIT_END + 0x3BA
// Int32
#define PLAYER_FIELD_PVP_MEDALS             UNIT_END + 0x3BB
// Int32
#define PLAYER_FIELD_BUYBACK_ITEM_ID        UNIT_END + 0x3BC
// Int32
#define PLAYER_FIELD_BUYBACK_RANDOM_PROPERTIES_ID UNIT_END + 0x3BD
// Int32
#define PLAYER_FIELD_BUYBACK_SEED           UNIT_END + 0x3BE
// Int32
#define PLAYER_FIELD_BUYBACK_PRICE          UNIT_END + 0x3BF
// Int32
#define PLAYER_FIELD_BUYBACK_DURABILITY     UNIT_END + 0x3C0
// Int32
#define PLAYER_FIELD_BUYBACK_COUNT          UNIT_END + 0x3C1
// Chars?
#define PLAYER_FIELD_SESSION_KILLS          UNIT_END + 0x3C2
// Chars?
#define PLAYER_FIELD_YESTERDAY_KILLS        UNIT_END + 0x3C3
// Chars?
#define PLAYER_FIELD_LAST_WEEK_KILLS        UNIT_END + 0x3C4
// Chars?
#define PLAYER_FIELD_THIS_WEEK_KILLS        UNIT_END + 0x3C5
// Int32
#define PLAYER_FIELD_THIS_WEEK_CONTRIBUTION UNIT_END + 0x3C6
// Int32
#define PLAYER_FIELD_LIFETIME_HONORBALE_KILLS UNIT_END + 0x3C7
// Int32
#define PLAYER_FIELD_LIFETIME_DISHONORBALE_KILLS UNIT_END + 0x3C8
// Int32
#define PLAYER_FIELD_YESTERDAY_CONTRIBUTION UNIT_END + 0x3C9
// Int32
#define PLAYER_FIELD_LAST_WEEK_CONTRIBUTION UNIT_END + 0x3CA
// Int32
#define PLAYER_FIELD_LAST_WEEK_RANK         UNIT_END + 0x3CB
// Bytes
#define PLAYER_FIELD_BYTES2                 UNIT_END + 0x3CC
// Int32
#define PLAYER_FIELD_PADDING                UNIT_END + 0x3CD
#define PLAYER_END                                          UNIT_END + 0x3CE

// Object
// GUID
#define OBJECT_FIELD_CREATED_BY             OBJECT_END + 0x000
// GUID
#define OBJECT_FIELD_CREATED_BY_01          OBJECT_END + 0x001

// Gameobject
// Int32
#define GAMEOBJECT_DISPLAYID                OBJECT_END + 0x002
// Int32
#define GAMEOBJECT_FLAGS                    OBJECT_END + 0x003
// Float
#define GAMEOBJECT_ROTATION             OBJECT_END + 0x004
// Float
#define GAMEOBJECT_ROTATION_01          OBJECT_END + 0x005
// Float
#define GAMEOBJECT_ROTATION_02          OBJECT_END + 0x006
// Float
#define GAMEOBJECT_ROTATION_03          OBJECT_END + 0x007
// Int32
#define GAMEOBJECT_STATE                    OBJECT_END + 0x008
// Int32
#define GAMEOBJECT_TIMESTAMP                OBJECT_END + 0x009
// Float
#define GAMEOBJECT_POS_X                    OBJECT_END + 0x00A
// Float
#define GAMEOBJECT_POS_Y                    OBJECT_END + 0x00B
// Float
#define GAMEOBJECT_POS_Z                    OBJECT_END + 0x00C
// Float
#define GAMEOBJECT_FACING                   OBJECT_END + 0x00D
// Int32
#define GAMEOBJECT_DYN_FLAGS                OBJECT_END + 0x00E
// Int32
#define GAMEOBJECT_FACTION                  OBJECT_END + 0x00F
// Int32
#define GAMEOBJECT_TYPE_ID                  OBJECT_END + 0x010
// Int32
#define GAMEOBJECT_LEVEL                    OBJECT_END + 0x011
#define GAMEOBJECT_END                                OBJECT_END + 0x012

// Dynamicobject
// GUID
#define DYNAMICOBJECT_CASTER                OBJECT_END + 0x000
// GUID
#define DYNAMICOBJECT_CASTER_01             OBJECT_END + 0x001
// Bytes
#define DYNAMICOBJECT_BYTES                 OBJECT_END + 0x002
// Int32
#define DYNAMICOBJECT_SPELLID               OBJECT_END + 0x003
// Float
#define DYNAMICOBJECT_RADIUS                OBJECT_END + 0x004
// Float
#define DYNAMICOBJECT_POS_X                 OBJECT_END + 0x005
// Float
#define DYNAMICOBJECT_POS_Y                 OBJECT_END + 0x006
// Float
#define DYNAMICOBJECT_POS_Z                 OBJECT_END + 0x007
// Float
#define DYNAMICOBJECT_FACING                OBJECT_END + 0x008
// Bytes
#define DYNAMICOBJECT_PAD                   OBJECT_END + 0x009
#define DYNAMICOBJECT_END                             OBJECT_END + 0x00A

// Corpse
// GUID
#define CORPSE_FIELD_OWNER              OBJECT_END + 0x000
// GUID
#define CORPSE_FIELD_OWNER_01           OBJECT_END + 0x001
// Float
#define CORPSE_FIELD_FACING                 OBJECT_END + 0x002
// Float
#define CORPSE_FIELD_POS_X                  OBJECT_END + 0x003
// Float
#define CORPSE_FIELD_POS_Y                  OBJECT_END + 0x004
// Float
#define CORPSE_FIELD_POS_Z                  OBJECT_END + 0x005
// Int32
#define CORPSE_FIELD_DISPLAY_ID             OBJECT_END + 0x006
// Int32
#define CORPSE_FIELD_ITEM               OBJECT_END + 0x007
// Int32
#define CORPSE_FIELD_ITEM_01            OBJECT_END + 0x008
// Int32
#define CORPSE_FIELD_ITEM_02            OBJECT_END + 0x009
// Int32
#define CORPSE_FIELD_ITEM_03            OBJECT_END + 0x00A
// Int32
#define CORPSE_FIELD_ITEM_04            OBJECT_END + 0x00B
// Int32
#define CORPSE_FIELD_ITEM_05            OBJECT_END + 0x00C
// Int32
#define CORPSE_FIELD_ITEM_06            OBJECT_END + 0x00D
// Int32
#define CORPSE_FIELD_ITEM_07            OBJECT_END + 0x00E
// Int32
#define CORPSE_FIELD_ITEM_08            OBJECT_END + 0x00F
// Int32
#define CORPSE_FIELD_ITEM_09            OBJECT_END + 0x010
// Int32
#define CORPSE_FIELD_ITEM_10            OBJECT_END + 0x011
// Int32
#define CORPSE_FIELD_ITEM_11            OBJECT_END + 0x012
// Int32
#define CORPSE_FIELD_ITEM_12            OBJECT_END + 0x013
// Int32
#define CORPSE_FIELD_ITEM_13            OBJECT_END + 0x014
// Int32
#define CORPSE_FIELD_ITEM_14            OBJECT_END + 0x015
// Int32
#define CORPSE_FIELD_ITEM_15            OBJECT_END + 0x016
// Int32
#define CORPSE_FIELD_ITEM_16            OBJECT_END + 0x017
// Int32
#define CORPSE_FIELD_ITEM_17            OBJECT_END + 0x018
// Int32
#define CORPSE_FIELD_ITEM_18            OBJECT_END + 0x019
// Bytes
#define CORPSE_FIELD_BYTES_1                OBJECT_END + 0x01A
// Bytes
#define CORPSE_FIELD_BYTES_2                OBJECT_END + 0x01B
// Int32
#define CORPSE_FIELD_GUILD                  OBJECT_END + 0x01C
// Int32
#define CORPSE_FIELD_FLAGS                  OBJECT_END + 0x01D
// Int32
#define CORPSE_FIELD_DYNAMIC_FLAGS          OBJECT_END + 0x01E
// Int32
#define CORPSE_FIELD_PAD                    OBJECT_END + 0x01F
#define CORPSE_END                                      OBJECT_END + 0x020
