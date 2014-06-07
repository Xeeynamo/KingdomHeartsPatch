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

#include "IDX.h"

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
	entries = NULL;
	idx = NULL;
	fIDX = NULL;
	fIMG = NULL;
	idxInternal = NULL;

	found = 0;
	createlist = false;
	idxInternalCount = 0;
	currentOffset = 0;

	heap = GetProcessHeap();
}
IDX::~IDX()
{
	//
}

u32 IDX::Create(char *archiveName, int howentries)
{
	char realarchivenames[MAX_PATH];
	sprintf(realarchivenames, "%s.idx", archiveName);
	if (!(fIDX = fopen(realarchivenames, "w+b"))) return 0x80000001;
	sprintf(realarchivenames, "KH2NEW.img");
	if (currentOffset == 0)
		if (!(fIMG = fopen(realarchivenames, "w+b"))) return 0x80000002;

	entries = 0;
	idx = (LIBKH::KH2::IDX::Entry*)malloc(howentries * sizeof(LIBKH::KH2::IDX::Entry));
	return 0;
}
u32 IDX::Open(char *archiveName)
{
	char realarchivenames[MAX_PATH];
	sprintf(realarchivenames, "%s.idx", archiveName);
	if (!(fIDX = fopen(realarchivenames, "rb"))) return 0x80000001;
	sprintf(realarchivenames, "KH2.img", archiveName);
	if (!(fIMG = fopen(realarchivenames, "rb"))) return 0x80000002;

	fseek(fIDX, 0, SEEK_END);
	int fIDXSize = ftell(fIDX);
	fseek(fIDX, 0, SEEK_SET);

	fread(&entries, 4, 1, fIDX);
	if (fIDXSize != entries*sizeof(LIBKH::KH2::IDX::Entry) + 4)
	{
		Close();
		return 0x80000003;
	}
	found = 0;

	idx = (LIBKH::KH2::IDX::Entry*)malloc(entries * sizeof(LIBKH::KH2::IDX::Entry));
	fread(idx, sizeof(LIBKH::KH2::IDX::Entry), entries, fIDX);
	return 0;
}
u32 IDX::Open(void *data, FILE *img)
{
	entries = *((int*)data);
	idx = (LIBKH::KH2::IDX::Entry*)(((int*)data) + 1);
	return 0;
}
void IDX::Save()
{
	fseek(fIDX, 0, SEEK_SET);
	fwrite(&entries, 4, 1, fIDX);
	fwrite(idx, sizeof(LIBKH::KH2::IDX::Entry), entries, fIDX);
	free(idx);
	fflush(fIDX);
	fflush(fIMG);
	fclose(fIDX);
}
void IDX::Close()
{
	entries = NULL;
	if (idx) free(idx); idx = NULL;
	if (fIDX) fclose(fIDX); fIDX = NULL;
	if (fIMG) fclose(fIMG); fIMG = NULL;
}
void IDX::CreateList(bool list, char *archiveName)
{
	if (createlist != list)
	{
		if (createlist)
		{
			fclose(fList);
		}
		else
		{
			char path[MAX_PATH];
			sprintf(path, "%s.txt", archiveName);
			fList = fopen(path, "w+b");
		}
		createlist = list;
	}
}

bool IDX::AddFile(char *filename, bool compress)
{
	char path[MAX_PATH];
	sprintf(path, "export/%s", filename);
	FILE *f = fopen(path, "r+b");
	if (!f) return false;

	fseek(f, 0, SEEK_END);
	int fSize = ftell(f);
	fseek(f, 0, SEEK_SET);

	void *data = malloc(fSize);
	fread(data, 1, fSize, f);
	fclose(f);

	idx[entries].hash1 = LIBKH::KH2::IDX::CalculateHash32(filename);
	idx[entries].hash2 = LIBKH::KH2::IDX::CalculateHash16(filename);
	idx[entries].realsize = fSize;
    if (filename[0] == '@')
	{
		char *fnamex = filename;
		while(*fnamex++);
		while(*--fnamex != '/');
		idx[entries].hash1 = HexToDec(++fnamex);
	}
	if (compress)
	{
		u32 compressSize;
		void *compress = LIBKH::KH2::IDX::Compress(data, compressSize, fSize);
		free(data);
		fSize = compressSize;
		data = compress;
	}
	if (fSize & 0x7FF)
	{
		idx[entries].blocksize = (fSize + (0x800-(fSize%0x800)))/0x800 - 1;
	}
	else
	{
		idx[entries].blocksize = fSize/0x800 -1;
	}
	if (compress) 
		idx[entries].blocksize |= 0x4000;

	idx[entries].offset = (u32)currentOffset;
	_fseeki64(fIMG, currentOffset*0x800, SEEK_SET);
#ifdef DEBUG
	int test = 0;
	fread(&test, 4, 1, fIMG);
	if (test != 0)
	{
		printf("\n==== ERROR ====\n");
		printf("%s\n", filename);
		printf("Offset %08X\n", currentOffset*0x800);
		printf("Size   %08X\n", fSize);
		printf("Block  %04X\n", idx[entries].blocksize & 0x3FFF);
		printf("Compr. %8i\n", idx[entries].blocksize & 0x3FFF ? true : false);
		while(true) Sleep(1);
	}
#endif
	fwrite(data, 1, fSize, fIMG);
	free(data);

	currentOffset += (idx[entries].blocksize & 0x3FFF) + 1;
	entries++;
	return true;
}

u32 IDX::GetEntries()
{
	return entries;
}
void IDX::AddIDX(char *filename)
{
	idxInternalCount++;
	idxInternal = (IDX*)realloc(idxInternal, idxInternalCount);
	//idxInternal[idxInternal-1].Open()
	/*u32 addedEntries;
	FILE *f = fopen(filename, "rb");
	if (!f) return;
	fread(&addedEntries, 4, 1, f);
	realloc(idx, (entries+addedEntries) * sizeof(IDXLBA));
	fread(idx+entries, sizeof(IDXLBA), addedEntries, f);
	entries += addedEntries;
	fclose(f);*/
}
u32 IDX::GetStringIndex(char *fileName)
{
	u32 hash1 = LIBKH::KH2::IDX::CalculateHash32(fileName);
	u32 hash2 = LIBKH::KH2::IDX::CalculateHash16(fileName);
    u32 index = 0;
    while(true)
    {
        if (idx[index].hash1 == hash1 &&
			idx[index].hash2 == hash2) return index;
        if (++index == entries) return -1;
    }
}
u32 IDX::ExtractFileLL(void **buffer, int *size, char *fileName)
{
    if (entries)
    {
		u32 index;
		if ((unsigned short)fileName != 0xDEF0)
		{
			index = GetStringIndex(fileName);
			if (index == -1) return -1;
		}
		else
		{
			index = (unsigned short)((int)fileName>>16);
			if (index > entries) return -1;
		}
		if (idx[index].realsize == 0)
		{
			return -1;
		}
		u32 blockSize = ((idx[index].blocksize & 0x3FFF) + 1) * 0x800;
		if (idx[index].realsize > 0xC00000)
			blockSize += 0x800000;
        *buffer = malloc(blockSize);
        long long offset = idx[index].offset*0x800;
        _fseeki64(fIMG, offset, SEEK_SET);
        fread(*buffer, blockSize, 1, fIMG);

        if (idx[index].blocksize & 0x4000)   // IS COMPRESSED
        {
			void *decbuffer = LIBKH::KH2::IDX::Decompress(*buffer, idx[index].realsize, blockSize);
			if (decbuffer != *buffer) free(*buffer);
			*buffer = decbuffer;
			*size = idx[index].realsize;
        }
        else
        {
            *size = idx[index].realsize;
        }
		if (idx[index].offset == 1425953)
		{
			idx[index].offset = 0;
		}
		return index;
    }
    return -1;
}
bool IDX::ExtractFile(char *fileName)
{
	void *buf;
	int fileSize;
    char filePath[MAX_PATH];
	sprintf(filePath, "export/%s", fileName);
	u32 index = ExtractFileLL(&buf, &fileSize, fileName);
    if (index != -1)
    {
        char drive[MAX_PATH];
        char dir[MAX_PATH];
        char filename[MAX_PATH];
        char ext[MAX_PATH];
        _splitpath(filePath, drive, dir, filename, ext);
        CreateSubDirs(dir);
        FILE *fOut = fopen(filePath, "w+b");
        if (!fOut)
        {
            printf("ERROR: Unable to create %s\n", filePath);
            return false;
        }
        fwrite(buf, 1, fileSize, fOut);
		fclose(fOut);
		free(buf);
        printf("Created %s\n", filePath);
		idx[index].realsize = 0;
		found++;

		if (createlist)
		{
			IDXOUTLIST out;
			memset(&out, 0, sizeof(IDXOUTLIST));
			strcpy(out.name, fileName);
			out.compressed = idx[index].blocksize >> 14;
			fseek(fList, index * sizeof(IDXOUTLIST), SEEK_SET);
			fwrite(&out, sizeof(IDXOUTLIST), 1, fList);
		}
		return true;
    }
    return false;
}
bool IDX::CreateSubDirs(char *path)
{
    char newDirName[MAX_PATH];
    memset(&newDirName, 0, MAX_PATH);
    if (path)
    {
        char *pNewDirName = newDirName;
        do
        {
            *pNewDirName = *path;
            if (*path== '\\' || *path == '/')
            {
                if (*(path-1) != ':')
                {
                    CreateDirectory(newDirName, 0);
                }
            }
            pNewDirName++;
        } while (*++path);
        return true;
    }
    return false;
}
void IDX::ExtractRemains()
{
	void *buf;
	int fileSize;
	char fileName[MAX_PATH];
	char filePath[MAX_PATH];
	CreateSubDirs("export/@noname/");
	for(u32 i=0; i<entries; i++)
	{
		if (idx[i].realsize != 0)	// Quando è zero significa che il file è stato già estratto
		{
			u32 index = ExtractFileLL(&buf, &fileSize, (char*)(0xDEF0 + (i<<16)));
			if (index != -1)
			{
				char *ext;
				u32 header = *(u32*)buf;
				switch(header)
				{
				case 0x70474156: // VAG
					ext = "vag";
					break;
				case 0x56706F49: // IopVoice
					ext = "vsb";
					break;
				case 0x6C426553: // SeBlock
					ext = "seb";
					break;
				case 0x01524142: // BAR
					ext = "bar";
					break;
				case 0x44474D49: // IMGD
					ext = "imd";
					break;
				case 0x5A474D49: // IMGZ
					ext = "imz";
					break;
				case 0x5F584150: // PAX
					ext = "pax";
					break;
				case 0x204D4742: // BGM
					ext = "bgm";
					break;
				case 0x5F584247: // GBX
					ext = "gbx";
					break;
				case 843925844: // TIM 2
					ext = "tm2";
					break;
				default:		 // Unknow
					ext = "bin";
				}

				sprintf(fileName, "@noname/%08X.%s", idx[i].hash1, ext);
				sprintf(filePath, "export/%s", fileName);
				FILE *fOut = fopen(filePath, "wb");
				if (!fOut)
					printf("ERROR: Unable to create %s\n", filePath);
				fwrite(buf, fileSize, 1, fOut);
				fclose(fOut);
				free(buf);
				printf("%s created!\n", filePath);

				if (createlist)
				{
					IDXOUTLIST out;
					memset(&out, 0, sizeof(IDXOUTLIST));
					strcpy(out.name, fileName);
					out.compressed = idx[index].blocksize >> 14;	//(idx[index].blocksize & 0x4000) != 0;
					fseek(fList, index * sizeof(IDXOUTLIST), SEEK_SET);
					fwrite(&out, sizeof(IDXOUTLIST), 1, fList);
				}
			}
		}
	}
}