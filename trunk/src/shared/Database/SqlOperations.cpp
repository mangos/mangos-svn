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

#include "SqlOperations.h"
#include "DatabaseEnv.h"

void SqlStatement::Execute(Database *db)
{
    db->DirectExecute(m_sql);
}

void SqlTransaction::Execute(Database *db)
{
    if(m_queue.empty()) return;
    db->DirectExecute("START TRANSACTION");
    while(!m_queue.empty())
    {
        char const *sql = m_queue.front();
        m_queue.pop();
        db->DirectExecute(sql);
        free((void*)const_cast<char*>(sql));
    }
    db->DirectExecute("COMMIT");
}
