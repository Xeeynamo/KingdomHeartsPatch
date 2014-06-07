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