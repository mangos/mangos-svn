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

#ifndef __BATTLEGROUNDWS_H
#define __BATTLEGROUNDWS_H

#include "BattleGround.h"

class BattleGroundWS : public BattleGround
{
    friend class BattleGroundMgr;

    public:
        /* Construction */
        BattleGroundWS();
        ~BattleGroundWS();
        void Update(time_t diff);

        /* BG Flags */
        uint64 GetAllianceFlagPickerGUID() const { return m_AllianceFlagPickerGUID; }
        uint64 GetHordeFlagPickerGUID() const { return m_HordeFlagPickerGUID; }
        void SetAllianceFlagPicker(uint64 guid) { m_AllianceFlagPickerGUID = guid; }
        void SetHordeFlagPicker(uint64 guid) { m_HordeFlagPickerGUID = guid; }
        bool IsAllianceFlagPickedup() const { return m_AllianceFlagPickerGUID != 0; }
        bool IsHordeFlagPickedup() const { return m_HordeFlagPickerGUID != 0; }
        void RespawnFlag(uint32 Team, bool captured);

        /* Battleground Events */
        void EventPlayerCapturedFlag(Player* Source);
        void EventPlayerDroppedFlag(Player* Source);
        void EventPlayerReturnedFlag(Player* Source);
        void EventPlayerPickedUpFlag(Player* Source);

        void RemovePlayer(Player *plr,uint64 guid);
        void HandleAreaTrigger(Player* Source, uint32 Trigger);
        void SetupBattleGround();

    private:
        /* WS */
        uint64 m_AllianceFlagPickerGUID;
        uint64 m_HordeFlagPickerGUID;
        GameObject *m_HordeFlag;
        GameObject *m_AllianceFlag;
        GameObject *SpeedBonus1;
        GameObject *SpeedBonus2;
        GameObject *RegenBonus1;
        GameObject *RegenBonus2;
        GameObject *BerserkBonus1;
        GameObject *BerserkBonus2;
        int32 SpeedBonus1Spawn[2];
        int32 SpeedBonus2Spawn[2];
        int32 RegenBonus1Spawn[2];
        int32 RegenBonus2Spawn[2];
        int32 BerserkBonus1Spawn[2];
        int32 BerserkBonus2Spawn[2];
        int32 AllianceFlagSpawn[2];
        int32 HordeFlagSpawn[2];

};
#endif
