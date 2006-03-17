/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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

    Log::getSingleton().outDebug("CHAT: packet received");

    uint32 type;
    uint32 lang;

    recv_data >> type;
    recv_data >> lang;

    switch(type)
    {
        case CHAT_MSG_SAY:
        {
            std::string msg = "";
            recv_data >> msg;

            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
                break;

            sChatHandler.FillMessageData( &data, this, type, LANG_UNIVERSAL, NULL, msg.c_str() );
            GetPlayer()->SendMessageToSet( &data, true );
        } break;
        case CHAT_MSG_CHANNEL:
        {
            std::string channel = "", msg = "";
            recv_data >> channel;
            recv_data >> msg;
            Channel *chn = channelmgr.GetChannel(channel.c_str(),GetPlayer()); if(chn) chn->Say(GetPlayer(),msg.c_str());
        } break;
        case CHAT_MSG_WHISPER:
        {
            std::string to = "", msg = "";
            recv_data >> to >> msg;
            sChatHandler.FillMessageData(&data, this, type, LANG_UNIVERSAL, NULL, msg.c_str() );
            Player *player = objmgr.GetPlayer(to.c_str());
            if(!player)
            {
                data.clear();
                msg = "Player ";
                msg += to.c_str();
                msg += " is not online (Names are case sensitive)";
                sChatHandler.FillSystemMessageData( &data, this ,msg.c_str() );
                SendPacket(&data);
                break;
            }
            player->GetSession()->SendPacket(&data);
            
            sChatHandler.FillMessageData(&data, this, CHAT_MSG_WHISPER_INFORM, LANG_UNIVERSAL, ((char *)(player->GetGUID())), msg.c_str() );
            SendPacket(&data);
        } break;
        case CHAT_MSG_YELL:
        {
            std::string msg = "";
            recv_data >> msg;

            if (sChatHandler.ParseCommands(msg.c_str(), this) > 0)
                break;

            sChatHandler.FillMessageData(&data, this, type, LANG_UNIVERSAL, NULL, msg.c_str() );
            SendPacket(&data);
            sWorld.SendGlobalMessage(&data, this);
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
        	
        default:
            Log::getSingleton().outError("CHAT: unknown msg type %u, lang: %u", type, lang);
    }
}


void WorldSession::HandleTextEmoteOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint32 text_emote, unk;
    uint64 guid;

    recv_data >> text_emote;
    recv_data >> unk;
    recv_data >> guid;

    const char *nam = 0;
    uint32 namlen = 1;

    
    Unit* unit = ObjectAccessor::Instance().GetUnit(*_player, guid);
    Creature *pCreature = dynamic_cast<Creature *>(unit);
    if( pCreature != NULL )
    {
        nam = pCreature->GetName();
        namlen = strlen(nam) + 1;
    }
    {
	Player *pChar = dynamic_cast<Player *>(unit);
	if( pChar != NULL )
	{
	    nam = pChar->GetName();
	    namlen = strlen(nam) + 1;
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
        sWorld.SendGlobalMessage(&data);

        data.Initialize(SMSG_TEXT_EMOTE);
        data << GetPlayer()->GetGUID();
        data << (uint32)text_emote;
        data << (uint32)0xFF;                     
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
        SendPacket( &data );
        sWorld.SendGlobalMessage(&data, this);
    }
}

void WorldSession::HandleChatIgnoredOpcode(WorldPacket& recv_data )
{
	WorldPacket data;
    uint64 iguid;
	std::string msg = "";
	Log::getSingleton().outDebug("WORLD: Recieved CMSG_CHAT_IGNORED");

    recv_data >> iguid;	

	Player *player = objmgr.GetPlayer(iguid);
	objmgr.GetPlayerNameByGUID(GetPlayer()->GetGUID(),msg);
	msg += " is ignoring you!";
	sChatHandler.FillSystemMessageData( &data, player->GetSession() ,msg.c_str() );
	player->GetSession()->SendPacket(&data);
}
