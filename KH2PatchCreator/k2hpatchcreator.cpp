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

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "PatchFS.h"

char stmp[0x800];
char *GetString()
{
	fgets (stmp, 0x800, stdin);
	char *stmpx = stmp;
	do
	{
		if (*stmpx == 0x0A)
		{
			*stmpx = 0x00;
			break;
		}
	} while(*stmpx++);
	return stmp;
}
int GetInt()
{
	fgets (stmp, 0x800, stdin);
	char *stmpx = stmp;
	while(*stmpx)
	{
		if (*++stmpx == 0x0A)
			*stmpx = 0x00;
	}
	return atoi(stmp);
}
int GetFileSize(FILE *f)
{
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);
	return size;
}
u32 StringHash(char *string)
{
	int c = -1;
	char *check = string;
	while(*check++);
	if (check != string+1)
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
void *Compress(void *buffer, unsigned int decsize, u32 *cmpsize)
{
	u8 *data = (u8*)malloc(decsize+5);
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

void Enc(void *data, int size)
{
	const unsigned char enctable[8] = {'X', 0x0C, 0xDD, 'Y', 0xF7, 0x24, 0x7F, 'O'};
	unsigned char *d = (unsigned char*)data;
	while(size--) *d++ ^= enctable[size&7];
}

int main()
{
	Head head;
	HeadInfo hinfo;
	printf("Kingdom Hearts II patch creator\n  Developed by Xeeynamo   rev 27/06/2011\n\n");
	FILE *f = fopen("output.kh2patch", "w+b");
	if (!f)
	{
		printf("Unable to create %s", "output.kh2patch");
		goto END;
	}
	head.header = 0x5032484B;
	printf("Revision of the patch (only numbers): ");
	head.version = GetInt();
	printf("Author: ");
	char *author = GetString();
	//memcpy(head.author, author, strlen(author));
	int strSize = strlen(author) + 1;
	head.infolbapos = sizeof(Head) + strlen(author) + 1;
	fseek(f, sizeof(Head), SEEK_SET);
	fwrite(author, strSize, 1, f);
BESERIOUS_CHANGELOG:
	printf("How changelog strings do you have? ");
	hinfo.changelog.how = GetInt();
	if (hinfo.changelog.how == 666)
	{
		printf("This number scared me lol\n", hinfo.changelog.how);
		goto BESERIOUS_CHANGELOG;
	}
BESERIOUS_HELPS:
	printf("How helps do you received? ");
	hinfo.thanks.how = GetInt();
	if (hinfo.thanks.how > 99)
	{
		printf("%i different sources/person gave you an help? Be serious please xD\n", hinfo.changelog.how);
		goto BESERIOUS_HELPS;
	}
	int fprev, fnext, fssize;
	int pos = 12;
	fseek(f, pos, SEEK_CUR);

	// Write changelog
	printf("Please write the entries of the changelog:\n");
	hinfo.posChangelog = pos;
	hinfo.changelog.pos = new int[hinfo.changelog.how];
	fwrite(&hinfo.changelog.how, 1, sizeof(hinfo.changelog.how), f);
	fprev = ftell(f);
	fssize = hinfo.changelog.how * sizeof(int*);
	pos += fssize;
	fseek(f, fssize, SEEK_CUR);
	for(int i=0, posx = fssize; i<hinfo.changelog.how; i++)
	{
		hinfo.changelog.pos[i] = posx;
		printf("[%2i/%2i] ", i+1, hinfo.changelog.how);
		char *s = GetString();
		int strSize = strlen(s) + 1;
		fwrite(s, strSize, 1, f);
		posx += strSize;
		pos += strSize;
	}
	fnext = ftell(f);
	fseek(f, fprev, SEEK_SET);
	fwrite(hinfo.changelog.pos, hinfo.changelog.how, sizeof(int*), f);
	fseek(f, fnext, SEEK_SET);

	// Write thanks
	printf("Please write the entries of the thanks:\n");
	hinfo.posThanks = pos+4;
	hinfo.thanks.pos = new int[hinfo.thanks.how];
	fwrite(&hinfo.thanks.how, 1, sizeof(hinfo.thanks.how), f);
	fprev = ftell(f);
	fssize = hinfo.thanks.how * sizeof(int*);
	pos += fssize;
	fseek(f, fssize, SEEK_CUR);
	for(int i=0, posx = fssize; i<hinfo.thanks.how; i++)
	{
		hinfo.thanks.pos[i] = posx;
		printf("[%2i/%2i] ", i+1, hinfo.thanks.how);
		char *s = GetString();
		int strSize = strlen(s) + 1;
		fwrite(s, strSize, 1, f);
		posx += strSize;
		pos += strSize;
	}
	fnext = ftell(f);
	fseek(f, fprev, SEEK_SET);
	fwrite(hinfo.thanks.pos, fssize, 1, f);
	fseek(f, fnext, SEEK_SET);

	// Write infos
	printf("Please write the entry the other infos without new lines:\n");
	hinfo.posOtherInfo = pos+8;
	char *s = GetString();
	strSize = strlen(s) + 1;
	fwrite(s, strSize, 1, f);
	head.filelbapos = ftell(f);

	fseek(f, head.infolbapos, SEEK_SET);
	fwrite(&hinfo.posChangelog, 1, sizeof(hinfo.posChangelog), f);
	fwrite(&hinfo.posThanks,    1, sizeof(hinfo.posThanks)   , f);
	fwrite(&hinfo.posOtherInfo, 1, sizeof(hinfo.posOtherInfo), f);
	fseek(f, head.filelbapos, SEEK_SET);

	// Writing the files
	printf("\nHow files do you want to patch? ");
	int lbaheadhowfiles = GetInt();
	fwrite(&lbaheadhowfiles, 1, sizeof(lbaheadhowfiles), f);
	fseek(f, lbaheadhowfiles * sizeof(LBA), SEEK_CUR);
	LBA *lba = new LBA[lbaheadhowfiles];
	for(int i=0; i<lbaheadhowfiles; i++)
	{
		printf("Patch this file: ");
		char path[64];
		strcpy(path, GetString());
		lba[i].hash = StringHash(path);
		printf("Internal archive (like 000tt.idx): ");
		lba[i].hasharc = StringHash(GetString());
		printf("Relink with this file (no text, no relink): ");
		lba[i].relink = StringHash(GetString());
		if (lba[i].relink == 0)
		{
BESERIOUS_COMPRESS:
			printf("Do you want to compress it? (0 = no, 1 = yes) ");
			int selection = GetInt();
			if (selection < 0 || selection > 1)
				goto BESERIOUS_COMPRESS;
			FILE *fIn = fopen(path, "rb");
			if (!fIn)
			{
				printf("Unable to locate %s.\nThe creation of the patch will be interrupted.", path);
				goto END;
			}
			u32 fInSize = GetFileSize(fIn);
			void *data = malloc(fInSize);
			fread(data, fInSize, 1, fIn);
			fclose(fIn);
			lba[i].realsize = fInSize;
			if (selection)
				data = Compress(data, lba[i].realsize, &fInSize);
			lba[i].SetSize(fInSize, selection);
			lba[i].pos = ftell(f);
			fwrite(data, lba[i].sizecmp, 1, f);
			free(data);
		}
	}
	fseek(f, head.filelbapos + 4, SEEK_SET);
	fwrite(lba, lbaheadhowfiles, sizeof(LBA), f);
	fseek(f, 0, SEEK_SET);
	fwrite(&head, sizeof(Head), 1, f);
	fclose(f);

	// Encrypt
	f = fopen("output.kh2patch", "r+b");
	int size = GetFileSize(f);
	void *data = malloc(size);
	fseek(f, 0, SEEK_SET);
	fread(data, 1, size, f);
	Enc(data, size);
	fseek(f, 0, SEEK_SET);
	fwrite(data, 1, size, f);
	free(data);
	fclose(f);
	
END:
	printf("\nFinish :D");
	while(true) Sleep(100);
}