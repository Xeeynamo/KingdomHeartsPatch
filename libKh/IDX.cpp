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

#include "IDX.h"
#include <cstdlib>
#include <cstring>

namespace LIBKH
{
	namespace KH2
	{
		u8 IDX::FindKey(const void* data, u32 length)
		{
			// find the proper key
			// it can be found through the finding of less used byte
			const u8* p = (u8*)data;
			const u8* end = p + length;

			u8 key = 0;
			u32 fkey[0x100] {0};

			while (p < end)
				fkey[*p++]++;

			// key must not be 0
			for (u32 i = 1, count = 0x7FFFFFFF; i < 0x100; i++)
			{
				if (fkey[i] < count)
				{
					count = fkey[i];
					key = i;
				}
			}
			return key;
		}

		void* IDX::Compress(const void* src, u32& dstSize, u32 srcSize)
		{
			u8 *dst = (u8*)malloc(srcSize);
			u8 *pSrc = (u8*)src;
			u8 *pSrcEnd = pSrc + srcSize;
			u8 *pDst = dst;
			u8 key = FindKey(src, srcSize);

			pSrc = (u8*)src;
			for (dstSize = 0; pSrc < pSrcEnd;)
			{
			READBYTE:
				u8 d = *pSrc;
				if (d == key)
				{
					*pDst++ = 0;
					*pDst++ = key;
					dstSize += 2;
					pSrc++;
					goto READBYTE;
				}
				int mi = 0;
				int matches = 0;
				__int64 farmax = pSrc + 0x100 < pSrcEnd ? 0x100 : pSrcEnd - pSrc;
				for (int i = 1; i < farmax; i++)
				{
					for (int j = i, m = 0; j < farmax; j++)
					{
						if (pSrc[j - i] == pSrc[j])
						{
							m++;
							if (j + 1 == farmax)
								goto COMPRESS;
						}
						else
						{
						COMPRESS:
							if (matches <= m)
							{
								matches = m;
								mi = i;
							}
							break;
						}
					}
				}
				if (matches > 3)
				{
					*pDst++ = matches - 3;
					*pDst++ = mi;	// Default 1
					*pDst++ = key;
					dstSize += 3;
					pSrc += matches;
				}
				else
				{
					pSrc++;
					*pDst++ = d;
					dstSize++;
				}
			}
			// save the original file size
			*pDst++ = (srcSize & 0xFF000000) >> 24;
			*pDst++ = (srcSize & 0x00FF0000) >> 16;
			*pDst++ = (srcSize & 0x0000FF00) >> 8;
			*pDst++ = (srcSize & 0x000000FF) >> 0;
			*pDst++ = key;
			dstSize += 5;
			return dst;
		}
		void* IDX::Decompress(const void* src, u32 dstSize, u32 srcSize)
		{
			u8 *dst = (u8*)malloc(dstSize);
			u8 *pSrc = (u8*)src + srcSize;
			while (!*--pSrc);
			u8 *pDst = dst + dstSize;
			int key = *pSrc;
			for (pSrc = pSrc - 5; (pSrc > src) || (pDst > dst); pSrc--)
			{
				if (pDst < dst) return dst;
				if (pSrc - (u8*)src < 8)
				{
					dst = dst;
				}
				u8 d = *pSrc;
				if (d == key)
				{
					int copypos = *(--pSrc);
					if (!copypos)	// Accade quando il byte da copiare è uguale alla chiave
						goto WRITE;
					u8 *pzdata = pDst + copypos;
					int count = *(--pSrc) + 3;
					for (int i = 0; i < count; i++)
					{
						if (pzdata >= dst)	// Questo permette di non andare underflow nel puntatore
						{
							*--pDst = *(--pzdata);
						}
					}
				}
				else
				{
				WRITE:
					*--pDst = d;
				}
			}
			return dst;
		}

		u32 IDX::CalculateHash32(const char* str)
		{
			int c = -1;
			int length = strlen(str);

			int strIndex = 0;
			do
			{
				c ^= str[strIndex] << 24;
				for (int i = 8; i > 0; i--)
				{
					if (c < 0)
						c = (c << 1) ^ 0x4C11DB7;
					else
						c <<= 1;
				}
				strIndex++;
			} while (strIndex < length);

			return (u32)~c;
		}
		u16 IDX::CalculateHash16(const char* str)
		{
			int s1 = -1;
			int length = strlen(str);
			while (--length >= 0)
			{
				s1 = (s1 ^ (str[length] << 8)) & 0xFFFF;
				for (int i = 8; i > 0; i--)
				{
					if ((s1 & 0x8000) != 0)
					{
						s1 = ((s1 << 1) ^ 0x1021) & 0xFFFF;
					}
					else
					{
						s1 <<= 1;
					}
				}
			}
			return (u16)~s1;
		}
	}
}