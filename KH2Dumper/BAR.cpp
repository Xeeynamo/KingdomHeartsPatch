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

#include "BAR.h"

#define TO16(i) ((u16*)(i))
#define TO32(i) ((u32*)(i))


void _LLRemoveExt(char *src, char *dst)
{
	size_t pos = 0;
	char *tmpsrc = src;
	while(*tmpsrc)
	{
		if (*tmpsrc++ == '.')
			pos = tmpsrc-src;
	}
	memcpy(dst, src, pos-1);
	dst[pos-1] = 0;
}
bool _LLFileSeek(HANDLE handle, u32 offset)
{
	return SetFilePointer(handle, offset, 0, FILE_BEGIN) != INVALID_SET_FILE_POINTER ? true : false;
}

bool _LLFileRead(HANDLE handle, LPVOID buffer, u32 bytestoread)
{
	DWORD bytesreaded;
	if (!ReadFile(handle, buffer, bytestoread, &bytesreaded, NULL))
		return false;
	if (bytestoread != bytesreaded)
		return false;
	return true;
}
bool _LLFileWrite(HANDLE handle, LPVOID buffer, u32 bytestowrite)
{
	DWORD byteswriteed;
	if (!WriteFile(handle, buffer, bytestowrite, &byteswriteed, NULL))
		return false;
	if (bytestowrite != byteswriteed)
		return false;
	return true;
}
u32 _LLGetStringSize(LPBYTE text)
{
	for(int i=0; ; i++)
		if (*text++ == 0)
			return i+1;
}

BAR::BAR(void)
{
	hFile = NULL;
	fileSize = NULL;
	data = NULL;
	head = NULL;
	lba  = NULL;
}
BAR::~BAR()
{
	Close();
	if (data) HeapFree(GetProcessHeap(), NULL, data);
}
bool BAR::Open(LPTSTR filename)
{
	Close();
	hFile = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;
	fileSize = GetFileSize(hFile, NULL);
	data = (LPBYTE)HeapAlloc(GetProcessHeap(), NULL, fileSize);
	_LLFileSeek(hFile, 0);
	if (!_LLFileRead(hFile, data, fileSize))
	{
		Close();
		return false;
	}
	if (*TO32(data) != 0x01524142)
	{
		Close();
		return false;
	}
	head = (Head*)data;
	lba  = (LBA *)(data+0x10);
	fileName = filename;
	return true;
}
bool BAR::Create(LPTSTR filename)
{
	return false;
}
void BAR::Save()
{
	if (fileName == NULL)
		return;
	CloseHandle(hFile);
	hFile = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	_LLFileSeek(hFile, 0);
	_LLFileWrite(hFile, data, fileSize);
}
void BAR::Close()
{
	if (hFile) CloseHandle(hFile);
	hFile = NULL;
	fileSize = NULL;
	data = NULL;
	head = NULL;
	lba  = NULL;
}
int  BAR::GetIndex(char *name)
{
	for(u32 i=0; i<head->howFiles; i++)
	{
		if (lba[i].name == *(unsigned int*)name)
			return i;
	}
	return -1;
}
void *BAR::GetPointer(int index)
{
	return data+lba[index].position;
}

bool BAR::EditName(u32 index, char *name)
{
	if (index >= head->howFiles) return false;
	memcpy(&lba[index].name, name, 4);
	return true;
}
bool BAR::Import(u32 index, char *name)
{
	if (index >= head->howFiles) return false;
	HANDLE hIn = CreateFile(name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIn == INVALID_HANDLE_VALUE) return false;
	int fInSize = GetFileSize(hIn, NULL);
	if (fInSize&0xF) fInSize += (0x10-(fInSize&0xF));
	void *buf = HeapAlloc(GetProcessHeap(), NULL, fInSize);
	_LLFileRead(hIn, buf, fInSize);
	if (fInSize != fileSize)
	{
		int difference = lba[index].size - fileSize;
		if (index != head->howFiles-1)
		{
			void *tmp = HeapAlloc(GetProcessHeap(), 0, fileSize - lba[index+1].position);
			memcpy(tmp, data + lba[index+1].position, fileSize - lba[index+1].position);
			HeapReAlloc(GetProcessHeap(), 0, data, fileSize + difference);
			memcpy(data + lba[index+1].position + difference, tmp, fileSize - lba[index+1].position);
		}
		else
			HeapReAlloc(GetProcessHeap(), 0, data, fileSize + difference);
		memcpy(data + lba[index].position, buf, fInSize);
		for(u32 i=index+1; i<head->howFiles; i++)
		{
			lba[index].position += difference;
		}
		lba[index].size = fInSize;
		fileSize += difference;
	}
	else
	{
		memcpy(data + lba[index].position, buf, fInSize);
	}
	return true;
	
}
bool BAR::Export(u32 index)
{
	if (index >= head->howFiles) return false;
	char outpath[MAX_PATH];
	char noext[MAX_PATH];
	char filename[5];
	_LLRemoveExt(fileName, noext);
	memcpy(filename, &lba[index].name, 4);
	filename[4] = 0;
	sprintf_s(outpath, MAX_PATH, "%s\\%s_%2X_%02i.bin", noext, filename, lba[index].unknow, index);
	CreateDirectory(noext, NULL);
	return Export(index, outpath);
}
bool BAR::Export(u32 index, char *name)
{
	if (index >= head->howFiles) return false;
	HANDLE hOut = CreateFile(name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	int asd = GetLastError();
	if (hOut == INVALID_HANDLE_VALUE) return false;
	void *buf = HeapAlloc(GetProcessHeap(), NULL, lba[index].size);
	_LLFileSeek(hFile, lba[index].position);
	_LLFileRead(hFile, buf, lba[index].size);
	_LLFileWrite(hOut, buf, lba[index].size);
	HeapFree(GetProcessHeap(), NULL, buf);
	CloseHandle(hOut);
	return true;
}
bool BAR::Add(u32 index)
{
	if (index >= head->howFiles) return false;
	HeapReAlloc(GetProcessHeap(), NULL, data, fileSize+0x10);
	memmove(&lba[index]+0x10, &lba[index], fileSize - (/*data-&lba[index]*/index*0x10 + 0x10));
	fileSize += 0x10;
	lba[index].name = 0;
	if (index != head->howFiles-1) lba[index].position = lba[index+1].position;
	else lba[index+1].position = fileSize;
	lba[index].size = 0;
	lba[index].unknow = 1;
	return true;
}
bool BAR::Remove(u32 index)
{
	if (index >= head->howFiles) return false;
	return false;
}