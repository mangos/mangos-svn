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

#ifndef MANGOSSERVER_PATH_H
#define MANGOSSERVER_PATH_H



class Path
{
    public:
        struct PathNode
        {
            float x,y,z;
        };
        Path( ): mNodes( 0 ), mLength( 0 ) { }
        inline void setLength( const uint16 & length )
        {
            Clear( );
            mLength = length; mNodes = new PathNode[ length ];
        }
        inline const uint16 & getLength( ) const { return mLength; };
        inline PathNode * getNodes( ) { return mNodes; }
        inline void Clear( ) { if( mNodes ) delete [ ] mNodes; mNodes = 0; }
        float getTotalLength( )
        {
            float len = 0, xd, yd, zd;
            for( uint32 a = 1; a < mLength; a ++ )
            {
                xd = mNodes[ a ].x - mNodes[ a-1 ].x;
                yd = mNodes[ a ].y - mNodes[ a-1 ].y;
                zd = mNodes[ a ].z - mNodes[ a-1 ].z;
                len += (float)sqrt( xd * xd + yd*yd + zd*zd );
            }
            return len;
        }
        ~Path( ) { Clear( ); }
    protected:
        PathNode * mNodes;
        uint16 mLength;
};
#endif
