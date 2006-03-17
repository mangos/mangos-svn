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

#ifndef MANGOSSERVER_SINGLETON_H
#define MANGOSSERVER_SINGLETON_H

#include "Errors.h"


#define initialiseSingleton( type ) \
template <> type * Singleton < type > :: mSingleton = 0



#define createFileSingleton( type ) \
initialiseSingleton( type ); \
type the##type

template < class type > class Singleton
{
    public:
        
        Singleton( )
        {
               WPAssert( mSingleton == 0 );
            mSingleton = static_cast<type *>(this);
        }
        
        ~Singleton( )
        {
            mSingleton = 0;
        }

        
        static type & getSingleton( ) { WPAssert( mSingleton ); return *mSingleton; }

        
        static type * getSingletonPtr( ) { return mSingleton; }

    protected:
        
        static type * mSingleton;
};
#endif
