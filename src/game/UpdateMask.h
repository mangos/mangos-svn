/* UpdateMask.h
 *
 * Copyright (C) 2004 Wow Daemon
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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

#ifndef __UPDATEMASK_H
#define __UPDATEMASK_H

#include "UpdateFields.h"

class UpdateMask
{
    public:
        UpdateMask( ) : mUpdateMask( 0 ), mCount( 0 ), mBlocks( 0 ) { }
        UpdateMask( const UpdateMask& mask ) : mUpdateMask( 0 ) { *this = mask; }

        ~UpdateMask( )
        {
            if(mUpdateMask)
                delete [] mUpdateMask;
        mUpdateMask = NULL;
        }

        void SetBit( const uint16 index )
        {
            ASSERT(index < mCount);
            ( (uint8 *)mUpdateMask )[ index >> 3 ] |= 1 << ( index & 0x7 );
            // ( (uint8 *)mUpdateMask )[ index / 8 ] |= 1 * pow( 2, index % 8 );
        }

        void UnsetBit( const uint16 index )
        {
            ASSERT(index < mCount);
            ( (uint8 *)mUpdateMask )[ index >> 3 ] &= (0xff ^ (1 <<  ( index & 0x7 ) ) );
            // ( (uint8 *)mUpdateMask )[ index / 8 ] &= 255 - ( 1 * pow( 2, index % 8 ) ) );
        }

        bool GetBit( const uint16 index ) const
        {
            ASSERT(index < mCount);
            return ( ( (uint8 *)mUpdateMask)[ index >> 3 ] & ( 1 << ( index & 0x7 ) )) != 0;
        }

        uint16 GetBlockCount() const { return mBlocks; }
        uint16 GetLength() const { return mBlocks << 2; }
        uint16 GetCount() const { return mCount; }
        const uint8* GetMask() const { return (uint8*)mUpdateMask; }

        void SetCount(uint16 valuesCount)
        {
            if(mUpdateMask)
                delete [] mUpdateMask;

            mCount = valuesCount;
            // mBlocks = (valuesCount >> 5) + 1;
            mBlocks = (valuesCount + 31) / 32;

            mUpdateMask = new uint32[mBlocks];
            memset(mUpdateMask, 0, mBlocks << 2);
        }

        void Clear()
        {
            if (mUpdateMask)
                memset(mUpdateMask, 0, mBlocks << 2);
        }

        UpdateMask& operator = ( const UpdateMask& mask )
        {
            SetCount(mask.mCount);
            memcpy(mUpdateMask, mask.mUpdateMask, mBlocks << 2);

            return *this;
        }

        void operator &= ( const UpdateMask& mask )
        {
            ASSERT(mask.mCount <= mCount);
            for(int i = 0; i < mBlocks; i++)
                mUpdateMask[i] &= mask.mUpdateMask[i];
        }

        void operator |= ( const UpdateMask& mask )
        {
            ASSERT(mask.mCount <= mCount);
            for(int i = 0; i < mBlocks; i++)
                mUpdateMask[i] |= mask.mUpdateMask[i];
        }

        UpdateMask operator & ( const UpdateMask& mask ) const
        {
            ASSERT(mask.mCount <= mCount);

            UpdateMask newmask;
            newmask = *this;
            newmask &= mask;

            return newmask;
        }

        UpdateMask operator | ( const UpdateMask& mask ) const
        {
            ASSERT(mask.mCount <= mCount);

            UpdateMask newmask;
            newmask = *this;
            newmask |= mask;

            return newmask;
        }

    private:
        uint16 mCount;                            // in values
        uint16 mBlocks;                           // in uint32 blocks
        uint32 *mUpdateMask;
};
#endif
