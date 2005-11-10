/* GridNotifiers.h
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

#ifndef MANGOS_GRIDNOTIFIERS_H
#define MANGOS_GRIDNOTIFIERS_H

/** @page Grid System Overview
    The Grid System (TGS) in MaNGOS uses mainly the visitor mixture with
    template pattern.  It allows object of interested to keep track of nearby
    objects (not necessary of the same types).  An object moving into or
    out of the grid has to be notify the grid (the grid itself does
    not keep track of the object movements.. only the objects themself do.
 */

#include "ObjectGridLoader.h"
#include "ByteBuffer.h"
#include "UpdateData.h"
#include <iostream>
class Player;
class Map;

namespace MaNGOS
{
    /** PlayerNotifier class is responsible to inform all players in the grid
	that's within range the existence of the player.  As well, inform
	the player himself the existence of other players. It also grap
	the objects within its visibility. This is only use when player enters
	during login..
    */
    struct MANGOS_DLL_DECL PlayerNotifier
    {
	PlayerNotifier(Player &pl) : i_player(pl) {}
	void Visit(PlayerMapType &);
	void Notify(void);
	Player &i_player;
	UpdateData i_data;
    };
    
    /** VisibleNotifier notifies the client a series of
     * objects that's visible to the player. Usually
     * use when player relocate
     */
    struct MANGOS_DLL_DECL VisibleNotifier
    {
	Player &i_player;
	UpdateData i_data;
	VisibleNotifier(Player &player) : i_player(player) {}
	template<class T> void Visit(std::map<OBJECT_HANDLE, T *> &m);
	void Notify(void);

	// specialization
    template<> void VisibleNotifier::Visit(std::map<OBJECT_HANDLE, Creature *> &);
    template<> void VisibleNotifier::Visit(std::map<OBJECT_HANDLE, Player *> &);
    };
    
    
    /** NotVisibleNotifier noties the client a series
     * of objects is no longer visible to the player
     * use when player relocate
     */
    struct MANGOS_DLL_DECL NotVisibleNotifier
    {
	Player &i_player;
	UpdateData i_data;
	NotVisibleNotifier(Player &player) : i_player(player) {}
	void Notify(void);
	template<class T> void Visit(std::map<OBJECT_HANDLE, T *> &m);
	// speicalization
    template<> void NotVisibleNotifier::Visit(std::map<OBJECT_HANDLE, Creature *> &);
    template<> void NotVisibleNotifier::Visit(std::map<OBJECT_HANDLE, Player *> &);
    };
    
    /** ObjectVisibleNotifier notifies player that an object is freshly enters.
     * Use when creature relocate.
     */
    struct MANGOS_DLL_DECL ObjectVisibleNotifier
    {
	Object &i_object;
	ObjectVisibleNotifier(Object &obj) : i_object(obj) {}
	void Visit(PlayerMapType &);
    };
    
    
    /** ObjectNotVisibleNotifier notifies the player the object is gone.
     */
    struct MANGOS_DLL_DECL ObjectNotVisibleNotifier
    {
	Object &i_object;
	ObjectNotVisibleNotifier(Object &obj) : i_object(obj) {}
	void Visit(PlayerMapType &);
    };
    
    
    /** GridUpdater updates all object status only	
     */
    struct MANGOS_DLL_DECL GridUpdater
    {
        GridType &i_grid;
        uint32 i_timeDiff;
        GridUpdater(GridType &grid, uint32 diff) : i_grid(grid), i_timeDiff(diff) {}
	
        template<class T> void updateObjects(std::map<OBJECT_HANDLE, T *> &m)
        {
            std::map<OBJECT_HANDLE, T *> tmp(m);
            for(typename std::map<OBJECT_HANDLE, T*>::iterator iter=tmp.begin(); iter != tmp.end(); ++iter)
                iter->second->Update(i_timeDiff);
        }
	
        void Visit(PlayerMapType &m) { updateObjects<Player>(m); }
        void Visit(CreatureMapType &m){ updateObjects<Creature>(m); }
        void Visit(GameObjectMapType &m) { updateObjects<GameObject>(m); }
        void Visit(DynamicObjectMapType &m) { updateObjects<DynamicObject>(m); }
        void Visit(CorpseMapType &m) { updateObjects<Corpse>(m); }
    };
    
    /** MessageDeliverer delivers a certain incomming message to all players
     */
    struct MANGOS_DLL_DECL MessageDeliverer
    {
        Player &i_player;
        WorldPacket *i_message;
        MessageDeliverer(Player &pl, WorldPacket *msg) : i_player(pl), i_message(msg) {}
        void Visit(PlayerMapType &m);
    };
    
    /** ObjectMessageDeliverer is a message from unit or object (a non player to his in range
     *  players
     */
    struct MANGOS_DLL_DECL ObjectMessageDeliverer
    {
        Object &i_object;
        WorldPacket *i_message;
        ObjectMessageDeliverer(Object &obj, WorldPacket *msg) : i_object(obj), i_message(msg) {}
        void Visit(PlayerMapType &m);
    };
    
    
    /** CreatureVisibleMovementNotifier again informs the player in certain
     * cell that a creature has moved into its visiblility
     */
    struct MANGOS_DLL_DECL CreatureVisibleMovementNotifier
    {
	Creature &i_creature;
	CreatureVisibleMovementNotifier(Creature &creature) : i_creature(creature) {}
	void Visit(PlayerMapType &m);
    };
    
    /** CreatureNotVisibleMovementNotifier informs the player in certain
     * cell that a creature has moved out of its visiblility
     */
    struct MANGOS_DLL_DECL CreatureNotVisibleMovementNotifier
    {
	Creature &i_creature;
	CreatureNotVisibleMovementNotifier(Creature &creature) : i_creature(creature) {}
	void Visit(PlayerMapType &m);
    };
    
    /** ObjectUpdate
     */
    struct MANGOS_DLL_DECL ObjectUpdater
    {
	uint32 i_timeDiff;
	ObjectUpdater(const uint32 &diff) : i_timeDiff(diff) {}
	template<class T> void Visit(std::map<OBJECT_HANDLE, T *> &m);	
    template<> void Visit(std::map<OBJECT_HANDLE, Creature *> &);
    };
    

    /** This uses to grap the object/creature that the player intested within
     * the user's visibility
     */
    template<class T>
    struct MANGOS_DLL_DECL ObjectAccessorNotifier
    {
	T *& i_object;
	uint64 i_id;
	ObjectAccessorNotifier(T * &obj, uint64 id) : i_object(obj), i_id(id)
	{
	    i_object = NULL;
	}
	
	void Visit(std::map<OBJECT_HANDLE, T *> &m )
	{
	    if( i_object == NULL )
	    {
		typename std::map<OBJECT_HANDLE, T *>::iterator iter = m.find(i_id);
		if( iter != m.end() )
		{
		    assert( iter->second != NULL );
		    i_object = iter->second;
		}
	    }
	}

	template<class NOT_INTERESTED> void Visit(std::map<OBJECT_HANDLE, NOT_INTERESTED *> &m) {}
    };
}

#endif
