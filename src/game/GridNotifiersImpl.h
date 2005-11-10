#ifndef MANGOS_GRIDNOTIFIERSIMPL_H
#define MANGOS_GRIDNOTIFIERSIMPL_H

#include "GridNotifiers.h"
#include "WorldPacket.h"
#include "Player.h"
#include "UpdateData.h"

template<>
inline void
MaNGOS::NotVisibleNotifier::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    // maybe we can player the trick of setting spriti healer's as a death state unint
    if( i_player.isAlive() )
    {
	for(std::map<OBJECT_HANDLE, Creature *>::iterator iter=m.begin(); iter != m.end(); ++iter)
	    if( iter->second->isAlive() )
		iter->second->BuildOutOfRangeUpdateBlock(&i_data);
    }
    else
    {
	for(std::map<OBJECT_HANDLE, Creature *>::iterator iter=m.begin(); iter != m.end(); ++iter)
	    if( iter->second->isDead() )
		iter->second->BuildOutOfRangeUpdateBlock(&i_data);
    }
}

template<>
inline void
MaNGOS::NotVisibleNotifier::Visit(std::map<OBJECT_HANDLE, Player *> &m)
{
    Player *player = &i_player;
    for(std::map<OBJECT_HANDLE, Player *>::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	if( iter->second == player )
	    continue;
	if( (i_player.isAlive() && iter->second->isAlive()) ||
	    (!i_player.isAlive() && !iter->second->isAlive()) )
	{
	    // build for me
	    iter->second->BuildOutOfRangeUpdateBlock(&i_data);
	    // build for him

	    UpdateData his_data;
	    WorldPacket his_pk;
	    i_player.BuildOutOfRangeUpdateBlock(&his_data);
	    his_data.BuildPacket(&his_pk);
	    iter->second->GetSession()->SendPacket(&his_pk);
	}
    }
}

 
template<>
inline void
MaNGOS::VisibleNotifier::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    // maybe we can player the trick of setting spriti healer's as a death state unint
    if( i_player.isAlive() )
    {
	for(std::map<OBJECT_HANDLE, Creature *>::iterator iter=m.begin(); iter != m.end(); ++iter)
	    if( iter->second->isAlive() )
		iter->second->BuildCreateUpdateBlockForPlayer(&i_data, &i_player);
    }
    else
    {
	for(std::map<OBJECT_HANDLE, Creature *>::iterator iter=m.begin(); iter != m.end(); ++iter)
	    if( iter->second->isDead() )
		iter->second->BuildCreateUpdateBlockForPlayer(&i_data, &i_player);
    }
}

template<>
inline void
MaNGOS::VisibleNotifier::Visit(std::map<OBJECT_HANDLE, Player *> &m)
{
    Player *player = &i_player;
    for(std::map<OBJECT_HANDLE, Player *>::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	if( iter->second == player )
	    continue;

	if( (i_player.isAlive() && iter->second->isAlive()) ||
	    (!i_player.isAlive() && !iter->second->isAlive()) )
	{
	    sLog.outDebug("Creating in range packet for both player %d and %d", i_player.GetGUID(), iter->second->GetGUID());
	    iter->second->BuildCreateUpdateBlockForPlayer(&i_data, &i_player);

	    // build meself for this guy
	    UpdateData his_data;
	    WorldPacket his_pk;
	    i_player.BuildCreateUpdateBlockForPlayer(&his_data, iter->second);
	    his_data.BuildPacket(&his_pk);
	    iter->second->GetSession()->SendPacket(&his_pk);
	}
    }
}

template<>
inline void
MaNGOS::ObjectUpdater::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    std::map<OBJECT_HANDLE, Creature *> tmp(m);
    for(std::map<OBJECT_HANDLE, Creature*>::iterator iter=tmp.begin(); iter != tmp.end(); ++iter)
	iter->second->Update(i_timeDiff);
}


#endif

