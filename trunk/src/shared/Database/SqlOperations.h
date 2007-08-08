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

#ifndef __SQLOPERATIONS_H
#define __SQLOPERATIONS_H

#include "Common.h"

#include "zthread/LockedQueue.h"
#include "zthread/FastMutex.h"
#include <queue>

/// ---- BASE ---

class Database;

class SqlOperation
{
    public:
        virtual void Execute(Database *db) = 0;
        virtual ~SqlOperation() {}
};

/// ---- ASYNC STATEMENTS / TRANSACTIONS ----

class SqlStatement : public SqlOperation
{
    private:
        const char *m_sql;
    public:
        SqlStatement(const char *sql) : m_sql(strdup(sql)){}
        ~SqlStatement() { free((void*)m_sql); }
        void Execute(Database *db);
};

class SqlTransaction : public SqlOperation
{
    private:
        std::queue<const char *> m_queue;
    public:
        SqlTransaction() {}
        void DelayExecute(const char *sql) { m_queue.push(strdup(sql)); }
        void Execute(Database *db);
};
#endif                                                      //__SQLOPERATIONS_H
