# Copyright (C) 2004 Wow Daemon
# Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

INCLUDES += -I$(srcdir) -I$(srcdir)/../../dep/include  -I$(srcdir)/../shared -I$(srcdir)/../framework

noinst_LIBRARIES = libgame.a

libgame_a_SOURCES = \
	AuctionHouse.cpp \
	Channel.cpp \
	ChannelHandler.cpp \
	CharacterHandler.cpp \
	Chat.cpp \
	ChatHandler.cpp \
	CombatHandler.cpp \
	Corpse.cpp \
	Creature.cpp \
	debugcmds.cpp \
	DuelHandler.cpp \
	DynamicObject.cpp \
	GameObject.cpp \
	GridNotifiers.cpp \
	GridStates.cpp \
	Group.cpp \
	GroupHandler.cpp \
	Item.cpp \
	ItemHandler.cpp \
	Level0.cpp \
	Level1.cpp \
	Level2.cpp \
	Level3.cpp \
	LootMgr.cpp \
	Mail.cpp \
	Map.cpp \
	MapManager.cpp \
	RedZoneDistrict.cpp \
	MapMgr.cpp \
	MiscHandler.cpp \
	MovementHandler.cpp \
	NPCHandler.cpp \
	ObjectAccessor.cpp \
	Object.cpp \
	ObjectGridLoader.cpp \
	ObjectMgr.cpp \
	Opcodes.cpp \
	Player.cpp \
	QueryHandler.cpp \
	QuestHandler.cpp \
	SkillHandler.cpp \
	Spell.cpp \
	SpellHandler.cpp \
	TaxiHandler.cpp \
	TradeHandler.cpp \
	Unit.cpp \
	UpdateData.cpp \
	World.cpp \
	WorldSession.cpp \
	WorldSocket.cpp \
	WorldSocketMgr.cpp \
	ZoneMapper.cpp \
	Affect.h \
	Channel.h \
	ChannelMgr.h \
	Chat.h \
	Corpse.h \
	Creature.h \
	DynamicObject.h \
	GameObject.h \
	GridNotifiers.h \
	Group.h \
	Item.h \
	ItemPrototype.h \
	LootMgr.h \
	Mail.h \
	MapCell.h \
	Map.h \
	MapMgr.h \
	NameTables.h \
	NPCHandler.h \
	ObjectGridLoader.h \
	Object.h \
	ObjectMgr.h \
	Opcodes.h \
	Path.h \
	Player.h \
	Quest.h \
	Spell.h \
	SpellPrototype.h \
	Stats.h \
	Unit.h \
	UpdateData.h \
	UpdateFields.h \
	UpdateMask.h \
	World.h \
	WorldSession.h \
	WorldSocket.h \
	WorldSocketMgr.h \
	ZoneMapper.h

if NEWFRAMEWORK
libgame_a_LIBADD = \
	AggressorAI.cpp \
	CreatureAI.cpp \
	CreatureAISelector.cpp \
	NullCreatureAI.cpp
endif
