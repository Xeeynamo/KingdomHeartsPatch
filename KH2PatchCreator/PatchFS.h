//////////////////////////////////////////////////////////////////////////
// Part of KH2PatchCreator
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

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

struct LBA
{
	u32 hash;
	u32 pos;
	u32 sizecmp;
	u32 realsize;
	u32 hasharc;
	u32 relink;
	u32 compressed;
	u32 addnew;
	char reserved[0x3C];
	u32 GetSize()
	{
		return sizecmp;
	}
	bool IsCompressed()
	{
		return compressed == 1;
	}
	void SetSize(int sizeParam, bool compressedParam)
	{
		sizecmp = sizeParam;
		compressed = compressedParam;
	}
};
struct Head
{
	u32 header;
	u32 infolbapos;
	u32 filelbapos;
	u32 version;
	char *GetAuthor()
	{
		return ((char*)&this->header) + 0x10;
	}
	int GetHowFiles()
	{
		return *((int*)((char*)&this->header + filelbapos));
	}
	LBA *GetFilesLBA()
	{
		return ((LBA*)((char*)&this->header + filelbapos + 4));
	}
	void *GetFile(int i)
	{
		return (char*)this + this->GetFilesLBA()[i].pos;
	}
};
struct InfoFS
{
	int pos;
	char *info;
};
struct Info
{
	int how;
	int *pos;
	char *GetString(int index)
	{
		if (index >= how)
			return NULL;
		return (char*)(((int)(&this->pos)) + *((int*)(&pos+index)));
	}
};
struct HeadInfo
{
	u32 posChangelog;
	u32 posThanks;
	u32 posOtherInfo;
	Info changelog;
	Info thanks;
	char *info;

	Info *GetChangelog()
	{
		return (Info*)((char*)(this) + posChangelog);
	}
	Info *GetThanks()
	{
		return (Info*)((char*)(this) + posThanks);
	}
	char *GetOtherInfo()
	{
		return (char*)(this) + posOtherInfo;
	}
};