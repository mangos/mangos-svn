
#include "MapManager.h"
#include "Policies/SingletonImp.h"
#include "Log.h"

#define CLASS_LOCK MaNGOS::ClassLevelLockable<MapManager, ZThread::Mutex>
INSTANTIATE_SINGLETON_2(MapManager, CLASS_LOCK);
INSTANTIATE_CLASS_MUTEX(MapManager, ZThread::Mutex);

MapManager::MapManager()
{
}

MapManager::~MapManager()
{    
    for(MapMapType::iterator iter=i_maps.begin(); iter != i_maps.end(); ++iter)
	delete iter->second;   
}

Map*
MapManager::GetMap(uint32 id)
{
    Map *m = NULL;
    if( ( m=_getMap(id) ) == NULL )
    {
	Guard guard(*this);
	if( (m = _getMap(id)) == NULL )
	{
	    m = new Map(id);
	    i_maps[id] = m;	    
	}
	// ok..other player has created.	
    }

    assert(m != NULL);
    return m;
}

