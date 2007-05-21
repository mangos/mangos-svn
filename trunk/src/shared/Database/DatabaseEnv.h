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

#if !defined(DATABASEENV_H)
#define DATABASEENV_H

/************************************************************************/
/* Database Support Setup                                               */
/************************************************************************/
// Define the databases that you would like the server to be compiled with here.

#define DATABASE_SUPPORT_MYSQL
//#define DATABASE_SUPPORT_PGSQL
//#define DATABASE_SUPPORT_ORACLE10

//! Other libs we depend on.
#include "Common.h"
#include "Log.h"

//! Our own includes.
#include <mysql/mysql.h>
#include "Database/DBCStores.h"
#include "Database/Field.h"
#include "Database/Database.h"

// Implementations

/************************************************************************/
/* MySQL                                                                */
/************************************************************************/
#ifdef DATABASE_SUPPORT_MYSQL
#include "Database/impl/MySQLDatabase.h"
#endif

/************************************************************************/
/* PostgreSQL                                                           */
/************************************************************************/
#ifdef DATABASE_SUPPORT_PGSQL
#include "Database/impl/PostgreDatabase.h"
#endif

/************************************************************************/
/* Oracle 10g                                                           */
/************************************************************************/
#ifdef DATABASE_SUPPORT_ORACLE10
//#include "Database/impl/Oracle10.h"
#endif

#endif
