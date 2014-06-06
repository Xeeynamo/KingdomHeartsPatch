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
#include "Extra.h"

char path[MAX_PATH];
const int dumpHowLanguages = 9;
const int dumpHowWorlds = 22;
const int dumpHowVoices = 17;
const int dumpHowEVoices = 20;
const int dumpHowFiles = 14;
const int dumpHowMenus = 10;
const int dumpHowMagics = 36;
const int dumpHowPSS = 12;
char *dumpLanguages[dumpHowLanguages] = {"jp", "us", "uk", "it", "sp", "gr", "fr", "fm", "fj"};
char *dumpWorlds[dumpHowWorlds] = {"zz", "es", "tt", "di", "hb", "bb", "he", "al", "mu",
	"po", "lk", "lm", "dc", "wi", "nm", "wm", "ca", "tr", "eh", "ex", "jm", "gumi"};
char *dumpVoices[dumpHowVoices] = {"reraise", "sora", "donald", "goofy", "micky", "auron", "mulan",
	"alladin", "sparrow", "beast", "jack", "simba", "tron", "riku", "roxas", "pin", "us"};
char *dumpEVoices[dumpHowEVoices] = {"rk", "rs", "sr", "x1"};
char *dumpFiles[dumpHowFiles] = {"al_ca", "ca_na", "gm_ps", "hb_co", "he_co", "he_co2", "he_ft",
	"lm_al", "mg_lm_mu", "mu_mi", "tr_cy", "tr_tm", "tt_my", "wm_ws"};
char *dumpMenus[dumpHowMenus] = {"camp", "ending", "itemshop", "jiminy", "jm_world", "jmemo", "mixdata", "pause", "save", "title"};
char *dumppss[dumpHowPSS] = {"ena", "hca", "hcb", "lop", "me2", "me3", "me4", "me6", "mea", "opn", "sec", "sec2"};

bool IsString(char *p)
{
	int i = 0;
ISSTRING_REDO:
	if ((*p >= '0' && *p <= '9') || (*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || *p=='_' || *p=='/')
	{
		p++;
		if (i++ >= 2)
			return true;
		goto ISSTRING_REDO;
	}
	return false;
}

void ExtractMDLX(IDX *idx, char *filename)
{
	int fSize;
	void *data;
    char fileName[MAX_PATH];

	FILE *f = fopen(filename, "r+b");
	if (!f) return;
	fseek(f, 0, SEEK_END);
	fSize = ftell(f);
	fseek(f, 0, SEEK_SET);
	data = malloc(fSize);
	fread(data, 1, fSize, f);
	fclose(f);
	char *currentName = (char*)data + 0x10;
	char *endoffile = (char *)data + fSize;
	if ((char*)data+0x10 < (char*)data+fSize)
	{
		do
		{
			sprintf(fileName, "obj/%s.mdlx", currentName);
			idx->ExtractFile(fileName);
			sprintf(fileName, "obj/%s.a.fm", currentName);
			idx->ExtractFile(fileName);
			sprintf(fileName, "obj/%s.mset", currentName);
			idx->ExtractFile(fileName);
			sprintf(fileName, "obj/%s_MEMO.mset", currentName);
			idx->ExtractFile(fileName);
			if (*currentName != -0x20)
			{
				sprintf(fileName, "obj/%s", currentName + 0x20);
				idx->ExtractFile(fileName);
			}
			currentName += 0x60;
		} while (currentName < endoffile);
	}
	free(data);
}

void ExtractBATTLE(IDX *idx, char *filename)
{
	BAR bar;
	if (!bar.Open(filename)) return;

	// DUMP LIMIT
	int *barBattleLimit = (int*)bar.GetPointer(bar.GetIndex("limt"));
	int battleLimitCount = *(barBattleLimit+1);
	char *barBattleLimitChar = (char*)(barBattleLimit+3);
	for(int l=0; l<dumpHowLanguages; l++)
	{
		for(int i=0; i<battleLimitCount; i++)
		{
			sprintf(path, "limit/%s/%s", dumpLanguages[l], barBattleLimitChar + (i*0x40));
			idx->ExtractFile(path);
		}
	}

	// DUMP MAGICS
	int *barBattleMagic = (int*)bar.GetPointer(bar.GetIndex("magc"));
	int battleMagicCount = *(barBattleLimit+1);
	char *barBattleMagicChar = (char*)(barBattleMagic+3);
	for(int i=0; i<battleMagicCount; i++)
	{
		idx->ExtractFile(barBattleMagicChar + (i*0x38));
	}
}
void ExtractSYSTEM(IDX *idx, char *filename)
{
	BAR bar;
	if (!bar.Open(filename)) return;
	
	// DUMP SOME OBJECTS
	int current = bar.GetIndex("wmst");
	int *barSystem = (int*)bar.GetPointer(current);
	int elements = ((int)bar.GetPointer(current+1) - (int)barSystem)/0x20;

	for(int i=0; i<elements; i++)
	{
		if (*(barSystem + i*0x20) != 0)
		{
			sprintf(path, "obj/%s.mdlx", barSystem + i*0x20);
			idx->ExtractFile(path);
			sprintf(path, "obj/%s.a.fm", barSystem + i*0x20);
			idx->ExtractFile(path);
			sprintf(path, "obj/%s.mset", barSystem + i*0x20);
			idx->ExtractFile(path);
			sprintf(path, "obj/%s_MEMO.mset", barSystem + i*0x20);
			idx->ExtractFile(path);
		}
	}
}
void ExtractARC(IDX *idx, char *filename)
{
	BAR bar;
	Event event;
	char spath[MAX_PATH];
	char *anmloc[0x100];
	sprintf(spath, "export/%s", filename);
	if (bar.Open(spath))
	{
		int e = bar.head->howFiles;
		for(int i=0; i<e; i++)
		{
			if (i==6)
				i=i;
			if (event.Load(bar.GetPointer(i), bar.lba[i].size))
			{
				int t;
				char *p;
				int anmindex = 0;
				while ((p = event.GetFile(&t)) != (char*)1)
				{
					if (p)
					{
						switch(t)
						{
						case 0x15:	// MSN/%s(LANG)/%s(ID).bar
							if (IsString(p))
							{
								for(int i=0; i<dumpHowLanguages; i++)
								{
									sprintf(spath, "msn/%s/%s.bar", dumpLanguages[i], p);
									idx->ExtractFile(spath);
								}
							}
							break;
						case 0x25:
						{
							if (IsString(p))
							{
								char *anmx = p;
								anmx[3] = 0;
								anmx[6] = 0;
								for(int a = 0; a<anmindex; a++)
								{
									sprintf(spath, "%s/%s/%s/%s.anb", anmx+0, anmx+4, anmloc[a],anmx+7);
									idx->ExtractFile(spath);
								}
							}
							break;
						}
						case 0x23:
						case 0x26:
							if (IsString(p))
							{
								for(int l=0; l<dumpHowLanguages; l++)
								{
									sprintf(spath, "voice/%s/event/%s.vag", dumpLanguages[l], p);
									idx->ExtractFile(spath);
								}
							}
							break;
						case 0x38:
							if (IsString(p))
							{
								anmloc[anmindex++] = p;
								if (anmindex > 0x100)
								{
									anmindex = anmindex;
								}
							}
							break;
						case 0x3E:
							for(int l=0; l<dumpHowLanguages; l++)
							{
								sprintf(spath, "event/layout/%s/%s.2ld", dumpLanguages[l], p);
								idx->ExtractFile(spath);
							}
							break;
						default:
							break;
						}
					}
				}
			}
		}
		bar.Close();
	}
	else
	{
		printf("Unable to open %s", spath);
	}
}

void ExtractEverything(IDX *idx)
{
	idx->ExtractFile("ovl_title.x");
	idx->ExtractFile("ovl_gumibattle.x");
	idx->ExtractFile("ovl_gumimenu.x");
	idx->ExtractFile("ovl_shop.x");
	idx->ExtractFile("ovl_movie.x");
	for(int l=0; l<dumpHowLanguages; l++)
		for(int i=0; i<dumpHowPSS; i++)
		{
			sprintf(path, "zmovie/%s/%s.pss", dumpLanguages[l], dumppss[i]);
			idx->ExtractFile(path);
		}
	for(int w=0; w<dumpHowWorlds; w++)
	{
		for(int i=0; i<100; i++)
		{
			sprintf(path, "ard/%s%02d.ard", dumpWorlds[w], i);
			if (idx->ExtractFile(path))
			{
				if (w == 9)
					if (i == 5)
						i = 5;
				ExtractARC(idx, path);
			}
		}
	}
	for(int i=0; i<dumpHowWorlds; i++)
	{
		sprintf(path, "000%s.idx", dumpWorlds[i]);
		idx->ExtractFile(path);
	}
	for(int l=0; l<dumpHowLanguages; l++)
	{
		for(int w=0; w<dumpHowWorlds; w++)
		{
			for(int i=0; i<10; i++)
			{
				sprintf(path, "field2d/%s/%s%1dcommand.2dd", dumpLanguages[l], dumpWorlds[w], i);
				idx->ExtractFile(path);
			}
			sprintf(path, "field2d/%s/%s0field.2dd", dumpLanguages[l], dumpWorlds[w]);
			idx->ExtractFile(path);
		}
		sprintf(path, "field2d/%s/zz0command.2dd", dumpLanguages[l]);
		idx->ExtractFile(path);
	}
	for(int l=0; l<dumpHowLanguages; l++)
	{
		for(int w=0; w<dumpHowWorlds; w++)
		{
			for(int i=0; i<100; i++)
			{
				sprintf(path, "map/%s/%s%02d.map", dumpLanguages[l], dumpWorlds[w], i);
				idx->ExtractFile(path);
			}
		}
	}
	for(int i=0; i<100; i++)
	{
		sprintf(path, "gumibattle/gm%02d.map", i);
		idx->ExtractFile(path);
	}
	for(int l=0; l<dumpHowLanguages; l++)
	{
		for(int w=0; w<dumpHowWorlds; w++)
		{
			sprintf(path, "msg/%s/%s.bar", dumpLanguages[l], dumpWorlds[w]);
			idx->ExtractFile(path);
		}
		sprintf(path, "msg/%s/jm.bar", dumpLanguages[l]);
		idx->ExtractFile(path);
		sprintf(path, "msg/%s/gumi.bar", dumpLanguages[l]);
		idx->ExtractFile(path);
		sprintf(path, "msg/%s/title.bar", dumpLanguages[l]);
		idx->ExtractFile(path);
		sprintf(path, "msg/%s/sys.bar", dumpLanguages[l]);
		idx->ExtractFile(path);
		sprintf(path, "msg/%s/place.bin", dumpLanguages[l]);
		idx->ExtractFile(path);
		sprintf(path, "msg/%s/fontinfo.bar", dumpLanguages[l]);
		idx->ExtractFile(path);
		sprintf(path, "msg/%s/fontimage.bar", dumpLanguages[l]);
		idx->ExtractFile(path);
	}
	idx->ExtractFile("00font.bar");
	idx->ExtractFile("00objentry.bin");
	idx->ExtractFile("00progress.bin");
	idx->ExtractFile("00battle.bin");
	idx->ExtractFile("00common.bdx");
	idx->ExtractFile("00system.bin");
	idx->ExtractFile("00worldpoint.bin");
	idx->ExtractFile("00effect.bar");
	idx->ExtractFile("03system.bin");
	idx->ExtractFile("07localset.bin");
	idx->ExtractFile("10font.bar");
	idx->ExtractFile("12soundinfo.bar");
	idx->ExtractFile("14mission.bar");
	idx->ExtractFile("15jigsaw.bin");
	idx->ExtractFile("50gumientry.bin");
	idx->ExtractFile("70landing.bar");
	idx->ExtractFile("eventviewer.bar");
	idx->ExtractFile("effect/texcommon.dpd");
	idx->ExtractFile("obj/GAMEOVER.mset");
	idx->ExtractFile("obj/WM_CURSOR.mdlx");
	idx->ExtractFile("obj/WM_SYMBOL_TT.mdlx");
	idx->ExtractFile("obj/WM_SYMBOL_HB.mdlx");
	idx->ExtractFile("obj/WM_SYMBOL_MU.mdlx");
	idx->ExtractFile("obj/WM_SYMBOL_BB.mdlx");
	idx->ExtractFile("obj/WM_SYMBOL_HE.mdlx");
	idx->ExtractFile("obj/WM_SYMBOL_DC.mdlx");
	idx->ExtractFile("obj/WM_SYMBOL_LM.mdlx");
	idx->ExtractFile("obj/WM_SYMBOL_CA.mdlx");
	idx->ExtractFile("obj/WM_SYMBOL_NM.mdlx");
	idx->ExtractFile("obj/WM_SYMBOL_AL.mdlx");
	idx->ExtractFile("obj/WM_SYMBOL_LK.mdlx");
	idx->ExtractFile("obj/WM_SYMBOL_EH.mdlx");
	idx->ExtractFile("obj/WM_GUMI_01.mdlx");
	idx->ExtractFile("obj/WM_CURSOR.a.fm");
	idx->ExtractFile("obj/WM_SYMBOL_TT.a.fm");
	idx->ExtractFile("obj/WM_SYMBOL_HB.a.fm");
	idx->ExtractFile("obj/WM_SYMBOL_MU.a.fm");
	idx->ExtractFile("obj/WM_SYMBOL_BB.a.fm");
	idx->ExtractFile("obj/WM_SYMBOL_HE.a.fm");
	idx->ExtractFile("obj/WM_SYMBOL_DC.a.fm");
	idx->ExtractFile("obj/WM_SYMBOL_LM.a.fm");
	idx->ExtractFile("obj/WM_SYMBOL_CA.a.fm");
	idx->ExtractFile("obj/WM_SYMBOL_NM.a.fm");
	idx->ExtractFile("obj/WM_SYMBOL_AL.a.fm");
	idx->ExtractFile("obj/WM_SYMBOL_LK.a.fm");
	idx->ExtractFile("obj/WM_SYMBOL_EH.a.fm");
	idx->ExtractFile("obj/WM_GUMI_01.a.fm");
	idx->ExtractFile("obj/WM_CURSOR.mset");
	idx->ExtractFile("obj/WM_SYMBOL_TT.mset");
	idx->ExtractFile("obj/WM_SYMBOL_HB.mset");
	idx->ExtractFile("obj/WM_SYMBOL_MU.mset");
	idx->ExtractFile("obj/WM_SYMBOL_BB.mset");
	idx->ExtractFile("obj/WM_SYMBOL_HE.mset");
	idx->ExtractFile("obj/WM_SYMBOL_DC.mset");
	idx->ExtractFile("obj/WM_SYMBOL_LM.mset");
	idx->ExtractFile("obj/WM_SYMBOL_CA.mset");
	idx->ExtractFile("obj/WM_SYMBOL_NM.mset");
	idx->ExtractFile("obj/WM_SYMBOL_AL.mset");
	idx->ExtractFile("obj/WM_SYMBOL_LK.mset");
	idx->ExtractFile("obj/WM_SYMBOL_EH.mset");
	idx->ExtractFile("npack/worldobj.bar");
		
	for(int l=0; l<dumpHowLanguages; l++)
	{
		sprintf(path, "msn/%s/WORLDMAP.bar", dumpLanguages[l]);
		idx->ExtractFile(path);
	}

	for(int l=0; l<dumpHowLanguages; l++)
	{
		for(int w=0; w<dumpHowWorlds; w++)
		{
			for(int i=0; i<10; i++)
			{
				for(int v=0; v<dumpHowVoices; v++)
				{
					sprintf(path, "voice/%s/battle/%s%1d_%s.vsb", dumpLanguages[l], dumpWorlds[w], i, dumpVoices[v]);
					idx->ExtractFile(path);
				}
			}
		}
		sprintf(path, "voice/%s/gumibattle/gumi.vsb", dumpLanguages[l]);
		idx->ExtractFile(path);
		sprintf(path, "voice/%s/event/tt_titleL.vsb", dumpLanguages[l]);
		idx->ExtractFile(path);
		sprintf(path, "voice/%s/event/tt_titleR.vsb", dumpLanguages[l]);
		idx->ExtractFile(path);
		sprintf(path, "voice/%s/event/tt_titleL.vag", dumpLanguages[l]);
		idx->ExtractFile(path);
		sprintf(path, "voice/%s/event/tt_titleR.vag", dumpLanguages[l]);
		idx->ExtractFile(path);
	}
	for(int i=0; i<1000; i++)
	{
		sprintf(path, "itempic/item-%03d.imd", i);
		idx->ExtractFile(path);
	}
	for(int w=0; w<dumpHowWorlds; w++)
	{
		sprintf(path, "libretto-%s.bar", dumpWorlds[w]);
		idx->ExtractFile(path);
	}
	for(int w=0; w<dumpHowWorlds; w++)
	{
		for(int i=0; i<1000; i++)
		{
			sprintf(path, "event/effect/%s_event_%03d.pax", dumpWorlds[w], i);
			idx->ExtractFile(path);
			sprintf(path, "event/effect/%s_event_%03da.pax", dumpWorlds[w], i);
			idx->ExtractFile(path);
			sprintf(path, "event/effect/%s_event_%03db.pax", dumpWorlds[w], i);
			idx->ExtractFile(path);
			sprintf(path, "event/effect/%s_event_%03dc.pax", dumpWorlds[w], i);
			idx->ExtractFile(path);
			sprintf(path, "event/effect/%s_event_%03dd.pax", dumpWorlds[w], i);
			idx->ExtractFile(path);
			sprintf(path, "event/effect/%s_event_%03de.pax", dumpWorlds[w], i);
			idx->ExtractFile(path);
			sprintf(path, "event/effect/%s_event_%03df.pax", dumpWorlds[w], i);
			idx->ExtractFile(path);
			sprintf(path, "event/effect/%s_event_%03dg.pax", dumpWorlds[w], i);
			idx->ExtractFile(path);
		}
	}
	for(int l=0; l<dumpHowLanguages; l++)
	{
		for(int i=0; i<dumpHowFiles; i++)
		{
			sprintf(path, "file/%s/%s.2ld", dumpLanguages[l], dumpFiles[i]);
			idx->ExtractFile(path);
		}
	}
	for(int l=0; l<dumpHowLanguages; l++)
	{
		for(int m=0; m<dumpHowMenus; m++)
		{
			sprintf(path, "menu/%s/%s.2ld", dumpLanguages[l], dumpMenus[m]);
			idx->ExtractFile(path);
			sprintf(path, "menu/%s/%s.bar", dumpLanguages[l], dumpMenus[m]);
			idx->ExtractFile(path);
		}
		for(int w=0; w<dumpHowWorlds; w++)
		{
			sprintf(path, "menu/%s/jm_top_%s.2ld", dumpLanguages[l], dumpWorlds[w]);
			idx->ExtractFile(path);
			sprintf(path, "menu/%s/jmface_%s.2ld", dumpLanguages[l], dumpWorlds[w]);
			idx->ExtractFile(path);
		}
	}
	for(int l=0; l<dumpHowLanguages; l++)
	{
		for(int w=0; w<dumpHowWorlds; w++)
		{
			for(int i=0; i<100; i++)
			{
				sprintf(path, "menu/%s/jm_photo/%s%02d.bin", dumpLanguages[l], dumpWorlds[w], i);
				idx->ExtractFile(path);
			}
		}
	}
	for(int l=0; l<dumpHowLanguages; l++)
	{
		for(int i=0; i<1000; i++)
		{
			sprintf(path, "menu/%s/jm_puzzle/puzzle%03d.bin", dumpLanguages[l], i);
			idx->ExtractFile(path);
		}
	}
	for(int l=0; l<dumpHowLanguages; l++)
	{
		for(int w=0; w<dumpHowWorlds; w++)
		{
			for(int i=0; i<100; i++)
			{
				sprintf(path, "menu/%s/jm_photo/%s%02d.bin", dumpLanguages[l], dumpWorlds[w], i);
				idx->ExtractFile(path);
			}
		}
	}
	for(int l=0; l<dumpHowLanguages; l++)
	{
		for(int w=0; w<dumpHowWorlds; w++)
		{
			for(int i=0; i<1000; i++)
			{
				sprintf(path, "menu/%s/shopface/p_%s%03d.bin", dumpLanguages[l], dumpWorlds[w], i);
				idx->ExtractFile(path);
			}
		}
	}
	for(int w=0; w<dumpHowWorlds; w++)
	{
		for(int i=0; i<100; i++)
		{
			sprintf(path, "minigame/%s%02d.bar", dumpWorlds[w], i);
			idx->ExtractFile(path);
		}
	}
	idx->ExtractFile("vagstream/Title.vas");
	idx->ExtractFile("vagstream/End_Piano.vas");
	idx->ExtractFile("vagstream/GM1_Asteroid.vas");
	idx->ExtractFile("vagstream/GM2_Highway.vas");
	idx->ExtractFile("vagstream/GM3_Cloud.vas");
	idx->ExtractFile("vagstream/GM4_Floating.vas");
	idx->ExtractFile("vagstream/GM5_Senkan.vas");

	// DUMP VOICES
	/*for(int l=0; l<dumpHowLanguages; l++)
	{
		for(int w=0; w<dumpHowWorlds; w++)
		{
			for(int i=0; i<100000; i++)
			{
				for(int v=0; v<dumpHowVoiceEvt; v++)
				{
					sprintf(path, "voice/%s/event/%s%04d%s", dumpLanguages[l], dumpWorlds[w], i, dumpVoiceEvt[v];
				}
			}
		}
	}*/

	for(int i=0; i<10000; i++)
	{
		sprintf(path, "bgm/music%03d.bgm", i);
		idx->ExtractFile(path);
		sprintf(path, "bgm/music%03d.vsb", i);
		idx->ExtractFile(path);
		sprintf(path, "bgm/wave%04d.wd", i);
		idx->ExtractFile(path);
		sprintf(path, "se/se%03d.seb", i);
		idx->ExtractFile(path);
		sprintf(path, "se/wave%04d.wd", i);
		idx->ExtractFile(path);

		sprintf(path, "gumibattle/se/se%03d.seb", i);
		idx->ExtractFile(path);
		sprintf(path, "gumibattle/se/wave%04d.wd", i);
		idx->ExtractFile(path);
	}
	// libretto/%s/wm%d.bar
	for(int l=0; l<dumpHowLanguages; l++)
	{
		for(int i=0; i<1000; i++)
		{
			sprintf(path, "gumibattle/advice/%s/advice_%03d.imz", dumpLanguages[l], i);
			idx->ExtractFile(path);
		}
		sprintf(path, "gumibattle/advice/%s/advice.lad", dumpLanguages[l]);
		idx->ExtractFile(path);
		sprintf(path, "gumimenu/%s/gumiedit.bar", dumpLanguages[l]);
		idx->ExtractFile(path);
		sprintf(path, "gumimenu/%s/gumimenu.bar", dumpLanguages[l]);
		idx->ExtractFile(path);
	}

	idx->ExtractFile("gumibattle/advice.bin");
	idx->ExtractFile("gumibattle/playerparam.bin");
	idx->ExtractFile("gumibattle/enemyparam.bin");
	idx->ExtractFile("gumibattle/rank.bin");
	idx->ExtractFile("gumibattle/treasure.bin");
	idx->ExtractFile("gumibattle/item.bin");
	idx->ExtractFile("gumibattle/vibration.bar");
	idx->ExtractFile("gumibattle/systemparam.bin");
	idx->ExtractFile("gumibattle/treasure.bin");
	idx->ExtractFile("gumibattle/decogumiparam.bin");

	idx->ExtractFile("gumimenu/attachdata.bar");
	idx->ExtractFile("gumimenu/editmenu.bar");
	idx->ExtractFile("gumimenu/event.bar");
	idx->ExtractFile("gumimenu/gumiblockdata.bin");
	idx->ExtractFile("gumimenu/gumimenu.bin");
	idx->ExtractFile("gumimenu/lump.bar");
	idx->ExtractFile("gumimenu/menuentry.bar");
	idx->ExtractFile("gumimenu/gumiobj.bar");
	idx->ExtractFile("gumimenu/planset.bin");
	idx->ExtractFile("gumimenu/tinyset.bin");

	for(int i=0; i<1000; i++)
	{
		sprintf(path, "gumimenu/manual_%03d.imd", i);
		idx->ExtractFile(path);
	}

	idx->ExtractFile("gumiblock/tex.bar");
	idx->ExtractFile("gumiblock/clt.bar");
	idx->ExtractFile("gumiblock/pxl.bar");
	idx->ExtractFile("gumiblock/knt.bar");

	idx->ExtractFile("gumiedit/tex/tex_gr.tex");

	for(int i=0; i<100; i++)
	{
		for(int c='a'; c<='z'; c++)
		{
			sprintf(path, "gumibattle/gm%02d%c.gbx", i, c);
			idx->ExtractFile(path);
		}
		sprintf(path, "gumibattle/maphide_gm%02d.bin", i);
		idx->ExtractFile(path);
	}
}