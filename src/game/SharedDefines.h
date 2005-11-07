// genders --------------------------------------
#define GENDER_MALE							0
#define GENDER_FEMALE						1
#define GENDER_NONE							2

// races ----------------------------------------
#define RACE_HUMAN							1
#define RACE_ORC							2
#define RACE_DWARF							3
#define RACE_NIGHT_ELF						4
#define RACE_UNDEAD							5
#define RACE_TAUREN							6
#define RACE_GNOME							7
#define RACE_TROLL							8

// classes --------------------------------------
#define CLASS_WARRIOR						1
#define CLASS_PALADIN						2
#define CLASS_HUNTER						3
#define CLASS_ROGUE							4
#define CLASS_PRIEST						5
#define CLASS_UNK1							6
#define CLASS_SHAMAN						7
#define CLASS_MAGE							8
#define CLASS_WARLOCK						9
#define CLASS_UNK2							10
#define CLASS_DRUID							11

// stats --------------------------------------------
#define STAT_STRENGTH						0
#define STAT_AGILITY						1
#define STAT_STAMINA						2
#define STAT_INTELLECT						3
#define STAT_SPIRIT							4

// powers -------------------------------------------
#define POWER_MANA							0
#define POWER_RAGE							1
#define POWER_FOCUS							2
#define POWER_ENERGY						3
#define POWER_HAPPINESS						4

// schools ------------------------------------------
#define SPELL_SCHOOL_HOLY					1
#define SPELL_SCHOOL_FIRE					2
#define SPELL_SCHOOL_NATURE					3
#define SPELL_SCHOOL_FROST					4
#define SPELL_SCHOOL_SHADOW					5
#define SPELL_SCHOOL_ARCANE					6

// classes ------------------------------------------
/*
#define ITEM_CLASS_CONSUMABLE 				0
#define ITEM_CLASS_CONTAINER				1
#define ITEM_CLASS_WEAPON					2
#define ITEM_CLASS_JEWELRY					3 // obsolete
#define ITEM_CLASS_ARMOR					4
#define ITEM_CLASS_REAGENT					5
#define ITEM_CLASS_PROJECTILE				6
#define ITEM_CLASS_TRADE_GOODS				7
#define ITEM_CLASS_GENERIC					8
#define ITEM_CLASS_BOOK						9
#define ITEM_CLASS_MONEY					10 // obsolete
#define ITEM_CLASS_QUIVER					11
#define ITEM_CLASS_QUEST					12
#define ITEM_CLASS_KEY						13
#define ITEM_CLASS_PERMANENT				14
#define ITEM_CLASS_JUNK						15
*/

// subclasses ----------------------------------------
/*
#define ITEM_SUBCLASS_NEWITEM				-1
#define ITEM_SUBCLASS_CONSUMABLE			0
#define ITEM_SUBCLASS_FOOD					1 // obsolete
#define ITEM_SUBCLASS_LIQUID				2 // obsolete
#define ITEM_SUBCLASS_CONTAINER				0
#define ITEM_SUBCLASS_WEAPON_AXE			0
#define ITEM_SUBCLASS_WEAPON_AXE2			1
#define ITEM_SUBCLASS_WEAPON_BOW			2
#define ITEM_SUBCLASS_WEAPON_GUN			3
#define ITEM_SUBCLASS_WEAPON_MACE			4
#define ITEM_SUBCLASS_WEAPON_MACE2			5
#define ITEM_SUBCLASS_WEAPON_POLEARM		6
#define ITEM_SUBCLASS_WEAPON_SWORD			7
#define ITEM_SUBCLASS_WEAPON_SWORD2			8
#define ITEM_SUBCLASS_WEAPON_obsolete		9
#define ITEM_SUBCLASS_WEAPON_STAFF			10
#define ITEM_SUBCLASS_WEAPON_EXOTIC			11
#define ITEM_SUBCLASS_WEAPON_EXOTIC2		12
#define ITEM_SUBCLASS_WEAPON_UNARMED		13
#define ITEM_SUBCLASS_WEAPON_GENERIC		14
#define ITEM_SUBCLASS_WEAPON_DAGGER			15
#define ITEM_SUBCLASS_WEAPON_THROWN			16
#define ITEM_SUBCLASS_WEAPON_SPEAR			17
#define ITEM_SUBCLASS_WEAPON_CROSSBOW		18
#define ITEM_SUBCLASS_WEAPON_WAND			19
#define ITEM_SUBCLASS_WEAPON_FISHING_POLE	20
#define ITEM_SUBCLASS_JEWELRY				0 // obsolete
#define ITEM_SUBCLASS_ARMOR_GENERIC			0
#define ITEM_SUBCLASS_ARMOR_CLOTH			1
#define ITEM_SUBCLASS_ARMOR_LEATHER			2
#define ITEM_SUBCLASS_ARMOR_MAIL			3
#define ITEM_SUBCLASS_ARMOR_PLATE			4
#define ITEM_SUBCLASS_ARMOR_BUCKLER			5
#define ITEM_SUBCLASS_ARMOR_SHIELD			6
#define ITEM_SUBCLASS_REAGENT				0
#define ITEM_SUBCLASS_NONE					0 // wands
#define ITEM_SUBCLASS_BOLT					1
#define ITEM_SUBCLASS_ARROW					2
#define ITEM_SUBCLASS_BULLET				3
#define ITEM_SUBCLASS_THROWN				4
#define ITEM_SUBCLASS_TRADE_GOODS			0
#define ITEM_SUBCLASS_HERBS					1 // obsolete
#define ITEM_SUBCLASS_GEMS					2 // obsolete
#define ITEM_SUBCLASS_GENERIC				0 // obsolete
#define ITEM_SUBCLASS_BOOK					0
#define ITEM_SUBCLASS_SCROLL				1 // obsolete
#define ITEM_SUBCLASS_BOOK_WAND				2 // obsolete
#define ITEM_SUBCLASS_MONEY					0
#define ITEM_SUBCLASS_QUIVER0				0 // obsolete
#define ITEM_SUBCLASS_QUIVER1				1
#define ITEM_SUBCLASS_QUIVER2				2
#define ITEM_SUBCLASS_AMMOPOUCH				3
#define ITEM_SUBCLASS_QUEST					0
#define ITEM_SUBCLASS_KEY					0
#define ITEM_SUBCLASS_LOCKPICK				1
#define ITEM_SUBCLASS_PERMANENT				0
#define ITEM_SUBCLASS_JUNK					0
*/

// itemtypes -----------------------------------------
/*
#define INVTYPE_NONE						0
#define INVTYPE_HEAD						1
#define INVTYPE_NECK						2
#define INVTYPE_SHOULDER					3
#define INVTYPE_BODY						4
#define INVTYPE_CHEST						5
#define INVTYPE_WAIST						6
#define INVTYPE_LEGS						7
#define INVTYPE_FEET						8
#define INVTYPE_WRIST						9
#define INVTYPE_HAND						10
#define INVTYPE_FINGER						11
#define INVTYPE_TRINKET						12
#define INVTYPE_WEAPON						13
#define INVTYPE_SHIELD						14
#define INVTYPE_RANGED						15
#define INVTYPE_CLOAK						16
#define INVTYPE_2HWEAPON					17
#define INVTYPE_BAG							18
#define INVTYPE_TABARD						19
#define INVTYPE_ROBE						20
#define INVTYPE_WEAPONMAINHAND				21
#define INVTYPE_WEAPONOFFHAND				22
#define INVTYPE_HOLDABLE					23
#define INVTYPE_AMMO						24
#define INVTYPE_THROWN						25
#define INVTYPE_RANGEDRIGHT					26
*/

// sheath types ----------------------------------
#define SHEATHETYPE_NONE					0
#define SHEATHETYPE_MAINHAND				1
#define SHEATHETYPE_OFFHAND					2
#define SHEATHETYPE_LARGEWEAPONLEFT			3
#define SHEATHETYPE_LARGEWEAPONRIGHT		4
#define SHEATHETYPE_HIPWEAPONLEFT			5
#define SHEATHETYPE_HIPWEAPONRIGHT			6
#define SHEATHETYPE_SHIELD					7

// slots ----------------------------------------
#define SLOT_HEAD							0
#define SLOT_NECK							1
#define SLOT_SHOULDERS						2
#define SLOT_SHIRT							3
#define SLOT_CHEST							4
#define SLOT_WAIST							5
#define SLOT_LEGS							6
#define SLOT_FEET							7
#define SLOT_WRISTS							8
#define SLOT_HANDS							9
#define SLOT_FINGER1						10
#define SLOT_FINGER2						11
#define SLOT_TRINKET1						12
#define SLOT_TRINKET2						13
#define SLOT_BACK							14
#define SLOT_MAIN_HAND						15
#define SLOT_OFF_HAND						16
#define SLOT_RANGED							17
#define SLOT_TABARD							18
#define SLOT_EMPTY							19

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define LANG_GLOBAL							0
#define LANG_UNIVERSAL						0 //LANG_GLOBAL
#define LANG_ORCISH							1
#define LANG_DARNASSIAN						2
#define LANG_TAURAHE						3
#define LANG_DWARVISH						6
#define LANG_COMMON							7
#define LANG_DEMONIC						8
#define LANG_TITAN							9
#define LANG_THELASSIAN						10
#define LANG_DRACONIC						11
#define LANG_KALIMAG						12
#define LANG_GNOMISH						13
#define LANG_TROLL							14
#define LANG_GUTTERSPEAK					33

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SPELL_EFFECT_INSTAKILL					1	//done
#define SPELL_EFFECT_SCHOOL_DAMAGE				2	//done
#define SPELL_EFFECT_DUMMY						3	//nope
#define SPELL_EFFECT_PORTAL_TELEPORT			4	//not exist
#define SPELL_EFFECT_TELEPORT_UNITS				5	//done
#define SPELL_EFFECT_APPLY_AURA					6	//done
#define SPELL_EFFECT_ENVIRONMENTAL_DAMAGE		7	//done
#define SPELL_EFFECT_MANA_DRAIN					8	//done
#define SPELL_EFFECT_HEALTH_LEECH				9	//done
#define SPELL_EFFECT_HEAL						10	//done
#define SPELL_EFFECT_BIND						11	//done
#define SPELL_EFFECT_PORTAL						12
#define SPELL_EFFECT_RITUAL_BASE				13
#define SPELL_EFFECT_RITUAL_SPECIALIZE			14
#define SPELL_EFFECT_RITUAL_ACTIVATE_PORTAL		15
#define SPELL_EFFECT_QUEST_COMPLETE				16	//done
#define SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL		17	//done
#define SPELL_EFFECT_RESURRECT					18	//done
#define SPELL_EFFECT_ADD_EXTRA_ATTACKS			19	//done
#define SPELL_EFFECT_DODGE						20	//done
#define SPELL_EFFECT_EVADE						21	//done
#define SPELL_EFFECT_PARRY						22	//done
#define SPELL_EFFECT_BLOCK						23	//done
#define SPELL_EFFECT_CREATE_ITEM				24	//done
#define SPELL_EFFECT_WEAPON						25
#define SPELL_EFFECT_DEFENSE					26
#define SPELL_EFFECT_PERSISTENT_AREA_AURA		27	//done
#define SPELL_EFFECT_SUMMON						28	//done
#define SPELL_EFFECT_LEAP						29	//done
#define SPELL_EFFECT_ENERGIZE					30	//done
#define SPELL_EFFECT_WEAPON_PERCENT_DAMAGE		31	//done
#define SPELL_EFFECT_TRIGGER_MISSILE			32	//not exist
#define SPELL_EFFECT_OPEN_LOCK					33	//done
#define SPELL_EFFECT_SUMMON_MOUNT_OBSOLETE		34	//obsolete
#define SPELL_EFFECT_APPLY_AREA_AURA			35	//done
#define SPELL_EFFECT_LEARN_SPELL				36	//done
#define SPELL_EFFECT_SPELL_DEFENSE				37	//not exist
#define SPELL_EFFECT_DISPEL						38	//done
#define SPELL_EFFECT_LANGUAGE					39
#define SPELL_EFFECT_DUAL_WIELD					40	//done
#define SPELL_EFFECT_SUMMON_WILD				41	//done
#define SPELL_EFFECT_SUMMON_GUARDIAN			42	//done
#define SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER	43
#define SPELL_EFFECT_SKILL_STEP					44	//done
#define SPELL_EFFECT_UNDEFINED_45				45	//not exist
#define SPELL_EFFECT_SPAWN						46
#define SPELL_EFFECT_TRADE_SKILL				47
#define SPELL_EFFECT_STEALTH					48
#define SPELL_EFFECT_DETECT						49
#define SPELL_EFFECT_SUMMON_OBJECT				50	//done
#define SPELL_EFFECT_FORCE_CRITICAL_HIT			51
#define SPELL_EFFECT_GUARANTEE_HIT				52
#define SPELL_EFFECT_ENCHANT_ITEM				53	//done
#define SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY		54	//done
#define SPELL_EFFECT_TAMECREATURE				55	//done
#define SPELL_EFFECT_SUMMON_PET					56	//done
#define SPELL_EFFECT_LEARN_PET_SPELL			57	//done
#define SPELL_EFFECT_WEAPON_DAMAGE				58	//done
#define SPELL_EFFECT_OPEN_LOCK_ITEM				59	//done
#define SPELL_EFFECT_PROFICIENCY				60
#define SPELL_EFFECT_SEND_EVENT					61	//done
#define SPELL_EFFECT_POWER_BURN					62
#define SPELL_EFFECT_THREAT						63
#define SPELL_EFFECT_TRIGGER_SPELL				64	//done
#define SPELL_EFFECT_HEALTH_FUNNEL				65
#define SPELL_EFFECT_POWER_FUNNEL				66
#define SPELL_EFFECT_HEAL_MAX_HEALTH			67	//done
#define SPELL_EFFECT_INTERRUPT_CAST				68
#define SPELL_EFFECT_DISTRACT					69
#define SPELL_EFFECT_PULL						70
#define SPELL_EFFECT_PICKPOCKET					71
#define SPELL_EFFECT_ADD_FARSIGHT				72
#define SPELL_EFFECT_SUMMON_POSSESSED			73	//done
#define SPELL_EFFECT_SUMMON_TOTEM				74	//done
#define SPELL_EFFECT_HEAL_MECHANICAL			75
#define SPELL_EFFECT_SUMMON_OBJECT_WILD			76
#define SPELL_EFFECT_SCRIPT_EFFECT				77	//done
#define SPELL_EFFECT_ATTACK						78
#define SPELL_EFFECT_SANCTUARY					79
#define SPELL_EFFECT_ADD_COMBO_POINTS			80	//done
#define SPELL_EFFECT_CREATE_HOUSE				81
#define SPELL_EFFECT_BIND_SIGHT					82
#define SPELL_EFFECT_DUEL						83
#define SPELL_EFFECT_STUCK						84
#define SPELL_EFFECT_SUMMON_PLAYER				85
#define SPELL_EFFECT_ACTIVATE_OBJECT			86
#define SPELL_EFFECT_SUMMON_TOTEM_SLOT_1		87	//done
#define SPELL_EFFECT_SUMMON_TOTEM_SLOT_2		88	//done
#define SPELL_EFFECT_SUMMON_TOTEM_SLOT_3		89	//done
#define SPELL_EFFECT_SUMMON_TOTEM_SLOT_4		90	//done
#define SPELL_EFFECT_THREAT_ALL					91
#define SPELL_EFFECT_ENCHANT_HELD_ITEM			92
#define SPELL_EFFECT_SUMMON_PHANTASM			93
#define SPELL_EFFECT_SELF_RESURRECT				94	//done
#define SPELL_EFFECT_SKINNING					95	//done
#define SPELL_EFFECT_CHARGE						96	//done
#define SPELL_EFFECT_SUMMON_CRITTER				97	//done
#define SPELL_EFFECT_KNOCK_BACK					98	//done
#define SPELL_EFFECT_DISENCHANT					99	//done
#define SPELL_EFFECT_INEBRIATE					100	//done
#define SPELL_EFFECT_FEED_PET					101
#define SPELL_EFFECT_DISMISS_PET				102
#define SPELL_EFFECT_REPUTATION					103
#define SPELL_EFFECT_SUMMON_OBJECT_SLOT_1		104
#define SPELL_EFFECT_SUMMON_OBJECT_SLOT_2		105
#define SPELL_EFFECT_SUMMON_OBJECT_SLOT_3		106
#define SPELL_EFFECT_SUMMON_OBJECT_SLOT_4		107
#define SPELL_EFFECT_DISPEL_MECHANIC			108	//done
#define SPELL_EFFECT_SUMMON_DEAD_PET			109
#define SPELL_EFFECT_DESTROY_ALL_TOTEMS			110
#define SPELL_EFFECT_DURABILITY_DAMAGE			111
#define SPELL_EFFECT_SUMMON_DEMON				112	//done
#define SPELL_EFFECT_RESURRECT_NEW				113	//done
#define SPELL_EFFECT_ATTACK_ME					114
#define SPELL_EFFECT_DURABILITY_DAMAGE_PCT		115
#define SPELL_EFFECT_SKIN_PLAYER_CORPSE			116
#define SPELL_EFFECT_SPIRIT_HEAL				117
#define SPELL_EFFECT_SKILL						118
#define SPELL_EFFECT_APPLY_AURA_NEW				119	//done
#define SPELL_EFFECT_TELEPORT_GRAVEYARD			120

// Auras
/*
#define SPELL_AURA_NONE									0	//none
#define SPELL_AURA_BIND_SIGHT							1
#define SPELL_AURA_MOD_POSSESS							2
#define SPELL_AURA_PERIODIC_DAMAGE						3	//done
#define SPELL_AURA_DUMMY								4
#define SPELL_AURA_MOD_CONFUSE							5
#define SPELL_AURA_MOD_CHARM							6
#define SPELL_AURA_MOD_FEAR								7
#define SPELL_AURA_PERIODIC_HEAL						8	//done
#define SPELL_AURA_MOD_ATTACKSPEED						9
#define SPELL_AURA_MOD_THREAT							10
#define SPELL_AURA_MOD_TAUNT							11
#define SPELL_AURA_MOD_STUN								12
#define SPELL_AURA_MOD_DAMAGE_DONE						13
#define SPELL_AURA_MOD_DAMAGE_TAKEN						14
#define SPELL_AURA_DAMAGE_SHIELD						15	//done
#define SPELL_AURA_MOD_STEALTH							16
#define SPELL_AURA_MOD_DETECT							17
#define SPELL_AURA_MOD_INVISIBILITY						18	
#define SPELL_AURA_MOD_INVISIBILITY_DETECTION			19
#define SPELL_AURA_MOD_OBS_INTELLECT					20
#define SPELL_AURA_MOD_OBS_SPIRIT						21
#define SPELL_AURA_MOD_RESISTANCE						22	//done
#define SPELL_AURA_PERIODIC_TRIGGER_SPELL				23	//done
#define SPELL_AURA_PERIODIC_ENERGIZE					24	//done
#define SPELL_AURA_MOD_PACIFY							25
#define SPELL_AURA_MOD_ROOT								26	//done
#define SPELL_AURA_MOD_SILENCE							27
#define SPELL_AURA_REFLECT_SPELLS						28
#define SPELL_AURA_MOD_STAT								29	//done
#define SPELL_AURA_MOD_SKILL							30
#define SPELL_AURA_MOD_INCREASE_SPEED					31	//done
#define SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED			32	//done
#define SPELL_AURA_MOD_DECREASE_SPEED					33	//done
#define SPELL_AURA_MOD_INCREASE_HEALTH					34
#define SPELL_AURA_MOD_INCREASE_ENERGY					35
#define SPELL_AURA_MOD_SHAPESHIFT						36	//done
#define SPELL_AURA_EFFECT_IMMUNITY						37
#define SPELL_AURA_STATE_IMMUNITY						38
#define SPELL_AURA_SCHOOL_IMMUNITY						39
#define SPELL_AURA_DAMAGE_IMMUNITY						40
#define SPELL_AURA_DISPEL_IMMUNITY						41
#define SPELL_AURA_PROC_TRIGGER_SPELL					42	//done
#define SPELL_AURA_PROC_TRIGGER_DAMAGE					43	//done
#define SPELL_AURA_TRACK_CREATURES						44	//done
#define SPELL_AURA_TRACK_RESOURCES						45	//done
#define SPELL_AURA_MOD_PARRY_SKILL						46	//none
#define SPELL_AURA_MOD_PARRY_PERCENT					47	//done
#define SPELL_AURA_MOD_DODGE_SKILL						48	//none
#define SPELL_AURA_MOD_DODGE_PERCENT					49	//done
#define SPELL_AURA_MOD_BLOCK_SKILL						50
#define SPELL_AURA_MOD_BLOCK_PERCENT					51	//done
#define SPELL_AURA_MOD_CRIT_PERCENT						52	//done
#define SPELL_AURA_PERIODIC_LEECH						53	//done
#define SPELL_AURA_MOD_HIT_CHANCE						54
#define SPELL_AURA_MOD_SPELL_HIT_CHANCE					55
#define SPELL_AURA_TRANSFORM							56	//done
#define SPELL_AURA_MOD_SPELL_CRIT_CHANCE				57
#define SPELL_AURA_MOD_INCREASE_SWIM_SPEED				58
#define SPELL_AURA_MOD_DAMAGE_DONE_CREATURE				59
#define SPELL_AURA_MOD_PACIFY_SILENCE					60
#define SPELL_AURA_MOD_SCALE							61
#define SPELL_AURA_PERIODIC_HEALTH_FUNNEL				62
#define SPELL_AURA_PERIODIC_MANA_FUNNEL					63
#define SPELL_AURA_PERIODIC_MANA_LEECH					64
#define SPELL_AURA_MOD_CASTING_SPEED					65	//done
#define SPELL_AURA_FEIGN_DEATH							66
#define SPELL_AURA_MOD_DISARM							67
#define SPELL_AURA_MOD_STALKED							68
#define SPELL_AURA_SCHOOL_ABSORB						69	//done
#define SPELL_AURA_EXTRA_ATTACKS						70
#define SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL			71
#define SPELL_AURA_MOD_POWER_COST						72
#define SPELL_AURA_MOD_POWER_COST_SCHOOL				73
#define SPELL_AURA_REFLECT_SPELLS_SCHOOL				74
#define SPELL_AURA_MOD_LANGUAGE							75
#define SPELL_AURA_FAR_SIGHT							76
#define SPELL_AURA_MECHANIC_IMMUNITY					77
#define SPELL_AURA_MOUNTED								78	//done
#define SPELL_AURA_MOD_DAMAGE_DONE_PERCENT				79
#define SPELL_AURA_MOD_STAT_PERCENT						80	//done
#define SPELL_AURA_SPLIT_DAMAGE							81
#define SPELL_AURA_WATER_BREATHING						82
#define SPELL_AURA_MOD_BASE_RESISTANCE					83	//done
#define SPELL_AURA_MOD_REGEN							84	//done
#define SPELL_AURA_MOD_POWER_REGEN						85	//done
#define SPELL_AURA_CHANNEL_DEATH_ITEM					86	//done
#define SPELL_AURA_MOD_DAMAGE_TAKEN_PERCENT				87
#define SPELL_AURA_MOD_REGEN_PERCENT					88
#define SPELL_AURA_PERIODIC_DAMAGE_PERCENT				89
#define SPELL_AURA_MOD_RESIST_CHANCE					90
#define SPELL_AURA_MOD_DETECT_RANGE						91
#define SPELL_AURA_PREVENTS_FLEEING						92
#define SPELL_AURA_MOD_UNATTACKABLE						93
#define SPELL_AURA_INTERRUPT_REGEN						94
#define SPELL_AURA_GHOST								95
#define SPELL_AURA_SPELL_MAGNET							96
#define SPELL_AURA_MANA_SHIELD							97
#define SPELL_AURA_MOD_SKILL_TALENT						98
#define SPELL_AURA_MOD_ATTACK_POWER						99	//done
#define SPELL_AURA_AURAS_VISIBLE						100
#define SPELL_AURA_MOD_RESISTANCE_PCT					101	//done
#define SPELL_AURA_MOD_CREATURE_ATTACK_POWER			102
#define SPELL_AURA_MOD_TOTAL_THREAT						103
#define SPELL_AURA_WATER_WALK							104
#define SPELL_AURA_FEATHER_FALL							105
#define SPELL_AURA_HOVER								106
#define SPELL_AURA_ADD_FLAT_MODIFIER					107	//done
#define SPELL_AURA_ADD_PCT_MODIFIER						108	//done
#define SPELL_AURA_ADD_TARGET_TRIGGER					109
#define SPELL_AURA_MOD_POWER_REGEN_PERCENT				110
#define SPELL_AURA_ADD_CASTER_HIT_TRIGGER				111
#define SPELL_AURA_OVERRIDE_CLASS_SCRIPTS				112
#define SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN				113
#define SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT			114
#define SPELL_AURA_MOD_HEALING							115
#define SPELL_AURA_IGNORE_REGEN_INTERRUPT				116
#define SPELL_AURA_MOD_MECHANIC_RESISTANCE				117
#define SPELL_AURA_MOD_HEALING_PCT						118
#define SPELL_AURA_SHARE_PET_TRACKING					119
#define SPELL_AURA_UNTRACKABLE							120
#define SPELL_AURA_EMPATHY								121
#define SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT				122
#define SPELL_AURA_MOD_POWER_COST_PCT					123
#define SPELL_AURA_MOD_RANGED_ATTACK_POWER				124	//done
#define SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN				125
#define SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT			126
#define SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS	127
#define SPELL_AURA_MOD_POSSESS_PET						128
#define SPELL_AURA_MOD_INCREASE_SPEED_ALWAYS			129	//done
#define SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS				130	//done
#define SPELL_AURA_MOD_CREATURE_RANGED_ATTACK_POWER		131
#define SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT			132
#define SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT			133
#define SPELL_AURA_MOD_MANA_REGEN_INTERRUPT				134
#define SPELL_AURA_MOD_HEALING_DONE						135
#define SPELL_AURA_MOD_HEALING_DONE_PERCENT				136
#define SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE			137	//done
#define SPELL_AURA_MOD_HASTE							138	//done
#define SPELL_AURA_FORCE_REACTION						139
#define SPELL_AURA_MOD_RANGED_HASTE						140
#define SPELL_AURA_MOD_RANGED_AMMO_HASTE				141
#define SPELL_AURA_MOD_BASE_RESISTANCE_PCT				142	//done
#define SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE				143
#define SPELL_AURA_SAFE_FALL							144
#define SPELL_AURA_CHARISMA								145
#define SPELL_AURA_PERSUADED							146
#define SPELL_AURA_ADD_CREATURE_IMMUNITY				147
#define SPELL_AURA_RETAIN_COMBO_POINTS					148
#define SPELL_AURA_RESIST_PUSHBACK						149
#define SPELL_AURA_MOD_SHIELD_BLOCK						150
#define SPELL_AURA_TRACK_STEALTHED						151
#define SPELL_AURA_MOD_DETECTED_RANGE					152
#define SPELL_AURA_SPLIT_DAMAGE_FLAT					153
#define SPELL_AURA_MOD_STEALTH_LEVEL					154
#define SPELL_AURA_MOD_WATER_BREATHING					155
#define SPELL_AURA_MOD_REPUTATION_ADJUST				156
#define SPELL_AURA_PET_DAMAGE_MULTI						157
#define SPELL_AURA_MOD_SHIELD_BLOCK_VALUE				158
#define SPELL_AURA_NO_PVP_CREDIT						159
#define SPELL_AURA_UNK_160								160
#define SPELL_AURA_HEALTH_REGEN							161
#define SPELL_AURA_BURN_MANA_AND_HEALTH					162
#define SPELL_AURA_MOD_CRIT_DAMAGE_DONE_PCT				163
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define STATE_STANDING				0
#define STATE_SITTING				1
#define STATE_SITTINGCHAIR			2
#define STATE_SLEEPING				3
#define STATE_SITTINGCHAIRLOW		4
#define STATE_SITTINGCHAIRMEDIUM	5
#define STATE_SITTINGCHAIRHIGH		6
#define STATE_DEAD					7
#define STATE_KNEEL					8

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define GAMEOBJECT_TYPE_DOOR			0
#define GAMEOBJECT_TYPE_BUTTON			1
#define GAMEOBJECT_TYPE_QUESTGIVER		2
#define GAMEOBJECT_TYPE_CHEST			3
#define GAMEOBJECT_TYPE_BINDER			4
#define GAMEOBJECT_TYPE_GENERIC			5
#define GAMEOBJECT_TYPE_TRAP			6
#define GAMEOBJECT_TYPE_CHAIR			7
#define GAMEOBJECT_TYPE_SPELL_FOCUS		8
#define GAMEOBJECT_TYPE_TEXT			9
#define GAMEOBJECT_TYPE_GOOBER			10
#define GAMEOBJECT_TYPE_TRANSPORT		11
#define GAMEOBJECT_TYPE_AREADAMAGE		12
#define GAMEOBJECT_TYPE_CAMERA			13
#define GAMEOBJECT_TYPE_MAP_OBJECT		14
#define GAMEOBJECT_TYPE_MO_TRANSPORT	15
#define GAMEOBJECT_TYPE_DUEL_ARBITER	16
#define GAMEOBJECT_TYPE_FISHINGNODE		17
#define GAMEOBJECT_TYPE_RITUAL			18
#define GAMEOBJECT_TYPE_MAILBOX			19
#define GAMEOBJECT_TYPE_AUCTIONHOUSE	20
#define GAMEOBJECT_TYPE_GUARDPOST		21
#define GAMEOBJECT_TYPE_SPELLCASTER		22
#define GAMEOBJECT_TYPE_MEETINGSTONE	23
#define GAMEOBJECT_TYPE_FLAGSTAND		24
#define GAMEOBJECT_TYPE_FISHINGHOLE		25
#define GAMEOBJECT_TYPE_FLAGDROP		26

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define TEXTEMOTE_AGREE					1
#define TEXTEMOTE_AMAZE					2
#define TEXTEMOTE_ANGRY					3
#define TEXTEMOTE_APOLOGIZE				4
#define TEXTEMOTE_APPLAUD				5
#define TEXTEMOTE_BASHFUL				6
#define TEXTEMOTE_BECKON				7
#define TEXTEMOTE_BEG					8
#define TEXTEMOTE_BITE					9
#define TEXTEMOTE_BLEED					10
#define TEXTEMOTE_BLINK					11
#define TEXTEMOTE_BLUSH					12
#define TEXTEMOTE_BONK					13
#define TEXTEMOTE_BORED					14
#define TEXTEMOTE_BOUNCE				15
#define TEXTEMOTE_BRB					16
#define TEXTEMOTE_BOW					17
#define TEXTEMOTE_BURP					18
#define TEXTEMOTE_BYE					19
#define TEXTEMOTE_CACKLE				20
#define TEXTEMOTE_CHEER					21
#define TEXTEMOTE_CHICKEN				22
#define TEXTEMOTE_CHUCKLE				23
#define TEXTEMOTE_CLAP					24
#define TEXTEMOTE_CONFUSED				25
#define TEXTEMOTE_CONGRATULATE			26
#define TEXTEMOTE_COUGH					27
#define TEXTEMOTE_COWER					28
#define TEXTEMOTE_CRACK					29
#define TEXTEMOTE_CRINGE				30
#define TEXTEMOTE_CRY					31
#define TEXTEMOTE_CURIOUS				32
#define TEXTEMOTE_CURTSEY				33
#define TEXTEMOTE_DANCE					34
#define TEXTEMOTE_DRINK					35
#define TEXTEMOTE_DROOL					36
#define TEXTEMOTE_EAT					37
#define TEXTEMOTE_EYE					38
#define TEXTEMOTE_FART					39
#define TEXTEMOTE_FIDGET				40
#define TEXTEMOTE_FLEX					41
#define TEXTEMOTE_FROWN					42
#define TEXTEMOTE_GASP					43
#define TEXTEMOTE_GAZE					44
#define TEXTEMOTE_GIGGLE				45
#define TEXTEMOTE_GLARE					46
#define TEXTEMOTE_GLOAT					47
#define TEXTEMOTE_GREET					48
#define TEXTEMOTE_GRIN					49
#define TEXTEMOTE_GROAN					50
#define TEXTEMOTE_GROVEL				51
#define TEXTEMOTE_GUFFAW				52
#define TEXTEMOTE_HAIL					53
#define TEXTEMOTE_HAPPY					54
#define TEXTEMOTE_HELLO					55
#define TEXTEMOTE_HUG					56
#define TEXTEMOTE_HUNGRY				57
#define TEXTEMOTE_KISS					58
#define TEXTEMOTE_KNEEL					59
#define TEXTEMOTE_LAUGH					60
#define TEXTEMOTE_LAYDOWN				61
#define TEXTEMOTE_MASSAGE				62
#define TEXTEMOTE_MOAN					63
#define TEXTEMOTE_MOON					64
#define TEXTEMOTE_MOURN					65
#define TEXTEMOTE_NO					66
#define TEXTEMOTE_NOD					67
#define TEXTEMOTE_NOSEPICK				68
#define TEXTEMOTE_PANIC					69
#define TEXTEMOTE_PEER					70
#define TEXTEMOTE_PLEAD					71
#define TEXTEMOTE_POINT					72
#define TEXTEMOTE_POKE					73
#define TEXTEMOTE_PRAY					74
#define TEXTEMOTE_ROAR					75
#define TEXTEMOTE_ROFL					76
#define TEXTEMOTE_RUDE					77
#define TEXTEMOTE_SALUTE				78
#define TEXTEMOTE_SCRATCH				79
#define TEXTEMOTE_SEXY					80
#define TEXTEMOTE_SHAKE					81
#define TEXTEMOTE_SHOUT					82
#define TEXTEMOTE_SHRUG					83
#define TEXTEMOTE_SHY					84
#define TEXTEMOTE_SIGH					85
#define TEXTEMOTE_SIT					86
#define TEXTEMOTE_SLEEP					87
#define TEXTEMOTE_SNARL					88
#define TEXTEMOTE_SPIT					89
#define TEXTEMOTE_STARE					90
#define TEXTEMOTE_SURPRISED				91
#define TEXTEMOTE_SURRENDER				92
#define TEXTEMOTE_TALK					93
#define TEXTEMOTE_TALKEX				94
#define TEXTEMOTE_TALKQ					95
#define TEXTEMOTE_TAP					96
#define TEXTEMOTE_THANK					97
#define TEXTEMOTE_THREATEN				98
#define TEXTEMOTE_TIRED					99
#define TEXTEMOTE_VICTORY				100
#define TEXTEMOTE_WAVE					101
#define TEXTEMOTE_WELCOME				102
#define TEXTEMOTE_WHINE					103
#define TEXTEMOTE_WHISTLE				104
#define TEXTEMOTE_WORK					105
#define TEXTEMOTE_YAWN					106
#define TEXTEMOTE_BOGGLE				107
#define TEXTEMOTE_CALM					108
#define TEXTEMOTE_COLD					109
#define TEXTEMOTE_COMFORT				110
#define TEXTEMOTE_CUDDLE				111
#define TEXTEMOTE_DUCK					112
#define TEXTEMOTE_INSULT				113
#define TEXTEMOTE_INTRODUCE				114
#define TEXTEMOTE_JK					115
#define TEXTEMOTE_LICK					116
#define TEXTEMOTE_LISTEN				117
#define TEXTEMOTE_LOST					118
#define TEXTEMOTE_MOCK					119
#define TEXTEMOTE_PONDER				120
#define TEXTEMOTE_POUNCE				121
#define TEXTEMOTE_PRAISE				122
#define TEXTEMOTE_PURR					123
#define TEXTEMOTE_PUZZLE				124
#define TEXTEMOTE_RAISE					125
#define TEXTEMOTE_READY					126
#define TEXTEMOTE_SHIMMY				127
#define TEXTEMOTE_SHIVER				128
#define TEXTEMOTE_SHOO					129
#define TEXTEMOTE_SLAP					130
#define TEXTEMOTE_SMIRK					131
#define TEXTEMOTE_SNIFF					132
#define TEXTEMOTE_SNUB					133
#define TEXTEMOTE_SOOTHE				134
#define TEXTEMOTE_STINK					135
#define TEXTEMOTE_TAUNT					136
#define TEXTEMOTE_TEASE					137
#define TEXTEMOTE_THIRSTY				138
#define TEXTEMOTE_VETO					139
#define TEXTEMOTE_SNICKER				140
#define TEXTEMOTE_STAND					141
#define TEXTEMOTE_TICKLE				142
#define TEXTEMOTE_VIOLIN				143
#define TEXTEMOTE_SMILE					163
#define TEXTEMOTE_RASP					183
#define TEXTEMOTE_PITY					203
#define TEXTEMOTE_GROWL					204
#define TEXTEMOTE_BARK					205
#define TEXTEMOTE_SCARED				223
#define TEXTEMOTE_FLOP					224
#define TEXTEMOTE_LOVE					225
#define TEXTEMOTE_MOO					226
#define TEXTEMOTE_COMMEND				243
#define TEXTEMOTE_JOKE					329

// Emotes
#define EMOTE_ONESHOT_NONE					0
#define EMOTE_ONESHOT_TALK					1	//DNR
#define EMOTE_ONESHOT_BOW					2
#define EMOTE_ONESHOT_WAVE					3	//DNR
#define EMOTE_ONESHOT_CHEER					4	//DNR
#define EMOTE_ONESHOT_EXCLAMATION			5	//DNR
#define EMOTE_ONESHOT_QUESTION				6
#define EMOTE_ONESHOT_EAT					7
#define EMOTE_STATE_DANCE					10
#define EMOTE_ONESHOT_LAUGH					11
#define EMOTE_STATE_SLEEP					12
#define EMOTE_STATE_SIT						13
#define EMOTE_ONESHOT_RUDE					14	//DNR
#define EMOTE_ONESHOT_ROAR					15	//DNR
#define EMOTE_ONESHOT_KNEEL					16
#define EMOTE_ONESHOT_KISS					17
#define EMOTE_ONESHOT_CRY					18
#define EMOTE_ONESHOT_CHICKEN				19
#define EMOTE_ONESHOT_BEG					20
#define EMOTE_ONESHOT_APPLAUD				21
#define EMOTE_ONESHOT_SHOUT					22	//DNR
#define EMOTE_ONESHOT_FLEX					23
#define EMOTE_ONESHOT_SHY					24	//DNR
#define EMOTE_ONESHOT_POINT					25	//DNR
#define EMOTE_STATE_STAND					26
#define EMOTE_STATE_READYUNARMED			27
#define EMOTE_STATE_WORK					28
#define EMOTE_STATE_POINT					29	//DNR
#define EMOTE_STATE_NONE					30
#define EMOTE_ONESHOT_WOUND					33
#define EMOTE_ONESHOT_WOUNDCRITICAL			34
#define EMOTE_ONESHOT_ATTACKUNARMED			35
#define EMOTE_ONESHOT_ATTACK1H				36
#define EMOTE_ONESHOT_ATTACK2HTIGHT			37
#define EMOTE_ONESHOT_ATTACK2HLOOSE			38
#define EMOTE_ONESHOT_PARRYUNARMED			39
#define EMOTE_ONESHOT_PARRYSHIELD			43
#define EMOTE_ONESHOT_READYUNARMED			44
#define EMOTE_ONESHOT_READY1H				45
#define EMOTE_ONESHOT_READYBOW				48
#define EMOTE_ONESHOT_SPELLPRECAST			50
#define EMOTE_ONESHOT_SPELLCAST				51
#define EMOTE_ONESHOT_BATTLEROAR			53
#define EMOTE_ONESHOT_SPECIALATTACK1H		54
#define EMOTE_ONESHOT_KICK					60
#define EMOTE_ONESHOT_ATTACKTHROWN			61
#define EMOTE_STATE_STUN					64
#define EMOTE_STATE_DEAD					65
#define EMOTE_ONESHOT_SALUTE				66
#define EMOTE_STATE_KNEEL					68
#define EMOTE_STATE_USESTANDING				69
#define EMOTE_ONESHOT_WAVE_NOSHEATHE		70
#define EMOTE_ONESHOT_CHEER_NOSHEATHE		71
#define EMOTE_ONESHOT_EAT_NOSHEATHE			92
#define EMOTE_STATE_STUN_NOSHEATHE			93
#define EMOTE_ONESHOT_DANCE					94
#define EMOTE_ONESHOT_SALUTE_NOSHEATH		113
#define EMOTE_STATE_USESTANDING_NOSHEATHE	133
#define EMOTE_ONESHOT_LAUGH_NOSHEATHE		153
#define EMOTE_STATE_WORK_NOSHEATHE			173
#define EMOTE_STATE_SPELLPRECAST			193
#define EMOTE_ONESHOT_READYRIFLE			213
#define EMOTE_STATE_READYRIFLE				214
#define EMOTE_STATE_WORK_NOSHEATHE_MINING	233
#define EMOTE_STATE_WORK_NOSHEATHE_CHOPWOOD	234
#define EMOTE_zzOLDONESHOT_LIFTOFF			253
#define EMOTE_ONESHOT_LIFTOFF				254
#define EMOTE_ONESHOT_YES					273	//DNR
#define EMOTE_ONESHOT_NO					274	//DNR
#define EMOTE_ONESHOT_TRAIN					275	//DNR
#define EMOTE_ONESHOT_LAND					293
#define EMOTE_STATE_READY1H					333
#define EMOTE_STATE_AT_EASE					313
#define EMOTE_STATE_SPELLKNEELSTART			353
#define EMOTE_STATE_SUBMERGED				373
#define EMOTE_ONESHOT_SUBMERGE				374

// Anims
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

// GameObject Anims...
#define FIRST_GAMEOBJECTANIMATION       0x87	//135
#define ANIM_GAMEOBJ_STAND              0
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define FIRST_EFFECTANIMATION           0x93
#define ANIM_EFFECT_STAND               0
#define ANIM_EFFECT_HOLD                1
#define ANIM_EFFECT_DECAY               2

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define FIRST_ITEMANIMATION             0x96
#define ANIM_ITEM_STAND                 0
#define ANIM_ITEM_INFLIGHT              1
#define ANIM_ITEM_BOWPULL               2
#define ANIM_ITEM_BOWRELEASE            3

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define LOCKTYPE_PICKLOCK				1
#define LOCKTYPE_HERBALISM				2
#define LOCKTYPE_MINING					3
#define LOCKTYPE_DISARM_TRAP			4
#define LOCKTYPE_OPEN					5
#define LOCKTYPE_TREASURE				6
#define LOCKTYPE_CALCIFIED_ELVEN_GEMS	7
#define LOCKTYPE_CLOSE					8
#define LOCKTYPE_ARM_TRAP				9
#define LOCKTYPE_QUICK_OPEN				10
#define LOCKTYPE_QUICK_CLOSE			11
#define LOCKTYPE_OPEN_TINKERING			12
#define LOCKTYPE_OPEN_KNEELING			13
#define LOCKTYPE_OPEN_ATTACKING			14
#define LOCKTYPE_GAHZRIDIAN				15
#define LOCKTYPE_BLASTING				16
#define LOCKTYPE_SLOW_OPEN				17
#define LOCKTYPE_SLOW_CLOSE				18

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define TRAINER_TYPE_GENERAL			0
#define TRAINER_TYPE_TALENTS			1
#define TRAINER_TYPE_TRADESKILLS		2
#define TRAINER_TYPE_PETS				3

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SHAPESHIFT_FORM_CAT				1		//,1,0,0,0,0,0,0,0,983054, 0,1, 1534
#define SHAPESHIFT_FORM_TREE			2		//,2,0,0,0,0,0,0,0,7274526,0,0, 0
#define SHAPESHIFT_FORM_TRAVEL			3		//,0,0,0,0,0,0,0,0,983054, 0,1, 0
#define SHAPESHIFT_FORM_AQUATIC			4		//,0,0,0,0,0,0,0,0,983054, 0,1, 0
#define SHAPESHIFT_FORM_BEAR			5		//,3,0,0,0,0,0,0,0,983054, 0,1, 496
#define SHAPESHIFT_AMBIENT				6		//,0,0,0,0,0,0,0,0,7274526,0,0, 0
#define SHAPESHIFT_GHOUL				7		//,0,0,0,0,0,0,0,0,7274526,0,0, 0
#define SHAPESHIFT_FORM_DIRE_BEAR		8		//,3,0,0,0,0,0,0,0,983054, 0,1, 496
#define SHAPESHIFT_CREATURE_BEAR		14		//,0,0,0,0,0,0,0,0,983054, 0,1, 0
#define SHAPESHIFT_GHOST_WOLF			16		//,0,0,0,0,0,0,0,0,983054, 0,1, 0
#define SHAPESHIFT_BATTLE_STANCE		17		//,1,0,0,0,0,0,0,0,7274526,7,0, 0
#define SHAPESHIFT_DEFENSIVE_STANCE		18		//,2,0,0,0,0,0,0,0,7274526,7,0, 0
#define SHAPESHIFT_BERSERKER_STANCE		19		//,3,0,0,0,0,0,0,0,7274526,7,0, 0
#define SHAPESHIFT_FORM_SHADOW			28		//,0,0,0,0,0,0,0,0,983054, 8,-1,0
#define SHAPESHIFT_STEALTH				30		//,1,0,0,0,0,0,0,0,983070, 1,0, 0
#define SHAPESHIFT_MOONKIN				31		//,0,0,0,0,0,0,0,2031676,  0,-1,0

// creature types --------------------------------
#define CREATURE_TYPE_BEAST				1
#define CREATURE_TYPE_DRAGON			2
#define CREATURE_TYPE_DEMON				3
#define CREATURE_TYPE_ELEMENTAL			4
#define CREATURE_TYPE_GIANT				5
#define CREATURE_TYPE_UNDEAD			6
#define CREATURE_TYPE_HUMANOID			7
#define CREATURE_TYPE_CRITTER			8
#define CREATURE_TYPE_MECHANICAL		9
#define CREATURE_TYPE_UNKNOWN			10

///////////////////////////////////////////////////
#define CREATURE_FAMILY_WOLF			1
#define CREATURE_FAMILY_CAT				2
#define CREATURE_FAMILY_SPIDER			3
#define CREATURE_FAMILY_BEAR			4
#define CREATURE_FAMILY_BOAR			5
#define CREATURE_FAMILY_CROCILISK		6
#define CREATURE_FAMILY_CARRION_BIRD	7
#define CREATURE_FAMILY_CRAB			8
#define CREATURE_FAMILY_GORILLA			9
#define CREATURE_FAMILY_RAPTOR			11
#define CREATURE_FAMILY_TALLSTRIDER		12
#define CREATURE_FAMILY_FELHUNTER		15
#define CREATURE_FAMILY_VOIDWALKER		16
#define CREATURE_FAMILY_SUCCUBUS		17
#define CREATURE_FAMILY_DOOMGUARD		19
#define CREATURE_FAMILY_SCORPID			20
#define CREATURE_FAMILY_TURTLE			21
#define CREATURE_FAMILY_IMP				23
#define CREATURE_FAMILY_BAT				24
#define CREATURE_FAMILY_HYENA			25
#define CREATURE_FAMILY_OWL				26
#define CREATURE_FAMILY_WIND_SERPENT	27

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define CREATURE_ELITE_NORMAL			0
#define CREATURE_ELITE_ELITE			1
#define CREATURE_ELITE_RAREELITE		2
#define CREATURE_ELITE_WORLDBOSS		3
#define CREATURE_ELITE_RARE				4

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define QUEST_TYPE_ELITE			1
#define QUEST_TYPE_LIFE				21
#define QUEST_TYPE_PVP				41
#define QUEST_TYPE_RAID				62
#define QUEST_TYPE_DUNGEON			81

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define QUEST_SORT_EPIC				1
#define QUEST_SORT_WAILING_CAVERNS_OLD		21
#define QUEST_SORT_SEASONAL			22
#define QUEST_SORT_UNDERCITY_OLD		23
#define QUEST_SORT_HERBALISM			24
#define QUEST_SORT_SCARLET_MONASTERY_OLD	25
#define QUEST_SORT_ULDAMN_OLD			41
#define QUEST_SORT_WARLOCK			61
#define QUEST_SORT_WARRIOR			81
#define QUEST_SORT_SHAMAN			82
#define QUEST_SORT_FISHING			101
#define QUEST_SORT_BLACKSMITHING		121
#define QUEST_SORT_PALADIN			141
#define QUEST_SORT_MAGE				161
#define QUEST_SORT_ROGUE			162
#define QUEST_SORT_ALCHEMY			181
#define QUEST_SORT_LEATHERWORKING		182
#define QUEST_SORT_ENGINERING			201
#define QUEST_SORT_TREASURE_MAP			221
#define QUEST_SORT_SUNKEN_TEMPLE_OLD		241
#define QUEST_SORT_HUNTER			261
#define QUEST_SORT_PRIEST			262
#define QUEST_SORT_DRUID			263
#define QUEST_SORT_TAILORING			264
#define QUEST_SORT_SPECIAL			284
#define QUEST_SORT_COOKING			304
#define QUEST_SORT_FIRST_AID			324

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SKILL_FROST						6		// (class)
#define SKILL_FIRE						8		// (class)
#define SKILL_ARMS						26		// (class)
#define SKILL_COMBAT					38		// (class)
#define SKILL_SUBTLETY					39		// (class)
#define SKILL_POISONS					40		// (class)
#define SKILL_SWORDS					43		// (weapon)
#define SKILL_AXES						44		// (weapon)
#define SKILL_BOWS						45		// (weapon)
#define SKILL_GUNS						46		// (weapon)
#define SKILL_BEAST_MASTERY				50		// (class)
#define SKILL_SURVIVAL					51		// (class)
#define SKILL_MACES						54		// (weapon)
#define SKILL_HOLY						56		// (class)
#define SKILL_2H_SWORDS					55		// (weapon)
#define SKILL_SHADOW					78		// (class)
#define SKILL_DEFENSE					95		// (weapon)
#define SKILL_LANG_COMMON				98		// (lang)
#define SKILL_RACIAL_DWARVEN			101		// (secondary)
#define SKILL_LANG_ORCISH				109		// (lang)
#define SKILL_LANG_DWARVEN				111		// (lang)
#define SKILL_LANG_DARNASSIAN			113		// (lang)
#define SKILL_LANG_TAURAHE				115		// (lang)
#define SKILL_DUAL_WIELD				118		// (weapon)
#define SKILL_RACIAL_TAUREN				124		// (secondary)
#define SKILL_ORC_RACIAL				125		// (secondary)
#define SKILL_RACIAL_NIGHT_ELF			126		// (secondary)
#define SKILL_FIRST_AID					129		// (secondary)
#define SKILL_FERAL_COMBAT				134		// (class)
#define SKILL_LANG_THALASSIAN			137		// (lang)
#define SKILL_STAVES					136		// (weapon)
#define SKILL_LANG_DRACONIC				138		// (lang)
#define SKILL_LANG_DEMON_TONGUE			139		// (lang)
#define SKILL_LANG_TITAN				140		// (lang)
#define SKILL_LANG_OLD_TONGUE			141		// (lang)
#define SKILL_SURVIVAL2					142		// (secondary)
#define SKILL_RIDING_HORSE				148		// (secondary)
#define SKILL_RIDING_WOLF				149		// (secondary)
#define SKILL_RIDING_RAM				152		// (secondary)
#define SKILL_RIDING_TIGER				150		// (secondary)
#define SKILL_SWIMING					155		// (secondary)
#define SKILL_2H_MACES					160		// (weapon)
#define SKILL_UNARMED					162		// (weapon)
#define SKILL_MARKSMANSHIP				163		// (class)
#define SKILL_BLACKSMITHING				164		// (profession)
#define SKILL_LEATHERWORKING			165		// (profession)
#define SKILL_ALCHEMY					171		// (profession)
#define SKILL_2H_AXES					172		// (weapon)
#define SKILL_DAGGERS					173		// (weapon)
#define SKILL_THROWN					176		// (weapon)
#define SKILL_HERBALISM					182		// (profession)
#define SKILL_GENERIC_DND				183		// (-----)
#define SKILL_RETRIBUTION				184		// (class)
#define SKILL_COOKING					185		// (secondary)
#define SKILL_MINING					186		// (profession)
#define SKILL_PET_IMP					188		// (class)
#define SKILL_PET_FELHUNTER				189		// (class)
#define SKILL_TAILORING					197		// (profession)
#define SKILL_ENGINERING				202		// (profession)
#define SKILL_PET_SPIDER				203		// (class)
#define SKILL_PET_VOIDWALKER			204		// (class)
#define SKILL_PET_SUCCUBUS				205		// (class)
#define SKILL_PET_INFERNAL				206		// (class)
#define SKILL_PET_DOOMGUARD				207		// (class)
#define SKILL_PET_WOLF					208		// (class)
#define SKILL_PET_CAT					209		// (class)
#define SKILL_PET_BEAR					210		// (class)
#define SKILL_PET_BOAR					211		// (class)
#define SKILL_PET_CROCILISK				212		// (class)
#define SKILL_PET_CARRION_BIRD			213		// (class)
#define SKILL_PET_GORILLA				215		// (class)
#define SKILL_PET_CRAB					214		// (class)
#define SKILL_PET_RAPTOR				217		// (class)
#define SKILL_PET_TALLSTRIDER			218		// (class)
#define SKILL_RACIAL_UNDED				220		// (secondary)
#define SKILL_WEAPON_TALENTS			222		// (-----)
#define SKILL_CROSSBOWS					226		// (weapon)
#define SKILL_WANDS						228		// (weapon)
#define SKILL_POLEARMS					229		// (weapon)
#define SKILL_ATTRIBUTE_ENCHANCEMENTS	230		// (-----)
#define SKILL_SLAYER_TALENTS			231		// (-----)
#define SKILL_MAGIC_TALENTS				233		// (-----)
#define SKILL_DEFENSIVE_TALENTS			234		// (-----)
#define SKILL_PET_SCORPID				236		// (class)
#define SKILL_ARCANE					237		// (class)
#define SKILL_PET_TURTLE				251		// (class)
#define SKILL_FURY						256		// (class)
#define SKILL_PROTECTION				257		// (class)
#define SKILL_BEAST_TRAINING			261		// (class)
#define SKILL_PROTECTION2				267		// (class)
#define SKILL_PET_TALENTS				270		// (class)
#define SKILL_PLATE_MAIL				293		// (armor)
#define SKILL_ASSASSINATION				253		// (class)
#define SKILL_LANG_GNOMISH				313		// (lang)
#define SKILL_LANG_TROLL				315		// (lang)
#define SKILL_ENCHANTING				333		// (profession)
#define SKILL_DEMONOLOGY				354		// (class)
#define SKILL_AFFLICTION				355		// (class)
#define SKILL_FISHING					356		// (secondary)
#define SKILL_ENHANCEMENT				373		// (class)
#define SKILL_RESTORATION				374		// (class)
#define SKILL_ELEMENTAL_COMBAT			375		// (class)
#define SKILL_SKINNING					393		// (profession)
#define SKILL_LEATHER					414		// (armor)
#define SKILL_CLOTH						415		// (armor)
#define SKILL_MAIL						413		// (armor)
#define SKILL_SHIELD					433		// (armor)
#define SKILL_FIST_WEAPONS				473		// (weapon)
#define SKILL_TRACKING_BEAST			513		// (secondary)
#define SKILL_TRACKING_HUMANOID			514		// (secondary)
#define SKILL_TRACKING_DEMON			516		// (secondary)
#define SKILL_TRACKING_UNDEAD			517		// (secondary)
#define SKILL_TRACKING_DRAGON			518		// (secondary)
#define SKILL_TRACKING_ELEMENTAL		519		// (secondary)
#define SKILL_RIDING_RAPTOR				533		// (secondary)
#define SKILL_RIDING_MECHANOSTRIDER		553		// (secondary)
#define SKILL_RIDING_UNDEAD_HORSE		554		// (secondary)
#define SKILL_RESTORATION2				573		// (class)
#define SKILL_BALANCE					574		// (class)
#define SKILL_DESTRUCTION				593		// (class)
#define SKILL_HOLY2						594		// (class)
#define SKILL_DISCIPLINE				613		// (class)
#define SKILL_LOCKPICKING				633		// (class)
#define SKILL_PET_BAT					653		// (class)
#define SKILL_PET_HYENA					654		// (class)
#define SKILL_PET_OWL					655		// (class)
#define SKILL_PET_WIND_SERPENT			656		// (class)
#define SKILL_LANG_GUTTERSPEAK			673		// (lang)
#define SKILL_RIDING_KODO				713		// (secondary)
#define SKILL_RACIAL_TROLL				733		// (secondary)
#define SKILL_RACIAL_GNOME				753		// (secondary)
#define SKILL_RACIAL_HUMAN				754		// (secondary)

