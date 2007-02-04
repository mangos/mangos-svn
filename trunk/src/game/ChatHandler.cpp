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

#include "Common.h"
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "Opcodes.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "Database/DatabaseEnv.h"
#include "ChannelMgr.h"
#include "Group.h"
#include "Guild.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "ScriptCalls.h"

void WorldSession::HandleMessagechatOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    uint32 type;
    uint32 lang;

    recv_data >> type;
    recv_data >> lang;
    //sLog.outDebug("CHAT: packet received. type %u, lang %u", type, lang );

    // prevent talking at unknown language (cheating)
    LanguageDesc const* langDesc = GetLanguageDescByID(lang);
    if(!langDesc || langDesc->skill_id != 0 && !_player->HasSkill(langDesc->skill_id))
    {
        SendNotification("Unknown language");
        return;
    }

    // send in universal language infaction iteration allowed mode and if player in .gmon mode
    if (sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION) || _player->isGameMaster())
        lang = LANG_UNIVERSAL;

    switch(type)
    {
        case CHAT_MSG_SAY:
        case CHAT_MSG_EMOTE:                                // "/me text" emote type
        {
            std::string msg = "";
            recv_data >> msg;

            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
                break;

            if(type==CHAT_MSG_SAY)
            {
                sChatHandler.FillMessageData( &data, this, type, lang, NULL, 0, msg.c_str() );
                GetPlayer()->SendMessageToSet( &data, true );
            }
            else
            {
                std::ostringstream msg2;
                msg2 << GetPlayer()->GetName() << " " << msg;
                sChatHandler.FillMessageData( &data, this, type, lang, NULL, 0, msg2.str().c_str() );
                GetPlayer()->SendMessageToOwnTeamSet( &data, true );
            }
        } break;

        case CHAT_MSG_PARTY:
        {
            std::string msg = "";
            recv_data >> msg;

            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
                break;

            Group *group = GetPlayer()->groupInfo.group;
            if(!group)
                return;

            WorldPacket data;
            sChatHandler.FillMessageData(&data, this, CHAT_MSG_PARTY, lang, NULL, 0, msg.c_str());
            group->BroadcastPacket(&data, group->GetMemberGroup(GetPlayer()->GetGUID()));
        }
        break;

        case CHAT_MSG_GUILD:
        {
            std::string msg = "";
            recv_data >> msg;

            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
                break;

            if (GetPlayer()->GetGuildId())
            {
                Guild *guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
                if (guild)
                    guild->BroadcastToGuild(this, msg);
            }

            break;
        }
        case CHAT_MSG_OFFICER:
        {
            std::string msg = "";
            recv_data >> msg;

            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
                break;

            if (GetPlayer()->GetGuildId())
            {
                Guild *guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
                if (guild)
                    guild->BroadcastToOfficers(this, msg);
            }
            break;
        }

        case CHAT_MSG_YELL:
        {
            std::string msg = "";
            recv_data >> msg;

            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
                break;

            sChatHandler.FillMessageData(&data, this, type, lang, NULL, 0, msg.c_str() );

            //please test this, its important that this is correct. I leave the previouse code becouse of this.
            GetPlayer()->SendMessageToSet( &data, true );
            //SendPacket(&data);
            //sWorld.SendGlobalMessage(&data, this);

        } break;

        case CHAT_MSG_WHISPER:
        {
            std::string to, msg;
            recv_data >> to >> msg;

            if(to.size() == 0)
            {
                WorldPacket data(SMSG_CHAT_PLAYER_NOT_FOUND, (to.size()+1));
                data<<to;
                SendPacket(&data);
                break;
            }

            normalizePlayerName(to);
            Player *player = objmgr.GetPlayer(to.c_str());
            // send whispers from player to GM only if GM accept its (not show online state GM in other case)
            if(!player || GetSecurity() == 0 && player->GetSession()->GetSecurity() > 0 && !player->isAcceptWhispers())
            {
                WorldPacket data(SMSG_CHAT_PLAYER_NOT_FOUND, (to.size()+1));
                data<<to;
                SendPacket(&data);
                break;
            }
            if (!sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION) && GetSecurity() == 0 && player->GetSession()->GetSecurity() == 0 )
            {
                uint32 sidea = GetPlayer()->GetTeam();
                uint32 sideb = player->GetTeam();
                if( sidea != sideb )
                {
                    WorldPacket data(SMSG_CHAT_PLAYER_NOT_FOUND, (to.size()+1));
                    data<<to;
                    SendPacket(&data);
                    break;
                }
            }

            sChatHandler.FillMessageData(&data, this, type, lang, NULL, 0, msg.c_str() );
            player->GetSession()->SendPacket(&data);

            sChatHandler.FillMessageData(&data,this,CHAT_MSG_WHISPER_INFORM,lang,NULL,player->GetGUID(),msg.c_str() );
            SendPacket(&data);

            if(player->isAFK())
            {
                sChatHandler.FillMessageData(&data, this, CHAT_MSG_AFK, LANG_UNIVERSAL, NULL, player->GetGUID(), player->afkMsg.c_str());
                SendPacket(&data);
            }
            if(player->isDND())
            {
                sChatHandler.FillMessageData(&data, this, CHAT_MSG_DND, LANG_UNIVERSAL, NULL, player->GetGUID(), player->dndMsg.c_str());
                SendPacket(&data);
            }

            // Auto enable whispers accepting at sending whispers
            if(!GetPlayer()->isAcceptWhispers())
            {
                GetPlayer()->SetAcceptWhispers(true);
                sChatHandler.SendSysMessage(this ,"Whispers accepting now: ON");
            }
        } break;

        case CHAT_MSG_CHANNEL:
        {
            std::string channel = "", msg = "";
            recv_data >> channel;
            recv_data >> msg;

            if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
                if(Channel *chn = cMgr->GetChannel(channel,GetPlayer()))
                    chn->Say(GetPlayer(),msg.c_str(),lang);
        } break;

        case CHAT_MSG_AFK:
        {
            std::string msg;
            recv_data >> msg;

            GetPlayer()->afkMsg = msg;
            if((msg.size() == 0 || !GetPlayer()->isAFK()) && !GetPlayer()->isInCombat() )
            {
                GetPlayer()->ToggleAFK();
                if(GetPlayer()->isAFK() && GetPlayer()->isDND())
                    GetPlayer()->ToggleDND();
            }
        } break;

        case CHAT_MSG_DND:
        {
            std::string msg;
            recv_data >> msg;

            GetPlayer()->dndMsg = msg;
            if(msg.size() == 0 || !GetPlayer()->isDND())
            {
                GetPlayer()->ToggleDND();
                if(GetPlayer()->isDND() && GetPlayer()->isAFK())
                    GetPlayer()->ToggleAFK();
            }
        } break;

        case CHAT_MSG_RAID:
        {
            std::string msg="";
            recv_data >> msg;

            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
                break;

            Group *group = GetPlayer()->groupInfo.group;
            if(!group || !group->isRaidGroup())
                return;

            WorldPacket data;
            sChatHandler.FillMessageData(&data, this, CHAT_MSG_RAID, lang, "", 0, msg.c_str());
            group->BroadcastPacket(&data);
        } break;

        case CHAT_MSG_RAID_LEADER:
        {
            std::string msg="";
            recv_data >> msg;

            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
                break;

            Group *group = GetPlayer()->groupInfo.group;
            if(!group || !group->isRaidGroup() || !group->IsLeader(GetPlayer()->GetGUID()))
                return;

            WorldPacket data;
            sChatHandler.FillMessageData(&data, this, CHAT_MSG_RAID_LEADER, lang, "", 0, msg.c_str());
            group->BroadcastPacket(&data);
        } break;

        case CHAT_MSG_RAID_WARN:
        {
            std::string msg="";
            recv_data >> msg;

            Group *group = GetPlayer()->groupInfo.group;
            if(!group || !group->isRaidGroup() || !group->IsLeader(GetPlayer()->GetGUID()))
                return;

            WorldPacket data;
            sChatHandler.FillMessageData(&data, this, CHAT_MSG_RAID_WARN, lang, "", 0, msg.c_str());
            group->BroadcastPacket(&data);
        } break;

        default:
            sLog.outError("CHAT: unknown msg type %u, lang: %u", type, lang);
    }
}

void WorldSession::HandleTextEmoteOpcode( WorldPacket & recv_data )
{
    uint32 text_emote, emoteNum;
    uint64 guid;

    recv_data >> text_emote;
    recv_data >> emoteNum;
    recv_data >> guid;

    const char *nam = 0;
    uint32 namlen = 1;

    Unit* unit = ObjectAccessor::Instance().GetUnit(*_player, guid);
    Creature *pCreature = dynamic_cast<Creature *>(unit);
    if( pCreature != NULL )
    {
        nam = pCreature->GetCreatureInfo()->Name;
        namlen = (nam ? strlen(nam) : 0) + 1;
    }
    {
        Player *pChar = dynamic_cast<Player *>(unit);
        if( pChar != NULL )
        {
            nam = pChar->GetName();
            namlen = (nam ? strlen(nam) : 0) + 1;
        }
    }

    EmotesTextEntry const *em = sEmotesTextStore.LookupEntry(text_emote);
    if (em)
    {
        uint32 emote_anim = em->textid;

        WorldPacket data;
        data.Initialize(SMSG_EMOTE, 12);
        data << (uint32)emote_anim;
        data << GetPlayer()->GetGUID();
        WPAssert(data.size() == 12);
        GetPlayer()->SendMessageToSet( &data, true );

        data.Initialize(SMSG_TEXT_EMOTE, (20+namlen));
        data << GetPlayer()->GetGUID();
        data << (uint32)text_emote;
        data << emoteNum;
        data << (uint32)namlen;
        if( namlen > 1 )
        {
            data.append(nam, namlen);
        }
        else
        {
            data << (uint8)0x00;
        }

        WPAssert(data.size() == 20 + namlen);
        GetPlayer()->SendMessageToSet( &data, true );

        //Send scripted event call
        if (pCreature && Script)
            Script->ReceiveEmote(GetPlayer(),pCreature,emote_anim);
    }
}

void WorldSession::HandleChatIgnoredOpcode(WorldPacket& recv_data )
{
    WorldPacket data;
    uint64 iguid;
    //sLog.outDebug("WORLD: Received CMSG_CHAT_IGNORED");

    recv_data >> iguid;

    Player *player = objmgr.GetPlayer(iguid);
    if(!player || !player->GetSession())
        return;

    sChatHandler.FillMessageData(&data, this, CHAT_MSG_IGNORED, LANG_UNIVERSAL, NULL, GetPlayer()->GetGUID(), GetPlayer()->GetName());
    player->GetSession()->SendPacket(&data);
}
