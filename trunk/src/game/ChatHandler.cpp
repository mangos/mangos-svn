/* 
 * Copyright (C) 2005,2006 MaNGOS <http://www.mangosproject.org/>
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

void WorldSession::HandleMessagechatOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    uint32 type;
    uint32 lang;

    recv_data >> type;
    recv_data >> lang;
    sLog.outDebug("CHAT: packet received. type %u, lang %u", type, lang );

    if (sWorld.getConfig(CONFIG_SEPARATE_FACTION) == 0)
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

            if (GetPlayer()->IsInGroup())
            {
                Group *group = objmgr.GetGroupByLeader(GetPlayer()->GetGroupLeader());
                if (group)
                    group->BroadcastToGroup(this, msg);
            }
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
            Player *player = objmgr.GetPlayer(to.c_str());
            // send whispers from player to GM only if GM accept its (not show online state GM in other case)
            if(!player || GetSecurity() == 0 && player->GetSession()->GetSecurity() > 0 && !player->isAcceptWhispers())
            {
                std::string msg_err = "Player "+to+" is not online (Names are case sensitive)";
                sChatHandler.SendSysMessage(this ,msg_err.c_str() );
                break;
            }
            if (sWorld.getConfig(CONFIG_SEPARATE_FACTION) == 1 && GetSecurity() == 0 && player->GetSession()->GetSecurity() == 0 )
            {
                uint32 sidea = GetPlayer()->GetTeam();
                uint32 sideb = player->GetTeam();
                if( sidea != sideb )
                {
                    std::string msg_err = "Player "+to+" is not online (Names are case sensitive)";
                    sChatHandler.SendSysMessage(this ,msg_err.c_str() );
                    break;
                }
            }
            sChatHandler.FillMessageData(&data, this, type, lang, NULL, 0, msg.c_str() );
            player->GetSession()->SendPacket(&data);
            sChatHandler.FillMessageData(&data,this,CHAT_MSG_WHISPER_INFORM,lang,NULL,player->GetGUID(),msg.c_str() );
            SendPacket(&data);

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
            //player troggle's AFK
            break;

        default:
            sLog.outError("CHAT: unknown msg type %u, lang: %u", type, lang);
    }
}

void WorldSession::HandleTextEmoteOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
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

    emoteentry *em = sEmoteStore.LookupEntry(text_emote);
    if (em)
    {
        uint32 emote_anim = em->textid;

        data.Initialize(SMSG_EMOTE);
        data << (uint32)emote_anim;
        data << GetPlayer()->GetGUID();
        WPAssert(data.size() == 12);
        GetPlayer()->SendMessageToSet( &data, true );

        data.Initialize(SMSG_TEXT_EMOTE);
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
    }
}

void WorldSession::HandleChatIgnoredOpcode(WorldPacket& recv_data )
{
    WorldPacket data;
    uint64 iguid;
    std::string msg = "";
    sLog.outDebug("WORLD: Received CMSG_CHAT_IGNORED");

    recv_data >> iguid;

    Player *player = objmgr.GetPlayer(iguid);
    objmgr.GetPlayerNameByGUID(GetPlayer()->GetGUID(),msg);
    msg += " is ignoring you!";
    sChatHandler.FillSystemMessageData( &data, player->GetSession() ,msg.c_str() );
    player->GetSession()->SendPacket(&data);
}
