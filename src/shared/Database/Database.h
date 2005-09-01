/* Database.h
 *
 * Copyright (C) 2004 Wow Daemon
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

#if !defined(DATABASE_H)
#define DATABASE_H

class Database : public Singleton<Database>
{
    protected:
        Database() {}

    public:
//! Frees resources used by Database.
        virtual ~Database() {}

//! Initialize db, infoString is very implementation-dependant.
        virtual bool Initialize(const char *infoString) = 0;

//! Query the database with a SQL command.
/*
This is where all the transaction with the database takes place.
Any result data is held in memory and can be iterated through with NextRow().
@param sql SQL command to query the database with. Should be supported by all database modules being used.
@return Returns QueryResult if the query succeded and there are results and NULL otherwise */
        virtual QueryResult* Query(const char *sql) = 0;

//! Execute SQL command.
/*
@param sql SQL command to query the database with. Should be supported by all database modules being used.
@return Returns true if query succeded */
        virtual bool Execute(const char *sql) = 0;

//! Returns the status of the database, and should be used to ensure the database was opened successfully.
        virtual operator bool () const = 0;
};

#define sDatabase Database::getSingleton()
#endif
