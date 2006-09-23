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


#ifndef REALM_ACCEPTOR_H
#define REALM_ACCEPTOR_H

#include "RealmHandler.h"
#include "ace/Acceptor.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "ace/SOCK_Acceptor.h"

class RealmAcceptor : public ACE_Acceptor<RealmHandler,ACE_SOCK_ACCEPTOR>
{

public:

  RealmAcceptor (void);
  virtual ~RealmAcceptor (void);

  virtual int make_svc_handler (RealmHandler * & sh);

private:

  ACE_Recursive_Thread_Mutex mutex_;

};

#endif /* REALM_ACCEPTOR_H */
