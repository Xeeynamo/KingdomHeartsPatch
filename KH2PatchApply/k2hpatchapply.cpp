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

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "..\KH2PatchCreator\PatchFS.h"
#include "IDX.h"

char stmp[MAX_PATH];
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

int GetFileSize(FILE *f)
{
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);
	return size;
}

struct EmptySpace
{
	int pos;
	int size;
	EmptySpace *space;
};

bool fseekx(FILE *f, long long offset)
{
	if (_fseeki64(f, offset, SEEK_SET) != 0)
	{
FSEEKX_ERROR:
		printf("Unable to fseek in %08X offset. Current: %08X\n", offset, ftell(f));
		return false;
	}
	if (_ftelli64(f) != offset)
		goto FSEEKX_ERROR;
	return true;
}

bool WriteEmptyBlock(FILE *f, int offset, int size)
{
	void *tmpidxdata = malloc(size);
	memset(tmpidxdata, 0, size);
	fseekx(f, (long long)offset * 0x800);
	fwrite(tmpidxdata, size, 1, f);
	fflush(f);
	return true;
}
bool WriteEmptyBlock(IDX *idx, u32 index)
{
	if (idx->entries <= index)
		return false;
	u32 blockrealsize = (idx->idx[index].blocksize&0x3FFF) * 0x800 + 0x800;
	WriteEmptyBlock(idx->iso, idx->imgOffset + idx->idx[index].offset, blockrealsize);
	return true;
}
bool WriteBlock(IDX *idx, int index, void *data, int offset, int compressedsize, int realsize, bool isCompressed)
{
	if (idx->idx[index].offset != offset)
		WriteEmptyBlock(idx, index);
	idx->idx[index].offset = offset;
	idx->idx[index].realsize = realsize;
	idx->idx[index].blocksize = (compressedsize + (0x800-(compressedsize%0x800)))/0x800 - 1;
	if (isCompressed)
		idx->idx[index].blocksize |= 0x4000;
	fseekx(idx->iso, (idx->imgOffset + ((long long)offset)) * 0x800);
	fwrite(data, compressedsize, 1, idx->iso);
	fflush(idx->iso);
	fseekx(idx->iso, (idx->idxOffset*0x800) + 4);
	fwrite(idx->idx, idx->entries, sizeof(IDXLBA), idx->iso);
	return true;
}
bool WriteBlockSpecial(IDX *idx, int index, void *data, int offset, int compressedsize, int realsize, bool isCompressed)
{
	idx->idx[index].offset = offset;
	idx->idx[index].realsize = realsize;
	idx->idx[index].blocksize = (compressedsize + (0x800-(compressedsize%0x800)))/0x800 - 1;
	if (isCompressed)
		idx->idx[index].blocksize |= 0x4000;
	fseekx(idx->iso, (idx->imgOffset + ((long long)offset)) * 0x800);
	fwrite(data, compressedsize, 1, idx->iso);
	fflush(idx->iso);
	fseekx(idx->iso, (idx->idxOffset*0x800) + 4);
	fwrite(idx->idx, idx->entries, sizeof(IDXLBA), idx->iso);
	return true;
}

void Dec(void *data, int size)
{
	const unsigned char enctable[8] = {'X', 0x0C, 0xDD, 'Y', 0xF7, 0x24, 0x7F, 'O'};
	unsigned char *d = (unsigned char*)data;
	while(size--) *d++ ^= enctable[size&7];
}
int main(int argc, char *argv[])
{
	IDX idx;
	const int isolbaIDX = 1840;
	const int isolbaIMG = 671693;
	printf("Kingdom Hearts II patcher   rev4 05/07/2011\n  Developed by Xeeynamo\n  http://xeeynamo.blogspot.com\n\n");

	if (argc != 2)
	{
		printf("Please start the program dragging into the executable the patch for\nKingdom Hearts II.\n");
		goto END;
	}

	FILE *f = fopen(argv[1], "r+b");
	if (!f)
	{
		printf("Unable to open %s\n", argv[1]);
		goto END;
	}
	int fsize = GetFileSize(f);
	void *data = malloc(fsize);
	fread(data, fsize, 1, f);
	Dec(data, fsize);

	Head *patchHead = (Head*)data;
	if (patchHead->header != 0x5032484B)
	{
		printf("%s is an invalid patch of Kingdom Hearts II\n", argv[1]);
		goto END;
	}
	HeadInfo *patchInfo = (HeadInfo*)((char*)(data) + patchHead->infolbapos);

	printf("Info from the patch:\n");
	printf("Author: %s\n", patchHead->GetAuthor());
	printf("Revision: %i\n", patchHead->version);
	Sleep(2000);

	printf("\nChangelog:\n");
	for(int i=0; i<patchInfo->GetChangelog()->how; i++)
	{
		printf(" + %s\n", patchInfo->GetChangelog()->GetString(i));
	}
	printf("\nOther info:\n");
	printf("  %s\n\n", patchInfo->GetOtherInfo());

	printf("\nNow please specific the name of the ISO of Kingdom Hearts II Final Mix.\nAfter pressing the RETURN key the ISO will be edited, so be sure to create a\nbackup of your ISO in case that your original disc is ruined.\n");
	printf("ISO: ");
	char *isopath = GetString();

	if (!idx.Open(isopath, isolbaIDX, isolbaIMG))
	{
		printf("Unable to open the ISO %s", isopath);
		goto END;
	}
	int patchHowFiles = patchHead->GetHowFiles();
	LBA *patchlba = patchHead->GetFilesLBA();
	EmptySpace *space = new EmptySpace;
	space->space = NULL;
	u32 dummyID;
	for(u32 j=0; j<idx.entries; j++)
	{
		if (idx.idx[j].namehash == 0x10303F6F)
		{
			dummyID = j;
			WriteEmptyBlock(&idx, j);
			/*if (idx.idx[j].realsize > 0)
			{
				idx.idx[j].realsize = 0;
				idx.idx[j].blocksize += 0x1000;
			}*/
			break;
		}
	}
	for(int i=0; i<patchHowFiles; i++)
	{
		printf("\rPatching %3i/%3i", i+1, patchHowFiles);
		u32 idxarcOffset = isolbaIDX;
		if (patchlba[i].hasharc != 0)
		{
			for(u32 j=0; j<idx.entries; j++)
			{
				if (patchlba[i].hasharc == idx.idx[j].namehash)
				{
					idxarcOffset = isolbaIMG + idx.idx[j].offset;
					idx.LoadIDX(idxarcOffset);
					goto GOGOGO;
				}
			}
			printf("Unable to find the internal archive %08X in %i\n", patchlba[i].hasharc, i);
			goto END;
		}
GOGOGO:
		for(u32 j=0; j<idx.entries; j++)
		{
			if (patchlba[i].hash == idx.idx[j].namehash)
			{
				if (patchlba[i].relink != 0)
				{
					for(u32 k=0; k<idx.entries; k++)
					{
						if (patchlba[i].relink == idx.idx[j].namehash)
						{
							idx.idx[j].unknow = idx.idx[k].unknow;
							idx.idx[j].blocksize = idx.idx[k].blocksize;
							idx.idx[j].offset = idx.idx[k].offset;
							idx.idx[j].realsize = idx.idx[k].realsize;
							idx.SaveIDX(idxarcOffset);
						}
					}
					goto CONTINUE;
				}
				else
				{
					int blocksize = idx.idx[j].blocksize&0x3FFF;
					int blocksizeunc = blocksize * 0x800 + 0x800;
					int newblocksize = (patchlba[i].sizecmp + (0x800-(patchlba[i].sizecmp%0x800)))/0x800 - 1;
					WriteEmptyBlock(&idx, j);
					if (newblocksize <= blocksize)
					{
						WriteBlock(&idx, j, patchHead->GetFile(i),idx.idx[j].offset, patchlba[i].sizecmp, patchlba[i].realsize, patchlba[i].IsCompressed());
						if (newblocksize < blocksize)
						{
							EmptySpace *spx = space;
							while(true)
							{
								if (spx->space != 0)
									spx = spx->space;
								else
									break;
							}
							spx->size = blocksize - newblocksize;
							spx->pos = idx.idx[j].offset + newblocksize + 1;
							spx->space = new EmptySpace;
							spx->space->pos = 0;
							spx->space->size = 0;
							spx->space->space = 0;
						}
					}
					else
					{
						EmptySpace *spx = space;
						while(true)
						{
							if (spx->space != 0)
								spx = spx->space;
							else
								break;
						}
						spx->size = blocksize;
						spx->pos = idx.idx[j].offset;
						spx->space = new EmptySpace;
						spx->space->pos = 0;
						spx->space->size = 0;
						spx->space->space = 0;

						spx = space;
						while(true)
						{
							if (spx->space != 0)
							{
								if (newblocksize <= spx->size)
								{
									WriteBlock(&idx, j, patchHead->GetFile(i), spx->pos, patchlba[i].sizecmp, patchlba[i].realsize, (bool)patchlba[i].compressed);
									spx->pos = idx.idx[j].offset + newblocksize + 1;
									spx->size -= newblocksize;
									break;
								}
								else
									spx = spx->space;
							}
							else
							{
								idx.LoadIDX(isolbaIDX);
								if (newblocksize <= idx.idx[dummyID].blocksize)
								{
									u32 newoffset = idx.idx[dummyID].offset;
									idx.LoadIDX(idxarcOffset);
									WriteBlockSpecial(&idx, j, patchHead->GetFile(i), newoffset, patchlba[i].sizecmp, patchlba[i].realsize, (bool)patchlba[i].compressed);
									idx.LoadIDX(isolbaIDX);
									idx.idx[dummyID].blocksize -= newblocksize;
									idx.idx[dummyID].offset += newblocksize + 1;
									idx.idx[dummyID].realsize = 0;
									idx.SaveIDX(isolbaIDX);
								}
								else
									printf("  OVERFLOW ERROR: File not written\n");
								break;
							}
						}
						spx = space;
						while(true)
						{
							if (spx->space != 0)
								spx = spx->space;
							else
								break;
						}
					}
					goto CONTINUE;
				}
			}
		}
		printf("Unable to find the file %08X in %i\n", patchlba[i].hash, i);
		goto END;
CONTINUE:
		if (patchlba[i].hasharc != 0)
			idx.LoadIDX(isolbaIDX);
		if (patchlba[i].hasharc != 0)
		{
			//fseekx(w->iso, tmpOffset * 0x800 + 4);
			//fread(w->idx, tmpRealsize, 1, w->iso);
		}
	};
	printf("\nGame patched without errors ;)\n");

	printf("\nThanks to:\n");
	for(int i=0; i<patchInfo->GetThanks()->how; i++)
	{
		printf(" - %s\n", patchInfo->GetThanks()->GetString(i));
	}
END:
	printf("\n\nNothing's more to do, you can exit clicking the red X button on top-right\nof this window. If you have problems, please report them to\nxeeynamo-support@hotmail.com with KH2PATCHERR as subject. If you want to create\na patch with this software, conctact me to xeeynamo-support@hotmail.com with\nKH2MAKEPATCH as subject. Other mails with other subjects or contents\nthat will not match will be completely ignored.");
	while(true); Sleep(100);
}