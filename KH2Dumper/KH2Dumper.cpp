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

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "xstddef.h"
#include "IDX.h"
#include "Extra.h"
#define PATHLEN 0x100
#define Key(i) (GetAsyncKeyState(i)==-32767)

#define P_NONAME	((param&0x01)==0x01)
#define P_MAKELIST	((param&0x02)==0x02)
#define P_COMPNO	((param&0x04)==0x04)
#define P_COMPALL	((param&0x08)==0x08)

struct OBJ
{
	unsigned int	id;
	unsigned int	param;
	signed char		namemdl;
	signed char		nameset;
	unsigned int	param1;
	unsigned int	param2;
	unsigned int	param3;
	unsigned int	param4;
	unsigned int	param5;
	unsigned int	param6;

};

void SetXY(u8 x, u8 y)
{
	COORD pos = {x, y};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

int main(int argc, char *argv[])
{
	bool silent = false;
	char archive[MAX_PATH] = "KH2";
	char *narc = archive;
	if (argc >= 2)
	{
		while(*++argv[1]);
		while(*--argv[1]) *argv[1] != '\\';
		while(*++argv[1] != '.')
			*narc++ = *argv[1];
	}
	if (argc == 3) silent = true;

	printf("\n KH2DumperX   v1.0.1 05/07/2011\n");
	printf(" Developed by Xeeynamo\n");
	printf(" http://xeeynamo.blogspot.com\n");
	for(int i=0; i<60; i++) printf("\xCD");
	printf("\n\n");

	IDX idx;
	int selection = 0;
	int selected = 0;
	u8 param = 3;

	if (silent) goto PROGRAM;

// ==============================
// ===========  MENU  ===========
// ==============================
	printf("  >  Start unpack\n");
	printf("  >  Start repack\n");
	printf(" [ ] Extract nonames\t(UNPACK)\n");
	printf(" [ ] Create list.txt\t(UNPACK)\n");
	printf(" [ ] No compression\t(REPACK)\n");
	printf(" [ ] Compress all\t(REPACK)\n");
CHECKBOX:
	for(int i=0; i<8; i++)
	{
		SetXY(2, 8+i);
		if (param & (1<<i))
			printf("X");
		else
			printf(" ");
	}
CURSOR:
	SetXY(2, 6+selection);
	do
	{
		Sleep(16);
		if (Key(VK_UP))
		{
			if (selection > 0) selection--;
			goto CURSOR;
		}
		else if (Key(VK_DOWN))
		{
			if (selection <= 6) selection++;
			goto CURSOR;
		}
		else if (Key(VK_RETURN) || Key(VK_SPACE))
		{
			if (selection >= 2)
				param ^=  (1<<(selection-2));
			else
				selected = selection+1;
			goto CHECKBOX;
		}
	} while(!selected);
	SetXY(0, 16);
// ==============================
// =========  END MENU  =========
// ==============================

	int time_start = GetTickCount();
	u32 result;
	if (selected == 1)
	{
PROGRAM:
		idx.CreateList(P_MAKELIST, archive);
		result = idx.Open(archive);
		if (result == OK)
		{
			ExtractEverything(&idx);
			ExtractMDLX(&idx, "export/00objentry.bin");
			ExtractBATTLE(&idx, "export/00battle.bin");
			ExtractSYSTEM(&idx, "export/00system.bin");
			ExtractSYSTEM(&idx, "export/03system.bin");
			if (P_NONAME) idx.ExtractRemains();

			printf("Extracted %i/%i. %i files with an unknow name.\n", idx.found, idx.GetEntries(), idx.GetEntries() - idx.found);
		}
		else
			goto LOC_ERROR;
	}
	if (selected == 2)
	{
		char path[MAX_PATH];
		for(int w=0; w<dumpHowWorlds; w++)
		{
			sprintf(path, "000%s.txt", dumpWorlds[w]);
			SetConsoleTitle(path);
			FILE *fList = fopen(path, "r+b");
			if (fList)
			{
				fseek(fList, 0, SEEK_END);
				int entries = ftell(fList) / sizeof(IDXOUTLIST);
				fseek(fList, 0, SEEK_SET);
				IDXOUTLIST *list = (IDXOUTLIST*)malloc(entries * sizeof(IDXOUTLIST));
				fread(list, sizeof(IDXOUTLIST), entries, fList);
				fclose(fList);

				sprintf(path, "export/000%s", dumpWorlds[w]);
				result = idx.Create(path, entries);
				if (result != OK) goto LOC_ERROR;
				for(int i=0; i<entries; i++)
				{
					printf("[%5i/%5i %s] ", i, entries, dumpWorlds[w]);
					bool compressed = list[i].compressed == 1;
					if (P_COMPNO)  list[i].compressed = false;
					if (P_COMPALL) list[i].compressed = true;
					if (list[i].compressed) printf("C ");
					else printf("D ");
					if (idx.AddFile(list[i].name, compressed))
					{
						printf("Created %s\n", list[i].name);
					}
					else
					{
						printf("Unable to create %s\n", list[i].name);
					}
				}
				idx.Save();
				fclose(fList);
			}
		}
		SetConsoleTitle("sys.txt");
		FILE *fList = fopen("sys.txt", "r+b");
		if (fList)
		{
			fseek(fList, 0, SEEK_END);
			int entries = ftell(fList) / sizeof(IDXOUTLIST);
			fseek(fList, 0, SEEK_SET);
			IDXOUTLIST *list = (IDXOUTLIST*)malloc(entries * sizeof(IDXOUTLIST));
			fread(list, sizeof(IDXOUTLIST), entries, fList);
			fclose(fList);

			result = idx.Create("KH2NEW", entries);
			if (result != OK) goto LOC_ERROR;
			for(int i=0; i<entries; i++)
			{
				printf("[%5i/%5i] ", i, entries);
				bool compressed = list[i].compressed == 1;
				if (P_COMPNO)  list[i].compressed = false;
				if (P_COMPALL) list[i].compressed = true;
				if (list[i].compressed) printf("C ");
				else printf("D ");
				if (idx.AddFile(list[i].name, compressed))
				{
					printf("Created %s\n", list[i].name);
				}
				else
				{
					printf("Unable to create %s\n", list[i].name);
				}
			}
			idx.Save();
		}
	}
	if (0 == 1)
	{
LOC_ERROR:
		switch (result)
		{
		case NOIDX:
			printf("Unable to open %s.idx.\n", archive);
			break;
		case NOIMG:
			printf("Unable to open %s.img.\n", archive);
			break;
		case INVALID:
			printf("%s.idx is an corrupted index archive.\n", archive);
			break;
		}
	}
	int time_end = GetTickCount();
	printf("Done in %i seconds\n", (time_end-time_start)/1000);

	if (!silent)
	{
		printf("\nYou can close me pressing the red X button at top-left :).\nBye and thanks for using me!\n");
		while(true) Sleep(100);
	}
}