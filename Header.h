#ifndef HEADER_Header
#define HEADER_Header
/*
htop - Header.h
(C) 2004-2011 Hisham H. Muhammad
Released under the GNU GPLv2, see the COPYING file
in the source distribution for its full text.
*/

#include "Meter.h"
#include "ProcessList.h"
#include "Settings.h"
#include "Vector.h"

typedef struct Header_ {
   Vector** columns;
   Settings* settings;
   ProcessList* pl;
   unsigned int nrColumns;
   unsigned int pad;
   unsigned int height;
} Header;

#define Header_forEachColumn(this_, i_) for (unsigned int (i_)=0; (i_) < (this_)->nrColumns; ++(i_))

Header* Header_new(ProcessList* pl, Settings* settings, unsigned int nrColumns);

void Header_delete(Header* this);

void Header_populateFromSettings(Header* this);

void Header_writeBackToSettings(const Header* this);

MeterModeId Header_addMeterByName(Header* this, const char* name, unsigned int column);

void Header_setMode(Header* this, unsigned int i, MeterModeId mode, unsigned int column);

Meter* Header_addMeterByClass(Header* this, const MeterClass* type, unsigned int param, unsigned int column);

size_t Header_size(const Header* this, unsigned int column);

MeterModeId Header_readMeterMode(const Header* this, unsigned int i, unsigned int column);

void Header_reinit(Header* this);

void Header_draw(const Header* this);

void Header_updateData(Header* this);

unsigned int Header_calculateHeight(Header* this);

#endif
