//////////////////////////////////////////////////////////////////////////
// Part of libKh
// Copyright(C) 2014  Luciano Ciccariello (Xeeynamo)
// 
// This program is free software; you can redistribute it and / or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or(at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.

#pragma once
#include "xstddef.h"

namespace LIBKH
{
	namespace KH2
	{
		class IDX
		{
			static u8 FindKey(const void* data, u32 length);
		public:
			struct Entry
			{
				u32	hash1;				// StringHash1(const char* str)
				u16	hash2;				// StringHash2(const char* str)
				u16 blocksize	: 14;	// file size in blocks + flags
				u16 compressed	: 1;	// if file entry is compressed
				u16 streamed	: 1;	// if file entry is streamed 
				u32	offset;				// offset in blocks
				u32	realsize;			// real size into file entry
			};

			//! \brief compress a portion of memory
			//! \param[in] src data to compress
			//! \param[in] dstSize the new length of compressed data
			//! \param[in] srcSize length of src
			//! \return compressed data allocated with malloc
			static void* Compress(const void* src, u32& dstSize, u32 srcSize);

			//! \brief decompress a portion of memory
			//! \param[in] src data to decompress
			//! \param[in] dstSize length of uncompressed data
			//! \param[in] srcSize length of src
			//! \return uncompressed data allocated with malloc
			static void* Decompress(const void* src, u32 dstSize, u32 srcSize);

			//! \brief calculate a 32-bit hash from a string
			static u32 CalculateHash32(const char* str);

			//! \brief calculate a 16-bit hash from a string
			static u16 CalculateHash16(const char* str);
		};
	}
}