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

#ifndef MANGOS_SHAREDDEFINES_H
#define MANGOS_SHAREDDEFINES_H

#define GENDER_MALE                         0
#define GENDER_FEMALE                       1
#define GENDER_NONE                         2

// Race value is index in ChrRaces.dbc
enum Races
{
    RACE_HUMAN          = 1,
    RACE_ORC            = 2,
    RACE_DWARF          = 3,
    RACE_NIGHTELF       = 4,
    // if it needs be to official, it's actually SCOURGE acording to the story/.dbc
    RACE_UNDEAD_PLAYER  = 5,
    RACE_TAUREN         = 6,
    RACE_GNOME          = 7,
    RACE_TROLL          = 8,
    MAX_RACES           = 9,
    // officialy, this exists but was never taken into use.. neutral faction which could
    // learn some skills/spells from horde/alliance. maybe it'll be of some use later on.
    RACE_GOBLIN = 9
};

// Class value is index in ChrClasses.dbc
enum Classes
{
    CLASS_WARRIOR   = 1,
    CLASS_PALADIN   = 2,
    CLASS_HUNTER    = 3,
    CLASS_ROGUE     = 4,
    CLASS_PRIEST    = 5,
    // CLASS_UNK1   = 6, unused
    CLASS_SHAMAN    = 7,
    CLASS_MAGE      = 8,
    CLASS_WARLOCK   = 9,
    // CLASS_UNK2   = 10,unused
    CLASS_DRUID     = 11,
    MAX_CLASSES     = 12
};

enum ReputationRank
{
    REP_HATED       = 0,
    REP_HOSTILE     = 1,
    REP_UNFRIENDLY  = 2,
    REP_NEUTRAL     = 3,
    REP_FRIENDLY    = 4,
    REP_HONORED     = 5,
    REP_REVERTED    = 6,
    REP_EXALTED     = 7
};

#define MIN_REPUTATION_RANK (REP_HATED)
#define MAX_REPUTATION_RANK 8

enum MapTypes
{
    MAP_COMMON          = 0,
    MAP_INSTANCE        = 1,
    MAP_RAID            = 2,
    MAP_BATTLEGROUND    = 3,
};

enum MoneyConstants
{
    COPPER = 1,
    SILVER = COPPER*100,
    GOLD   = SILVER*100
};

enum Stats
{
    STAT_STRENGTH                      = 0,
    STAT_AGILITY                       = 1,
    STAT_STAMINA                       = 2,
    STAT_INTELLECT                     = 3,
    STAT_SPIRIT                        = 4
};

#define MAX_STATS                        5

enum Powers
{
    POWER_MANA                         = 0,
    POWER_RAGE                         = 1,
    POWER_FOCUS                        = 2,
    POWER_ENERGY                       = 3,
    POWER_HAPPINESS                    = 4
};

#define MAX_POWERS                       5

enum SpellSchools
{
    SPELL_SCHOOL_NORMAL                = 0,
    SPELL_SCHOOL_HOLY                  = 1,
    SPELL_SCHOOL_FIRE                  = 2,
    SPELL_SCHOOL_NATURE                = 3,
    SPELL_SCHOOL_FROST                 = 4,
    SPELL_SCHOOL_SHADOW                = 5,
    SPELL_SCHOOL_ARCANE                = 6
};

#define MAX_SPELL_SCHOOL                 7

#define ITEM_QUALITY_POOR                   0               //GREY
#define ITEM_QUALITY_NORMAL                 1               //WHITE
#define ITEM_QUALITY_UNCOMMON               2               //GREEN
#define ITEM_QUALITY_RARE                   3               //BLUE
#define ITEM_QUALITY_EPIC                   4               //PURPLE
#define ITEM_QUALITY_LEGENDARY              5               //ORANGE
#define ITEM_QUALITY_ARTIFACT               6               //LIGHT YELLOW

#define SHEATHETYPE_NONE                    0
#define SHEATHETYPE_MAINHAND                1
#define SHEATHETYPE_OFFHAND                 2
#define SHEATHETYPE_LARGEWEAPONLEFT         3
#define SHEATHETYPE_LARGEWEAPONRIGHT        4
#define SHEATHETYPE_HIPWEAPONLEFT           5
#define SHEATHETYPE_HIPWEAPONRIGHT          6
#define SHEATHETYPE_SHIELD                  7

#define SLOT_HEAD                           0
#define SLOT_NECK                           1
#define SLOT_SHOULDERS                      2
#define SLOT_SHIRT                          3
#define SLOT_CHEST                          4
#define SLOT_WAIST                          5
#define SLOT_LEGS                           6
#define SLOT_FEET                           7
#define SLOT_WRISTS                         8
#define SLOT_HANDS                          9
#define SLOT_FINGER1                        10
#define SLOT_FINGER2                        11
#define SLOT_TRINKET1                       12
#define SLOT_TRINKET2                       13
#define SLOT_BACK                           14
#define SLOT_MAIN_HAND                      15
#define SLOT_OFF_HAND                       16
#define SLOT_RANGED                         17
#define SLOT_TABARD                         18
#define SLOT_EMPTY                          19

enum Language
{
    LANG_GLOBAL         = 0, LANG_UNIVERSAL      = 0,
    LANG_ORCISH         = 1,
    LANG_DARNASSIAN     = 2,
    LANG_TAURAHE        = 3,
    LANG_DWARVISH       = 6,
    LANG_COMMON         = 7,
    LANG_DEMONIC        = 8,
    LANG_TITAN          = 9,
    LANG_THELASSIAN     = 10,
    LANG_DRACONIC       = 11,
    LANG_KALIMAG        = 12,
    LANG_GNOMISH        = 13,
    LANG_TROLL          = 14,
    LANG_GUTTERSPEAK    = 33
};

#define LANGUAGES_COUNT 15

enum Team
{
    ALLIANCE = 469,
    HORDE = 67,
    ALLIANCE_FORCES = 891,
    HORDE_FORCES = 892,
    STEAMWHEEDLE_CARTEL = 169,
};

// Spell Effects

#define SPELL_EFFECT_INSTAKILL                  1
#define SPELL_EFFECT_SCHOOL_DAMAGE              2
#define SPELL_EFFECT_DUMMY                      3
#define SPELL_EFFECT_PORTAL_TELEPORT            4
#define SPELL_EFFECT_TELEPORT_UNITS             5
#define SPELL_EFFECT_APPLY_AURA                 6
#define SPELL_EFFECT_ENVIRONMENTAL_DAMAGE       7
#define SPELL_EFFECT_MANA_DRAIN                 8
#define SPELL_EFFECT_HEALTH_LEECH               9
#define SPELL_EFFECT_HEAL                       10
#define SPELL_EFFECT_BIND                       11
#define SPELL_EFFECT_PORTAL                     12
#define SPELL_EFFECT_RITUAL_BASE                13
#define SPELL_EFFECT_RITUAL_SPECIALIZE          14
#define SPELL_EFFECT_RITUAL_ACTIVATE_PORTAL     15
#define SPELL_EFFECT_QUEST_COMPLETE             16
#define SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL     17
#define SPELL_EFFECT_RESURRECT                  18
#define SPELL_EFFECT_ADD_EXTRA_ATTACKS          19
#define SPELL_EFFECT_DODGE                      20
#define SPELL_EFFECT_EVADE                      21
#define SPELL_EFFECT_PARRY                      22
#define SPELL_EFFECT_BLOCK                      23
#define SPELL_EFFECT_CREATE_ITEM                24
#define SPELL_EFFECT_WEAPON                     25
#define SPELL_EFFECT_DEFENSE                    26
#define SPELL_EFFECT_PERSISTENT_AREA_AURA       27
#define SPELL_EFFECT_SUMMON                     28
#define SPELL_EFFECT_LEAP                       29
#define SPELL_EFFECT_ENERGIZE                   30
#define SPELL_EFFECT_WEAPON_PERCENT_DAMAGE      31
#define SPELL_EFFECT_TRIGGER_MISSILE            32
#define SPELL_EFFECT_OPEN_LOCK                  33
#define SPELL_EFFECT_SUMMON_CHANGE_ITEM         34
#define SPELL_EFFECT_APPLY_AREA_AURA            35
#define SPELL_EFFECT_LEARN_SPELL                36
#define SPELL_EFFECT_SPELL_DEFENSE              37
#define SPELL_EFFECT_DISPEL                     38
#define SPELL_EFFECT_LANGUAGE                   39
#define SPELL_EFFECT_DUAL_WIELD                 40
#define SPELL_EFFECT_SUMMON_WILD                41
#define SPELL_EFFECT_SUMMON_GUARDIAN            42
#define SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER 43
#define SPELL_EFFECT_SKILL_STEP                 44
#define SPELL_EFFECT_UNDEFINED_45               45
#define SPELL_EFFECT_SPAWN                      46
#define SPELL_EFFECT_TRADE_SKILL                47
#define SPELL_EFFECT_STEALTH                    48
#define SPELL_EFFECT_DETECT                     49
//#define SPELL_EFFECT_SUMMON_OBJECT            50
#define SPELL_EFFECT_TRANS_DOOR                 50
#define SPELL_EFFECT_FORCE_CRITICAL_HIT         51
#define SPELL_EFFECT_GUARANTEE_HIT              52
#define SPELL_EFFECT_ENCHANT_ITEM               53
#define SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY     54
#define SPELL_EFFECT_TAMECREATURE               55
#define SPELL_EFFECT_SUMMON_PET                 56
#define SPELL_EFFECT_LEARN_PET_SPELL            57
#define SPELL_EFFECT_WEAPON_DAMAGE              58
#define SPELL_EFFECT_OPEN_LOCK_ITEM             59
#define SPELL_EFFECT_PROFICIENCY                60
#define SPELL_EFFECT_SEND_EVENT                 61
#define SPELL_EFFECT_POWER_BURN                 62
#define SPELL_EFFECT_THREAT                     63
#define SPELL_EFFECT_TRIGGER_SPELL              64
#define SPELL_EFFECT_HEALTH_FUNNEL              65
#define SPELL_EFFECT_POWER_FUNNEL               66
#define SPELL_EFFECT_HEAL_MAX_HEALTH            67
#define SPELL_EFFECT_INTERRUPT_CAST             68
#define SPELL_EFFECT_DISTRACT                   69
#define SPELL_EFFECT_PULL                       70
#define SPELL_EFFECT_PICKPOCKET                 71
#define SPELL_EFFECT_ADD_FARSIGHT               72
#define SPELL_EFFECT_SUMMON_POSSESSED           73
#define SPELL_EFFECT_SUMMON_TOTEM               74
#define SPELL_EFFECT_HEAL_MECHANICAL            75
#define SPELL_EFFECT_SUMMON_OBJECT_WILD         76
#define SPELL_EFFECT_SCRIPT_EFFECT              77
#define SPELL_EFFECT_ATTACK                     78
#define SPELL_EFFECT_SANCTUARY                  79
#define SPELL_EFFECT_ADD_COMBO_POINTS           80
#define SPELL_EFFECT_CREATE_HOUSE               81
#define SPELL_EFFECT_BIND_SIGHT                 82
#define SPELL_EFFECT_DUEL                       83
#define SPELL_EFFECT_STUCK                      84
#define SPELL_EFFECT_SUMMON_PLAYER              85
#define SPELL_EFFECT_ACTIVATE_OBJECT            86
#define SPELL_EFFECT_SUMMON_TOTEM_SLOT1         87
#define SPELL_EFFECT_SUMMON_TOTEM_SLOT2         88
#define SPELL_EFFECT_SUMMON_TOTEM_SLOT3         89
#define SPELL_EFFECT_SUMMON_TOTEM_SLOT4         90
#define SPELL_EFFECT_THREAT_ALL                 91
#define SPELL_EFFECT_ENCHANT_HELD_ITEM          92
#define SPELL_EFFECT_SUMMON_PHANTASM            93
#define SPELL_EFFECT_SELF_RESURRECT             94
#define SPELL_EFFECT_SKINNING                   95
#define SPELL_EFFECT_CHARGE                     96
#define SPELL_EFFECT_SUMMON_CRITTER             97
#define SPELL_EFFECT_KNOCK_BACK                 98
#define SPELL_EFFECT_DISENCHANT                 99
#define SPELL_EFFECT_INEBRIATE                  100
#define SPELL_EFFECT_FEED_PET                   101
#define SPELL_EFFECT_DISMISS_PET                102
#define SPELL_EFFECT_REPUTATION                 103
#define SPELL_EFFECT_SUMMON_OBJECT_SLOT1        104
#define SPELL_EFFECT_SUMMON_OBJECT_SLOT2        105
#define SPELL_EFFECT_SUMMON_OBJECT_SLOT3        106
#define SPELL_EFFECT_SUMMON_OBJECT_SLOT4        107
#define SPELL_EFFECT_DISPEL_MECHANIC            108
#define SPELL_EFFECT_SUMMON_DEAD_PET            109
#define SPELL_EFFECT_DESTROY_ALL_TOTEMS         110
#define SPELL_EFFECT_DURABILITY_DAMAGE          111
#define SPELL_EFFECT_SUMMON_DEMON               112
#define SPELL_EFFECT_RESURRECT_NEW              113
#define SPELL_EFFECT_ATTACK_ME                  114
#define SPELL_EFFECT_DURABILITY_DAMAGE_PCT      115
#define SPELL_EFFECT_SKIN_PLAYER_CORPSE         116
#define SPELL_EFFECT_SPIRIT_HEAL                117
#define SPELL_EFFECT_SKILL                      118
#define SPELL_EFFECT_APPLY_AURA_NEW             119
#define SPELL_EFFECT_TELEPORT_GRAVEYARD         120
#define SPELL_EFFECT_NORMALIZED_WEAPON_DMG      121
#define TOTAL_SPELL_EFFECTS                     122

#define STATE_STANDING                          0
#define STATE_SITTING                           1
#define STATE_SITTINGCHAIR                      2
#define STATE_SLEEPING                          3
#define STATE_SITTINGCHAIRLOW                   4
#define STATE_SITTINGCHAIRMEDIUM                5
#define STATE_SITTINGCHAIRHIGH                  6
#define STATE_DEAD                              7
#define STATE_KNEEL                             8

#define GAMEOBJECT_TYPE_DOOR            0
#define GAMEOBJECT_TYPE_BUTTON          1
#define GAMEOBJECT_TYPE_QUESTGIVER      2
#define GAMEOBJECT_TYPE_CHEST           3
#define GAMEOBJECT_TYPE_BINDER          4
#define GAMEOBJECT_TYPE_GENERIC         5
#define GAMEOBJECT_TYPE_TRAP            6
#define GAMEOBJECT_TYPE_CHAIR           7
#define GAMEOBJECT_TYPE_SPELL_FOCUS     8
#define GAMEOBJECT_TYPE_TEXT            9
#define GAMEOBJECT_TYPE_GOOBER          10
#define GAMEOBJECT_TYPE_TRANSPORT       11
#define GAMEOBJECT_TYPE_AREADAMAGE      12
#define GAMEOBJECT_TYPE_CAMERA          13
#define GAMEOBJECT_TYPE_MAP_OBJECT      14
#define GAMEOBJECT_TYPE_MO_TRANSPORT    15
#define GAMEOBJECT_TYPE_DUEL_ARBITER    16
#define GAMEOBJECT_TYPE_FISHINGNODE     17
#define GAMEOBJECT_TYPE_RITUAL          18
#define GAMEOBJECT_TYPE_MAILBOX         19
#define GAMEOBJECT_TYPE_AUCTIONHOUSE    20
#define GAMEOBJECT_TYPE_GUARDPOST       21
#define GAMEOBJECT_TYPE_SPELLCASTER     22
#define GAMEOBJECT_TYPE_MEETINGSTONE    23
#define GAMEOBJECT_TYPE_FLAGSTAND       24
#define GAMEOBJECT_TYPE_FISHINGHOLE     25
#define GAMEOBJECT_TYPE_FLAGDROP        26
// Custom gametypes, can create problems at sending to client
#define GAMEOBJECT_TYPE_CUSTOM_TELEPORTER      27

#define TEXTEMOTE_AGREE                 1
#define TEXTEMOTE_AMAZE                 2
#define TEXTEMOTE_ANGRY                 3
#define TEXTEMOTE_APOLOGIZE             4
#define TEXTEMOTE_APPLAUD               5
#define TEXTEMOTE_BASHFUL               6
#define TEXTEMOTE_BECKON                7
#define TEXTEMOTE_BEG                   8
#define TEXTEMOTE_BITE                  9
#define TEXTEMOTE_BLEED                 10
#define TEXTEMOTE_BLINK                 11
#define TEXTEMOTE_BLUSH                 12
#define TEXTEMOTE_BONK                  13
#define TEXTEMOTE_BORED                 14
#define TEXTEMOTE_BOUNCE                15
#define TEXTEMOTE_BRB                   16
#define TEXTEMOTE_BOW                   17
#define TEXTEMOTE_BURP                  18
#define TEXTEMOTE_BYE                   19
#define TEXTEMOTE_CACKLE                20
#define TEXTEMOTE_CHEER                 21
#define TEXTEMOTE_CHICKEN               22
#define TEXTEMOTE_CHUCKLE               23
#define TEXTEMOTE_CLAP                  24
#define TEXTEMOTE_CONFUSED              25
#define TEXTEMOTE_CONGRATULATE          26
#define TEXTEMOTE_COUGH                 27
#define TEXTEMOTE_COWER                 28
#define TEXTEMOTE_CRACK                 29
#define TEXTEMOTE_CRINGE                30
#define TEXTEMOTE_CRY                   31
#define TEXTEMOTE_CURIOUS               32
#define TEXTEMOTE_CURTSEY               33
#define TEXTEMOTE_DANCE                 34
#define TEXTEMOTE_DRINK                 35
#define TEXTEMOTE_DROOL                 36
#define TEXTEMOTE_EAT                   37
#define TEXTEMOTE_EYE                   38
#define TEXTEMOTE_FART                  39
#define TEXTEMOTE_FIDGET                40
#define TEXTEMOTE_FLEX                  41
#define TEXTEMOTE_FROWN                 42
#define TEXTEMOTE_GASP                  43
#define TEXTEMOTE_GAZE                  44
#define TEXTEMOTE_GIGGLE                45
#define TEXTEMOTE_GLARE                 46
#define TEXTEMOTE_GLOAT                 47
#define TEXTEMOTE_GREET                 48
#define TEXTEMOTE_GRIN                  49
#define TEXTEMOTE_GROAN                 50
#define TEXTEMOTE_GROVEL                51
#define TEXTEMOTE_GUFFAW                52
#define TEXTEMOTE_HAIL                  53
#define TEXTEMOTE_HAPPY                 54
#define TEXTEMOTE_HELLO                 55
#define TEXTEMOTE_HUG                   56
#define TEXTEMOTE_HUNGRY                57
#define TEXTEMOTE_KISS                  58
#define TEXTEMOTE_KNEEL                 59
#define TEXTEMOTE_LAUGH                 60
#define TEXTEMOTE_LAYDOWN               61
#define TEXTEMOTE_MESSAGE               62
#define TEXTEMOTE_MOAN                  63
#define TEXTEMOTE_MOON                  64
#define TEXTEMOTE_MOURN                 65
#define TEXTEMOTE_NO                    66
#define TEXTEMOTE_NOD                   67
#define TEXTEMOTE_NOSEPICK              68
#define TEXTEMOTE_PANIC                 69
#define TEXTEMOTE_PEER                  70
#define TEXTEMOTE_PLEAD                 71
#define TEXTEMOTE_POINT                 72
#define TEXTEMOTE_POKE                  73
#define TEXTEMOTE_PRAY                  74
#define TEXTEMOTE_ROAR                  75
#define TEXTEMOTE_ROFL                  76
#define TEXTEMOTE_RUDE                  77
#define TEXTEMOTE_SALUTE                78
#define TEXTEMOTE_SCRATCH               79
#define TEXTEMOTE_SEXY                  80
#define TEXTEMOTE_SHAKE                 81
#define TEXTEMOTE_SHOUT                 82
#define TEXTEMOTE_SHRUG                 83
#define TEXTEMOTE_SHY                   84
#define TEXTEMOTE_SIGH                  85
#define TEXTEMOTE_SIT                   86
#define TEXTEMOTE_SLEEP                 87
#define TEXTEMOTE_SNARL                 88
#define TEXTEMOTE_SPIT                  89
#define TEXTEMOTE_STARE                 90
#define TEXTEMOTE_SURPRISED             91
#define TEXTEMOTE_SURRENDER             92
#define TEXTEMOTE_TALK                  93
#define TEXTEMOTE_TALKEX                94
#define TEXTEMOTE_TALKQ                 95
#define TEXTEMOTE_TAP                   96
#define TEXTEMOTE_THANK                 97
#define TEXTEMOTE_THREATEN              98
#define TEXTEMOTE_TIRED                 99
#define TEXTEMOTE_VICTORY               100
#define TEXTEMOTE_WAVE                  101
#define TEXTEMOTE_WELCOME               102
#define TEXTEMOTE_WHINE                 103
#define TEXTEMOTE_WHISTLE               104
#define TEXTEMOTE_WORK                  105
#define TEXTEMOTE_YAWN                  106
#define TEXTEMOTE_BOGGLE                107
#define TEXTEMOTE_CALM                  108
#define TEXTEMOTE_COLD                  109
#define TEXTEMOTE_COMFORT               110
#define TEXTEMOTE_CUDDLE                111
#define TEXTEMOTE_DUCK                  112
#define TEXTEMOTE_INSULT                113
#define TEXTEMOTE_INTRODUCE             114
#define TEXTEMOTE_JK                    115
#define TEXTEMOTE_LICK                  116
#define TEXTEMOTE_LISTEN                117
#define TEXTEMOTE_LOST                  118
#define TEXTEMOTE_MOCK                  119
#define TEXTEMOTE_PONDER                120
#define TEXTEMOTE_POUNCE                121
#define TEXTEMOTE_PRAISE                122
#define TEXTEMOTE_PURR                  123
#define TEXTEMOTE_PUZZLE                124
#define TEXTEMOTE_RAISE                 125
#define TEXTEMOTE_READY                 126
#define TEXTEMOTE_SHIMMY                127
#define TEXTEMOTE_SHIVER                128
#define TEXTEMOTE_SHOO                  129
#define TEXTEMOTE_SLAP                  130
#define TEXTEMOTE_SMIRK                 131
#define TEXTEMOTE_SNIFF                 132
#define TEXTEMOTE_SNUB                  133
#define TEXTEMOTE_SOOTHE                134
#define TEXTEMOTE_STINK                 135
#define TEXTEMOTE_TAUNT                 136
#define TEXTEMOTE_TEASE                 137
#define TEXTEMOTE_THIRSTY               138
#define TEXTEMOTE_VETO                  139
#define TEXTEMOTE_SNICKER               140
#define TEXTEMOTE_STAND                 141
#define TEXTEMOTE_TICKLE                142
#define TEXTEMOTE_VIOLIN                143
#define TEXTEMOTE_SMILE                 163
#define TEXTEMOTE_RASP                  183
#define TEXTEMOTE_PITY                  203
#define TEXTEMOTE_GROWL                 204
#define TEXTEMOTE_BARK                  205
#define TEXTEMOTE_SCARED                223
#define TEXTEMOTE_FLOP                  224
#define TEXTEMOTE_LOVE                  225
#define TEXTEMOTE_MOO                   226
#define TEXTEMOTE_COMMEND               243
#define TEXTEMOTE_JOKE                  329

#define EMOTE_ONESHOT_NONE                  0
#define EMOTE_ONESHOT_TALK                  1
#define EMOTE_ONESHOT_BOW                   2
#define EMOTE_ONESHOT_WAVE                  3
#define EMOTE_ONESHOT_CHEER                 4
#define EMOTE_ONESHOT_EXCLAMATION           5
#define EMOTE_ONESHOT_QUESTION              6
#define EMOTE_ONESHOT_EAT                   7
#define EMOTE_STATE_DANCE                   10
#define EMOTE_ONESHOT_LAUGH                 11
#define EMOTE_STATE_SLEEP                   12
#define EMOTE_STATE_SIT                     13
#define EMOTE_ONESHOT_RUDE                  14
#define EMOTE_ONESHOT_ROAR                  15
#define EMOTE_ONESHOT_KNEEL                 16
#define EMOTE_ONESHOT_KISS                  17
#define EMOTE_ONESHOT_CRY                   18
#define EMOTE_ONESHOT_CHICKEN               19
#define EMOTE_ONESHOT_BEG                   20
#define EMOTE_ONESHOT_APPLAUD               21
#define EMOTE_ONESHOT_SHOUT                 22
#define EMOTE_ONESHOT_FLEX                  23
#define EMOTE_ONESHOT_SHY                   24
#define EMOTE_ONESHOT_POINT                 25
#define EMOTE_STATE_STAND                   26
#define EMOTE_STATE_READYUNARMED            27
#define EMOTE_STATE_WORK                    28
#define EMOTE_STATE_POINT                   29
#define EMOTE_STATE_NONE                    30
#define EMOTE_ONESHOT_WOUND                 33
#define EMOTE_ONESHOT_WOUNDCRITICAL         34
#define EMOTE_ONESHOT_ATTACKUNARMED         35
#define EMOTE_ONESHOT_ATTACK1H              36
#define EMOTE_ONESHOT_ATTACK2HTIGHT         37
#define EMOTE_ONESHOT_ATTACK2HLOOSE         38
#define EMOTE_ONESHOT_PARRYUNARMED          39
#define EMOTE_ONESHOT_PARRYSHIELD           43
#define EMOTE_ONESHOT_READYUNARMED          44
#define EMOTE_ONESHOT_READY1H               45
#define EMOTE_ONESHOT_READYBOW              48
#define EMOTE_ONESHOT_SPELLPRECAST          50
#define EMOTE_ONESHOT_SPELLCAST             51
#define EMOTE_ONESHOT_BATTLEROAR            53
#define EMOTE_ONESHOT_SPECIALATTACK1H       54
#define EMOTE_ONESHOT_KICK                  60
#define EMOTE_ONESHOT_ATTACKTHROWN          61
#define EMOTE_STATE_STUN                    64
#define EMOTE_STATE_DEAD                    65
#define EMOTE_ONESHOT_SALUTE                66
#define EMOTE_STATE_KNEEL                   68
#define EMOTE_STATE_USESTANDING             69
#define EMOTE_ONESHOT_WAVE_NOSHEATHE        70
#define EMOTE_ONESHOT_CHEER_NOSHEATHE       71
#define EMOTE_ONESHOT_EAT_NOSHEATHE         92
#define EMOTE_STATE_STUN_NOSHEATHE          93
#define EMOTE_ONESHOT_DANCE                 94
#define EMOTE_ONESHOT_SALUTE_NOSHEATH       113
#define EMOTE_STATE_USESTANDING_NOSHEATHE   133
#define EMOTE_ONESHOT_LAUGH_NOSHEATHE       153
#define EMOTE_STATE_WORK_NOSHEATHE          173
#define EMOTE_STATE_SPELLPRECAST            193
#define EMOTE_ONESHOT_READYRIFLE            213
#define EMOTE_STATE_READYRIFLE              214
#define EMOTE_STATE_WORK_NOSHEATHE_MINING   233
#define EMOTE_STATE_WORK_NOSHEATHE_CHOPWOOD 234
#define EMOTE_zzOLDONESHOT_LIFTOFF          253
#define EMOTE_ONESHOT_LIFTOFF               254
#define EMOTE_ONESHOT_YES                   273
#define EMOTE_ONESHOT_NO                    274
#define EMOTE_ONESHOT_TRAIN                 275
#define EMOTE_ONESHOT_LAND                  293
#define EMOTE_STATE_READY1H                 333
#define EMOTE_STATE_AT_EASE                 313
#define EMOTE_STATE_SPELLKNEELSTART         353
#define EMOTE_STATE_SUBMERGED               373
#define EMOTE_ONESHOT_SUBMERGE              374

#define ANIM_STAND                      0x0
#define ANIM_DEATH                      0x1
#define ANIM_SPELL                      0x2
#define ANIM_STOP                       0x3
#define ANIM_WALK                       0x4
#define ANIM_RUN                        0x5
#define ANIM_DEAD                       0x6
#define ANIM_RISE                       0x7
#define ANIM_STANDWOUND                 0x8
#define ANIM_COMBATWOUND                0x9
#define ANIM_COMBATCRITICAL             0xA
#define ANIM_SHUFFLE_LEFT               0xB
#define ANIM_SHUFFLE_RIGHT              0xC
#define ANIM_WALK_BACKWARDS             0xD
#define ANIM_STUN                       0xE
#define ANIM_HANDS_CLOSED               0xF
#define ANIM_ATTACKUNARMED              0x10
#define ANIM_ATTACK1H                   0x11
#define ANIM_ATTACK2HTIGHT              0x12
#define ANIM_ATTACK2HLOOSE              0x13
#define ANIM_PARRYUNARMED               0x14
#define ANIM_PARRY1H                    0x15
#define ANIM_PARRY2HTIGHT               0x16
#define ANIM_PARRY2HLOOSE               0x17
#define ANIM_PARRYSHIELD                0x18
#define ANIM_READYUNARMED               0x19
#define ANIM_READY1H                    0x1A
#define ANIM_READY2HTIGHT               0x1B
#define ANIM_READY2HLOOSE               0x1C
#define ANIM_READYBOW                   0x1D
#define ANIM_DODGE                      0x1E
#define ANIM_SPELLPRECAST               0x1F
#define ANIM_SPELLCAST                  0x20
#define ANIM_SPELLCASTAREA              0x21
#define ANIM_NPCWELCOME                 0x22
#define ANIM_NPCGOODBYE                 0x23
#define ANIM_BLOCK                      0x24
#define ANIM_JUMPSTART                  0x25
#define ANIM_JUMP                       0x26
#define ANIM_JUMPEND                    0x27
#define ANIM_FALL                       0x28
#define ANIM_SWIMIDLE                   0x29
#define ANIM_SWIM                       0x2A
#define ANIM_SWIM_LEFT                  0x2B
#define ANIM_SWIM_RIGHT                 0x2C
#define ANIM_SWIM_BACKWARDS             0x2D
#define ANIM_ATTACKBOW                  0x2E
#define ANIM_FIREBOW                    0x2F
#define ANIM_READYRIFLE                 0x30
#define ANIM_ATTACKRIFLE                0x31
#define ANIM_LOOT                       0x32
#define ANIM_SPELL_PRECAST_DIRECTED     0x33
#define ANIM_SPELL_PRECAST_OMNI         0x34
#define ANIM_SPELL_CAST_DIRECTED        0x35
#define ANIM_SPELL_CAST_OMNI            0x36
#define ANIM_SPELL_BATTLEROAR           0x37
#define ANIM_SPELL_READYABILITY         0x38
#define ANIM_SPELL_SPECIAL1H            0x39
#define ANIM_SPELL_SPECIAL2H            0x3A
#define ANIM_SPELL_SHIELDBASH           0x3B
#define ANIM_EMOTE_TALK                 0x3C
#define ANIM_EMOTE_EAT                  0x3D
#define ANIM_EMOTE_WORK                 0x3E
#define ANIM_EMOTE_USE_STANDING         0x3F
#define ANIM_EMOTE_EXCLAMATION          0x40
#define ANIM_EMOTE_QUESTION             0x41
#define ANIM_EMOTE_BOW                  0x42
#define ANIM_EMOTE_WAVE                 0x43
#define ANIM_EMOTE_CHEER                0x44
#define ANIM_EMOTE_DANCE                0x45
#define ANIM_EMOTE_LAUGH                0x46
#define ANIM_EMOTE_SLEEP                0x47
#define ANIM_EMOTE_SIT_GROUND           0x48
#define ANIM_EMOTE_RUDE                 0x49
#define ANIM_EMOTE_ROAR                 0x4A
#define ANIM_EMOTE_KNEEL                0x4B
#define ANIM_EMOTE_KISS                 0x4C
#define ANIM_EMOTE_CRY                  0x4D
#define ANIM_EMOTE_CHICKEN              0x4E
#define ANIM_EMOTE_BEG                  0x4F
#define ANIM_EMOTE_APPLAUD              0x50
#define ANIM_EMOTE_SHOUT                0x51
#define ANIM_EMOTE_FLEX                 0x52
#define ANIM_EMOTE_SHY                  0x53
#define ANIM_EMOTE_POINT                0x54
#define ANIM_ATTACK1HPIERCE             0x55
#define ANIM_ATTACK2HLOOSEPIERCE        0x56
#define ANIM_ATTACKOFF                  0x57
#define ANIM_ATTACKOFFPIERCE            0x58
#define ANIM_SHEATHE                    0x59
#define ANIM_HIPSHEATHE                 0x5A
#define ANIM_MOUNT                      0x5B
#define ANIM_RUN_LEANRIGHT              0x5C
#define ANIM_RUN_LEANLEFT               0x5D
#define ANIM_MOUNT_SPECIAL              0x5E
#define ANIM_KICK                       0x5F
#define ANIM_SITDOWN                    0x60
#define ANIM_SITTING                    0x61
#define ANIM_SITUP                      0x62
#define ANIM_SLEEPDOWN                  0x63
#define ANIM_SLEEPING                   0x64
#define ANIM_SLEEPUP                    0x65
#define ANIM_SITCHAIRLOW                0x66
#define ANIM_SITCHAIRMEDIUM             0x67
#define ANIM_SITCHAIRHIGH               0x68
#define ANIM_LOADBOW                    0x69
#define ANIM_LOADRIFLE                  0x6A
#define ANIM_ATTACKTHROWN               0x6B
#define ANIM_READYTHROWN                0x6C
#define ANIM_HOLDBOW                    0x6D
#define ANIM_HOLDRIFLE                  0x6E
#define ANIM_HOLDTHROWN                 0x6F
#define ANIM_LOADTHROWN                 0x70
#define ANIM_EMOTE_SALUTE               0x71
#define ANIM_KNEELDOWN                  0x72
#define ANIM_KNEELING                   0x73
#define ANIM_KNEELUP                    0x74
#define ANIM_ATTACKUNARMEDOFF           0x75
#define ANIM_SPECIALUNARMED             0x76
#define ANIM_STEALTHWALK                0x77
#define ANIM_STEALTHSTAND               0x78
#define ANIM_KNOCKDOWN                  0x79
#define ANIM_EATING                     0x7A
#define ANIM_USESTANDINGLOOP            0x7B
#define ANIM_CHANNELCASTDIRECTED        0x7C
#define ANIM_CHANNELCASTOMNI            0x7D
#define ANIM_WHIRLWIND                  0x7E
#define ANIM_BIRTH                      0x7F
#define ANIM_USESTANDINGSTART           0x80
#define ANIM_USESTANDINGEND             0x81
#define ANIM_HOWL                       0x82
#define ANIM_DROWN                      0x83
#define ANIM_DROWNED                    0x84
#define ANIM_FISHINGCAST                0x85
#define ANIM_FISHINGLOOP                0x86
#define ANIM_FLY                        0x87
#define ANIM_EMOTE_WORK_NO_SHEATHE      0x88
#define ANIM_EMOTE_STUN_NO_SHEATHE      0x89
#define ANIM_EMOTE_USE_STANDING_NO_SHEATHE 0x8A
#define ANIM_SPELL_SLEEP_DOWN           0x8B
#define ANIM_SPELL_KNEEL_START          0x8C
#define ANIM_SPELL_KNEEL_LOOP           0x8D
#define ANIM_SPELL_KNEEL_END            0x8E
#define ANIM_SPRINT                     0x8F
#define ANIM_IN_FIGHT                   0x90

#define FIRST_GAMEOBJECTANIMATION       0x91
#define ANIM_GAMEOBJ_SPAWN              0
#define ANIM_GAMEOBJ_CLOSED             1
#define ANIM_GAMEOBJ_OPEN               2
#define ANIM_GAMEOBJ_OPENED             3
#define ANIM_GAMEOBJ_CLOSE              4
#define ANIM_GAMEOBJ_DESTROY            5
#define ANIM_GAMEOBJ_DESTROYED          6
#define ANIM_GAMEOBJ_REBUILD            7
#define ANIM_GAMEOBJ_CUSTOM0            8
#define ANIM_GAMEOBJ_CUSTOM1            9
#define ANIM_GAMEOBJ_CUSTOM2            10
#define ANIM_GAMEOBJ_CUSTOM3            11

#define FIRST_EFFECTANIMATION           0x93
#define ANIM_EFFECT_STAND               0
#define ANIM_EFFECT_HOLD                1
#define ANIM_EFFECT_DECAY               2

#define FIRST_ITEMANIMATION             0x96
#define ANIM_ITEM_STAND                 0
#define ANIM_ITEM_INFLIGHT              1
#define ANIM_ITEM_BOWPULL               2
#define ANIM_ITEM_BOWRELEASE            3

#define LOCKTYPE_PICKLOCK               1
#define LOCKTYPE_HERBALISM              2
#define LOCKTYPE_MINING                 3
#define LOCKTYPE_DISARM_TRAP            4
#define LOCKTYPE_OPEN                   5
#define LOCKTYPE_TREASURE               6
#define LOCKTYPE_CALCIFIED_ELVEN_GEMS   7
#define LOCKTYPE_CLOSE                  8
#define LOCKTYPE_ARM_TRAP               9
#define LOCKTYPE_QUICK_OPEN             10
#define LOCKTYPE_QUICK_CLOSE            11
#define LOCKTYPE_OPEN_TINKERING         12
#define LOCKTYPE_OPEN_KNEELING          13
#define LOCKTYPE_OPEN_ATTACKING         14
#define LOCKTYPE_GAHZRIDIAN             15
#define LOCKTYPE_BLASTING               16
#define LOCKTYPE_SLOW_OPEN              17
#define LOCKTYPE_SLOW_CLOSE             18

#define TRAINER_TYPE_CLASS              0
#define TRAINER_TYPE_MOUNTS             1
#define TRAINER_TYPE_TRADESKILLS        2
#define TRAINER_TYPE_PETS               3

#define CREATURE_TYPE_BEAST             1
#define CREATURE_TYPE_DRAGON            2
#define CREATURE_TYPE_DEMON             3
#define CREATURE_TYPE_ELEMENTAL         4
#define CREATURE_TYPE_GIANT             5
#define CREATURE_TYPE_UNDEAD            6
#define CREATURE_TYPE_HUMANOID          7
#define CREATURE_TYPE_CRITTER           8
#define CREATURE_TYPE_MECHANICAL        9
#define CREATURE_TYPE_UNKNOWN           10

#define CREATURE_FAMILY_WOLF            1
#define CREATURE_FAMILY_CAT             2
#define CREATURE_FAMILY_SPIDER          3
#define CREATURE_FAMILY_BEAR            4
#define CREATURE_FAMILY_BOAR            5
#define CREATURE_FAMILY_CROCILISK       6
#define CREATURE_FAMILY_CARRION_BIRD    7
#define CREATURE_FAMILY_CRAB            8
#define CREATURE_FAMILY_GORILLA         9
#define CREATURE_FAMILY_RAPTOR          11
#define CREATURE_FAMILY_TALLSTRIDER     12
#define CREATURE_FAMILY_FELHUNTER       15
#define CREATURE_FAMILY_VOIDWALKER      16
#define CREATURE_FAMILY_SUCCUBUS        17
#define CREATURE_FAMILY_DOOMGUARD       19
#define CREATURE_FAMILY_SCORPID         20
#define CREATURE_FAMILY_TURTLE          21
#define CREATURE_FAMILY_IMP             23
#define CREATURE_FAMILY_BAT             24
#define CREATURE_FAMILY_HYENA           25
#define CREATURE_FAMILY_OWL             26
#define CREATURE_FAMILY_WIND_SERPENT    27

#define CREATURE_ELITE_NORMAL           0
#define CREATURE_ELITE_ELITE            1
#define CREATURE_ELITE_RAREELITE        2
#define CREATURE_ELITE_WORLDBOSS        3
#define CREATURE_ELITE_RARE             4

#define QUEST_TYPE_ELITE                1
#define QUEST_TYPE_LIFE                 21
#define QUEST_TYPE_PVP                  41
#define QUEST_TYPE_RAID                 62
#define QUEST_TYPE_DUNGEON              81

#define QUEST_SORT_EPIC                 1
#define QUEST_SORT_WAILING_CAVERNS_OLD  21
#define QUEST_SORT_SEASONAL             22
#define QUEST_SORT_UNDERCITY_OLD        23
#define QUEST_SORT_HERBALISM            24
#define QUEST_SORT_SCARLET_MONASTERY_OLD 25
#define QUEST_SORT_ULDAMN_OLD           41
#define QUEST_SORT_WARLOCK              61
#define QUEST_SORT_WARRIOR              81
#define QUEST_SORT_SHAMAN               82
#define QUEST_SORT_FISHING              101
#define QUEST_SORT_BLACKSMITHING        121
#define QUEST_SORT_PALADIN              141
#define QUEST_SORT_MAGE                 161
#define QUEST_SORT_ROGUE                162
#define QUEST_SORT_ALCHEMY              181
#define QUEST_SORT_LEATHERWORKING       182
#define QUEST_SORT_ENGINERING           201
#define QUEST_SORT_TREASURE_MAP         221
#define QUEST_SORT_SUNKEN_TEMPLE_OLD    241
#define QUEST_SORT_HUNTER               261
#define QUEST_SORT_PRIEST               262
#define QUEST_SORT_DRUID                263
#define QUEST_SORT_TAILORING            264
#define QUEST_SORT_SPECIAL              284
#define QUEST_SORT_COOKING              304
#define QUEST_SORT_FIRST_AID            324

#define SKILL_FROST                     6
#define SKILL_FIRE                      8
#define SKILL_ARMS                      26
#define SKILL_COMBAT                    38
#define SKILL_SUBTLETY                  39
#define SKILL_POISONS                   40
#define SKILL_SWORDS                    43
#define SKILL_AXES                      44
#define SKILL_BOWS                      45
#define SKILL_GUNS                      46
#define SKILL_BEAST_MASTERY             50
#define SKILL_SURVIVAL                  51
#define SKILL_MACES                     54
#define SKILL_HOLY                      56
#define SKILL_2H_SWORDS                 55
#define SKILL_SHADOW                    78
#define SKILL_DEFENSE                   95
#define SKILL_LANG_COMMON               98
#define SKILL_RACIAL_DWARVEN            101
#define SKILL_LANG_ORCISH               109
#define SKILL_LANG_DWARVEN              111
#define SKILL_LANG_DARNASSIAN           113
#define SKILL_LANG_TAURAHE              115
#define SKILL_DUAL_WIELD                118
#define SKILL_RACIAL_TAUREN             124
#define SKILL_ORC_RACIAL                125
#define SKILL_RACIAL_NIGHT_ELF          126
#define SKILL_FIRST_AID                 129
#define SKILL_FERAL_COMBAT              134
#define SKILL_STAVES                    136
#define SKILL_LANG_THALASSIAN           137
#define SKILL_LANG_DRACONIC             138
#define SKILL_LANG_DEMON_TONGUE         139
#define SKILL_LANG_TITAN                140
#define SKILL_LANG_OLD_TONGUE           141
#define SKILL_SURVIVAL2                 142
#define SKILL_RIDING_HORSE              148
#define SKILL_RIDING_WOLF               149
#define SKILL_RIDING_RAM                152
#define SKILL_RIDING_TIGER              150
#define SKILL_SWIMING                   155
#define SKILL_2H_MACES                  160
#define SKILL_UNARMED                   162
#define SKILL_MARKSMANSHIP              163
#define SKILL_BLACKSMITHING             164
#define SKILL_LEATHERWORKING            165
#define SKILL_ALCHEMY                   171
#define SKILL_2H_AXES                   172
#define SKILL_DAGGERS                   173
#define SKILL_THROWN                    176
#define SKILL_HERBALISM                 182
#define SKILL_GENERIC_DND               183
#define SKILL_RETRIBUTION               184
#define SKILL_COOKING                   185
#define SKILL_MINING                    186
#define SKILL_PET_IMP                   188
#define SKILL_PET_FELHUNTER             189
#define SKILL_TAILORING                 197
#define SKILL_ENGINERING                202
#define SKILL_PET_SPIDER                203
#define SKILL_PET_VOIDWALKER            204
#define SKILL_PET_SUCCUBUS              205
#define SKILL_PET_INFERNAL              206
#define SKILL_PET_DOOMGUARD             207
#define SKILL_PET_WOLF                  208
#define SKILL_PET_CAT                   209
#define SKILL_PET_BEAR                  210
#define SKILL_PET_BOAR                  211
#define SKILL_PET_CROCILISK             212
#define SKILL_PET_CARRION_BIRD          213
#define SKILL_PET_GORILLA               215
#define SKILL_PET_CRAB                  214
#define SKILL_PET_RAPTOR                217
#define SKILL_PET_TALLSTRIDER           218
#define SKILL_RACIAL_UNDED              220
#define SKILL_WEAPON_TALENTS            222
#define SKILL_CROSSBOWS                 226
#define SKILL_SPEARS                    227
#define SKILL_WANDS                     228
#define SKILL_POLEARMS                  229
#define SKILL_ATTRIBUTE_ENCHANCEMENTS   230
#define SKILL_SLAYER_TALENTS            231
#define SKILL_MAGIC_TALENTS             233
#define SKILL_DEFENSIVE_TALENTS         234
#define SKILL_PET_SCORPID               236
#define SKILL_ARCANE                    237
#define SKILL_OPEN_LOCK                 242
#define SKILL_PET_TURTLE                251
#define SKILL_FURY                      256
#define SKILL_PROTECTION                257
#define SKILL_BEAST_TRAINING            261
#define SKILL_PROTECTION2               267
#define SKILL_PET_TALENTS               270
#define SKILL_PLATE_MAIL                293
#define SKILL_ASSASSINATION             253
#define SKILL_LANG_GNOMISH              313
#define SKILL_LANG_TROLL                315
#define SKILL_ENCHANTING                333
#define SKILL_DEMONOLOGY                354
#define SKILL_AFFLICTION                355
#define SKILL_FISHING                   356
#define SKILL_ENHANCEMENT               373
#define SKILL_RESTORATION               374
#define SKILL_ELEMENTAL_COMBAT          375
#define SKILL_SKINNING                  393
#define SKILL_LEATHER                   414
#define SKILL_CLOTH                     415
#define SKILL_MAIL                      413
#define SKILL_SHIELD                    433
#define SKILL_FIST_WEAPONS              473
#define SKILL_TRACKING_BEAST            513
#define SKILL_TRACKING_HUMANOID         514
#define SKILL_TRACKING_DEMON            516
#define SKILL_TRACKING_UNDEAD           517
#define SKILL_TRACKING_DRAGON           518
#define SKILL_TRACKING_ELEMENTAL        519
#define SKILL_RIDING_RAPTOR             533
#define SKILL_RIDING_MECHANOSTRIDER     553
#define SKILL_RIDING_UNDEAD_HORSE       554
#define SKILL_RESTORATION2              573
#define SKILL_BALANCE                   574
#define SKILL_DESTRUCTION               593
#define SKILL_HOLY2                     594
#define SKILL_DISCIPLINE                613
#define SKILL_LOCKPICKING               633
#define SKILL_PET_BAT                   653
#define SKILL_PET_HYENA                 654
#define SKILL_PET_OWL                   655
#define SKILL_PET_WIND_SERPENT          656
#define SKILL_LANG_GUTTERSPEAK          673
#define SKILL_RIDING_KODO               713
#define SKILL_RACIAL_TROLL              733
#define SKILL_RACIAL_GNOME              753
#define SKILL_RACIAL_HUMAN              754
#define SKILL_RIDING                    762

#define UNIT_DYNFLAG_LOOTABLE           0x0001

#define UNIT_DYNFLAG_TRACK_UNIT         0x0002

#define UNIT_DYNFLAG_OTHER_TAGGER       0x0004

#define UNIT_DYNFLAG_ROOTED             0x0008

#define UNIT_DYNFLAG_SPECIALINFO        0x0010

#define UNIT_DYNFLAG_DEAD               0x0020

#define UNIT_FLAG_NOT_ATTACKABLE        0x0002

#define UNIT_FLAG_ATTACKABLE            0x0008

#define UNIT_FLAG_NOT_ATTACKABLE_1      0x0080

#define UNIT_FLAG_NON_PVP_PLAYER        (UNIT_FLAG_ATTACKABLE + UNIT_FLAG_NOT_ATTACKABLE_1)

#define UNIT_FLAG_ANIMATION_FROZEN      0x0400
#define UNIT_FLAG_WAR_PLAYER            0x1000

#define SPELL_ID_AGGRO                    22764
#endif
