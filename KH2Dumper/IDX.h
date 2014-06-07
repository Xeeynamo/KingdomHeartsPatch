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
#include "xstddef.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "../libKh/IDX.h"

enum IDXOpenResults
{
	OK		= 0,
	NOIDX	= 0x80000001,
	NOIMG	= 0x80000002,
	INVALID = 0x80000003
};

struct IDXOUTLIST
{
	char name[0x34];
	unsigned int compressed;
};

class IDX
{
private:
	HANDLE heap;
	IDX *idxInternal;
	int idxInternalCount;

	// REPACK VARs
	__int64 currentOffset;

	FILE *fList;
	bool createlist;

	bool CreateSubDirs(char *path);
	u32 ExtractFileLL(void **buffer, int *size, char *fileName);
public:
	FILE *fIDX;
	FILE *fIMG;
	u32 entries;
	LIBKH::KH2::IDX::Entry *idx;
	int found;

	IDX();
	~IDX();

	u32   Create(char *archiveName, int entries);
	u32   Open(char *archiveName);
	u32   Open(void *idx, FILE *img);
	void  Save();
	void  Close();
	void  CreateList(bool, char *archiveName);

	bool  AddFile(char *file, bool compress);

	u32   GetEntries();
	u32   GetStringIndex(char *fileName);
	void  AddIDX(char *filename);
	bool  ExtractFile(char *filename);
	void  ExtractRemains();
};