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

#include "IDX.h"
/*#define wmalloc(size) HeapAlloc(GetProcessHeap(), 0, size)
#define wrealloc(p, size) HeapReAlloc(GetProcessHeap(), 0, p, size)
#define wfree(p) { HeapFree(GetProcessHeap(), 0, p); p = NULL; }*/

unsigned int HexToDec(char* s)
{
    unsigned int res = 0;
    for(int i=7; i>=0; i--, s++)
    {
        unsigned char value;
        if (*s >= '0' && *s <= '9') value = *s-'0';
        else if (*s >= 'A' && *s <= 'F') value = *s-'A'+0xA;
        else if (*s >= 'a' && *s <= 'f') value = *s-'a'+0xA;
#ifdef DEBUG
		else
		{
			printf("LOOP %s", s);
			while(true);
		}
#endif
        res |= (value<<(i*4));
    }
    return res;
}

IDX::IDX()
{
	iso = NULL;
	entries = NULL;
	idx = NULL;
	idxInternal = NULL;

	found = 0;
	idxInternalCount = 0;
}
IDX::~IDX()
{
	Close();
}
bool IDX::fseekx(long long offset)
{
	if (_fseeki64(iso, offset, SEEK_SET) != 0)
	{
FSEEKX_ERROR:
		printf("Unable to fseek in %08X offset. Current: %08X\n", offset, _ftelli64(iso));
		return false;
	}
	if (_ftelli64(iso) != offset)
		goto FSEEKX_ERROR;
	return true;
}
void IDX::LoadIDX(u32 block)
{
	idxOffset = block;
	fseekx(block*0x800);
	fread(&entries, 4, 1, iso);
	if (idx) delete idx;
	idx = new IDXLBA[entries];
	fread(idx, sizeof(IDXLBA), entries, iso);
}
void IDX::SaveIDX(u32 block)
{
	idxOffset = block;
	fseekx(block*0x800 + 4);
	fwrite(idx, sizeof(IDXLBA), entries, iso);
}
bool IDX::Open(char *isoname, u32 idxOffsetParam, u32 imgOffsetParam)
{
	iso = fopen(isoname, "r+b");
	if (!iso) return false;
	imgOffset = imgOffsetParam;
	LoadIDX(idxOffsetParam);
	return true;
}
void IDX::Close()
{
	entries = NULL;
	if (idx) delete idx;
	if (iso) fclose(iso); iso = NULL;
}
void *IDX::Compress(void *buffer, unsigned int decsize, u32 *cmpsize)
{
	u8 *data = (u8*)malloc(decsize);
	u8 *pbuf = (u8*)buffer;
	u8 *pbufmax = pbuf+decsize;
	u8 *pdata = data;

	// Find key
	u8 key;
	int fkey[0x100];
	memset(fkey, 0, 0x400);
	while(pbuf<pbufmax)
		fkey[*pbuf++]++;
	for(int i=1, count = 0x7FFFFFFF; i<0x100; i++)
	{
		if (fkey[i] < count)
		{
			count = fkey[i];
			key = i;
		}
	}

	pbuf = (u8*)buffer;
	int csize;
	for(csize=0 ;pbuf<pbufmax;)
	{
READBYTE:
		u8 d = *pbuf;
		if (d == key)	// Dato uguale alla chiave
		{
			*pdata++ = 0;
			*pdata++ = key;
			csize += 2;
			pbuf++;
			goto READBYTE;
		}
		int mi = 0;
		int matches = 0;
		__int64 farmax = pbuf+0x100 < pbufmax ? 0x100 : pbufmax-pbuf;
		for(int i=1; i<farmax; i++)
		{
			for(int j=i, m=0; j<farmax; j++)
			{
				if (pbuf[j-i] == pbuf[j])
				{
					m++;
					if (j+1 == farmax)
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
			*pdata++ = matches-3;
			*pdata++ = mi;	// Default 1
			*pdata++ = key;
			csize += 3;
			pbuf += matches;
		}
		else
		{
			pbuf++;
			*pdata++ = d;
			csize++;
		}
	}
	*pdata++ = (decsize&0xFF000000)>>24;
	*pdata++ = (decsize&0x00FF0000)>>16;
	*pdata++ = (decsize&0x0000FF00)>> 8;
	*pdata++ = (decsize&0x000000FF)>> 0;
	*pdata++ = key;
	csize += 5;
	*cmpsize = csize;
	return data;
}
u32 IDX::StringHash(char *string)
{
    int c = -1;
    char *check = string;
    while(*check++);
    if (check != string+1)  // La stringa non può essere più piccola di 1 byte
    {
        int stringSize = (int)strlen(string);
        for(int i=0; i<stringSize; i++)
        {
            c ^= string[i] << 24;
            for(int v=0; v<=7; v++)
            {
                if (c < 0)
                    c = 2 * c ^ 0x4C11DB7;
                else
                    c *= 2;
            }
        }
    }
    return -1 - c;
}
void *IDX::Decompress(void *buffer, unsigned int realsize, u32 comsize)
{
	u8 *data = (u8*)malloc(realsize);
	u8 *pbuf = (u8*)buffer + comsize;
	while(!*--pbuf);
	u8 *pdata = data + realsize;
	int key = *pbuf;
	for(pbuf=pbuf-5; (pbuf>buffer) || (pdata>data); pbuf--)
	{
		if (pdata < data) return data;
		if (pbuf - (u8*)buffer < 8)
		{
			data = data;
		}
		u8 d = *pbuf;
		if (d == key)
		{
			int copypos = *(--pbuf);
			if (!copypos)	// Accade quando il byte da copiare è uguale alla chiave
				goto WRITE;
			u8 *pzdata = pdata + copypos;
			int count = *(--pbuf) + 3;
			for(int i=0; i<count; i++)
			{
				if (pzdata >= data)	// Questo permette di non andare underflow nel puntatore
				{
					*--pdata = *(--pzdata);
				}
			}
		}
		else
		{
WRITE:
			*--pdata = d;
		}
	}
	return data;
}
