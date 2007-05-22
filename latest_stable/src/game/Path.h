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

#ifndef MANGOSSERVER_PATH_H
#define MANGOSSERVER_PATH_H

#include <vector>

class Path
{
    public:
        struct PathNode
        {
            float x,y,z;
        };

        inline void SetLength(const unsigned int sz)
        {
            i_nodes.resize( sz );
        }

        inline unsigned int Size(void) const { return i_nodes.size(); }
        inline void Resize(unsigned int sz) { i_nodes.resize(sz); }
        inline void Clear(void) { i_nodes.clear(); }
        inline PathNode* GetNodes() { return &i_nodes[0]; }
        float GetTotalLength(void)
        {
            float len = 0, xd, yd, zd;
            for(unsigned int idx=1; idx < i_nodes.size(); ++idx)
            {
                xd = i_nodes[ idx ].x - i_nodes[ idx-1 ].x;
                yd = i_nodes[ idx ].y - i_nodes[ idx-1 ].y;
                zd = i_nodes[ idx ].z - i_nodes[ idx-1 ].z;
                len += (float)sqrt( xd * xd + yd*yd + zd*zd );
            }
            return len;
        }

        PathNode& operator[](const unsigned int idx) { return i_nodes[idx]; }
        const PathNode& operator()(const unsigned int idx) const { return i_nodes[idx]; }

    protected:
        std::vector<PathNode> i_nodes;
};
#endif
