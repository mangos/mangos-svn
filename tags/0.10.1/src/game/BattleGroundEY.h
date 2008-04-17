/* 
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
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

#ifndef __BATTLEGROUNDEY_H
#define __BATTLEGROUNDEY_H

class BattleGround;

class BattleGroundEYScore : public BattleGroundScore
{
    public:
        BattleGroundEYScore () {};
        virtual ~BattleGroundEYScore() {};
        //todo FIX ME! (add special columns...)
};

class BattleGroundEY : public BattleGround
{
    friend class BattleGroundMgr;

    public:
        BattleGroundEY();
        ~BattleGroundEY();
        void Update(time_t diff);

        /* inherited from BattlegroundClass */
        virtual void AddPlayer(Player *plr);

        void RemovePlayer(Player *plr,uint64 guid);
        void HandleAreaTrigger(Player *Source, uint32 Trigger);
        //bool SetupBattleGround();

    private:
};
#endif
