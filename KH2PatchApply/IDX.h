//////////////////////////////////////////////////////////////////////////
// Part of KH2PatchApply
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
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned int	u32;

enum IDXOpenResults
{
	OK		= 0,
	NOIDX	= 0x80000001,
	NOIMG	= 0x80000002,
	INVALID = 0x80000003
};

struct IDXLBA
{
    unsigned int   namehash;	// 0x0
    unsigned short unknow;		// 0x4
    unsigned short blocksize;	// 0x6 if 0x4000 is compressed. File size is size&0x3FFF+1*0x400
    unsigned int   offset;		// 0x8 offset*0x800
    unsigned int   realsize;	// 0xC decompressed size
};
struct IDXOUTLIST
{
	char name[0x34];
	unsigned int compressed;
};

class IDX
{
private:
	IDX *idxInternal;
	int idxInternalCount;
public:
	u32 idxOffset, imgOffset;
	FILE *iso;
	u32 entries;
	IDXLBA *idx;
	int found;

	IDX();
	~IDX();

	bool fseekx(long long offset);
	void LoadIDX(u32 block);
	void SaveIDX(u32 block);
	bool Open(char *isoname, u32 idx, u32 img);
	void Close();

	void *Decompress(void *buffer, unsigned int realsize, u32 blocksize);
	void *Compress(void *buffer, unsigned int decsize, u32 *cmpsize);
	u32   StringHash(char *string);
};