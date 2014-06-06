//////////////////////////////////////////////////////////////////////////
// Part of KH2Dumper
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

typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned int	u32;

bool _LLFileSeek(HANDLE, u32 offset);
bool _LLFileRead(HANDLE, LPVOID buffer, u32 bytestoread);
bool _LLFileWrite(HANDLE, LPVOID buffer, u32 bytestowrite);

struct Head
{
	u32 header;
	u32 howFiles;
	u32 dummy1;
	u32 dummy2;
};
struct LBA
{
	u32 unknow;
	u32 name;
	u32 position;
	u32 size;
};

class BAR
{
private:
	HANDLE hFile;
	LPTSTR fileName;
	u32 fileSize;
	int enc;

public:
	Head *head;
	LBA  *lba;
	u8 *data;

	BAR();
	~BAR();

	bool Open(LPTSTR filename);
	bool Create(LPTSTR filename);
	void Save();
	void Close();

	int  GetIndex(char *name);
	void *GetPointer(int index);

	bool EditName(u32 index, char *name);
	bool Import(u32 index, char *name);
	bool Export(u32 index);
	bool Export(u32 index, char *name);
	bool Add(u32 index);
	bool Remove(u32 index);
};
