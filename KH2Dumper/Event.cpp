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

#include "Event.h"

u16 param0;
u16 param1;

Event::Event()
{
	data = NULL;
	data16 = NULL;
	size = 0;

	param0 = 0;
	param1 = 0;
}
Event::~Event()
{
	//
}

bool Event::Load(void *buffer, int fsize)
{
	if (!buffer) return false;
	data = (u8*)buffer;
	data16 = (u16*)buffer;
	if (*data16 == 0x0018)
		data16 += 0x18/sizeof(short);
	size = fsize;
	return true;
}

char *Event::GetFile(int *type)
{
	if ((int)data16-(int)data >= size)
		return (char*)1;
	char *r;

	if (param0)
	{
		switch(param0)
		{
		case 0x23:
LOC_PARAM23:
			if (param1 > 0)
			{
				param1--;
				r = (char*)data16;
				r++;
				data16 += 0x20/sizeof(short);
			}
			else
			{
				param0 = 0;
				r = NULL;
			}
			break;
		}
		return r;
	}

	switch(*data16++)
	{
	case 0x15:	// MSN/%s(LANG)/%s(ID).bar
		*type = 0x15;
		data16++;	// Always 0x09
		data16++;	// Unknow
		data16++;	// Unknow
		return (char*)data16;

	case 0x23:	// Group of voices
		param0 = 0x23;
		param1 = *data16++;	// Count
		data16++;	// Ever 0
		data16++;	// Ever 0
		data16++;	// Ever 0
		data16++;	// Unknow
		goto LOC_PARAM23;
		break;

	case 0x25:	// anm_tt/ETT99701E
		*type = 0x25;
		data16++;	// Unknow... 0x1C
		data16++;	// ID_1
		data16++;	// ID_2
		data16++;	// Unknow flag
		r = (char*)data16;
		data16 += 0x12/sizeof(short);
		break;

	case 0x26:	// voice/%s/event/%s.vag
		*type = 0x26;
		data16++;	// Maybe 0x0E ever
		r = (char*)data16;
		data16 += 0x0A/sizeof(short);
		if (strcmp(r, "es99503rk") == 0)
		{
			r = r;
		}
		break;

	case 0x10:
	case 0x38:	// Animation folder
		*type = 0x38;
		data16++;	// Unknow... 0x10, 0x01
		data16++;	// ID_1 - Unknow... Maybe index: 0x005A, 0x051D, 0x051E, 0x051F, 0x023B
		data16++;	// ID_2 - Unknow... 0, 2, 4, 6
		r = (char*)data16;
		data16 += 0x08/sizeof(short);
		break;

	case 0x3D:	// event/layout/%s/%s.2ld		event/layout/jp/tt_206s_1.2ld
		data16++;	// 2 bytes more than 0x3E
	case 0x3E:	// event/layout/%s/%s.2ld		event/layout/jp/tt_206s_1.2ld
		*type = 0x3E;
		data16++;	// Maybe 0x0E ever
		r = (char*)data16;
		data16 += 0x10/sizeof(short);
		break;

	default:
		*type = 0;
		r = NULL;
	}
	return r;
}