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

#define MAIL_MONEY_TAKEN 1
#define MAIL_ITEM_TAKEN 2
#define MAIL_RETURNED_TO_SENDER 3
#define MAIL_DELETED 4
#define MAIL_MADE_PERMANENT 5
#define MAIL_OK 0
#define MAIL_ERR_BAG_FULL 1
#define MAIL_ERR_CANNOT_SEND_TO_SELF 2
#define MAIL_ERR_NOT_ENOUGH_MONEY 3
#define MAIL_ERR_RECIPIENT_NOT_FOUND 4
#define MAIL_ERR_NOT_YOUR_TEAM 5
#define MAIL_ERR_INTERNAL_ERROR 6

struct Mail
{
    uint32 messageID;
    uint32 sender;
    uint32 receiver;
    std::string subject;
    std::string body;
    uint32 item_guidlow;
    uint32 item_id;
    time_t time;
    uint32 money;
    uint32 COD;
    uint32 checked;
};
