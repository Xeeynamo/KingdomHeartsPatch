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
#include "xstddef.h"
#include <windows.h>
#include <stdio.h>

struct EventHeader
{
	u32 head;
	u16 unknow1;
	u16 unknow2;
	u16 unknow3;
	char name[0x0C];
};

class Event
{
private:
	int size;
	u8 *data;
	u16 *data16;
public:
	Event();
	~Event();

	bool Load(void *buffer, int fsize);
	char *GetFile(int *type);
};