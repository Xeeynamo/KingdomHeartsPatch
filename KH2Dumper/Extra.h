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
#include "BAR.h"
#include "IDX.h"

extern char path[];


void ExtractMDLX(IDX *idx, char *filename);
void ExtractBATTLE(IDX *idx, char *filename);
void ExtractSYSTEM(IDX *idx, char *filename);
void ExtractARC(IDX *idx, char *filename);

void ExtractEverything(IDX *idx);

extern const int dumpHowWorlds;
extern char *dumpWorlds[];


/*
es
tt
di
hb
bb
he
al
mu
po
lk
lm
dc
wi
nm
wm
ca
tr
eh
voice/fm/event/es99506rk.vag
anm/dc/b_ex420/EDC20103A.anb
anm/dc/p_ex100/EEX00001C.anb
anm/es/h_ex710/EES99507E.anb
event/layout/jp/tt_telop.2ld
*/