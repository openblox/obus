/*
 * Copyright (C) 2017 John M. Harris, Jr. <johnmh@openblox.org>
 *
 * This file is part of OBus.
 *
 * OBus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OBus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OBus.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef OBUS_CONF_H_
#define OBUS_CONF_H_

//int
#define OBUS_CONF_ENT_TYPE_INT 1
//char*
#define OBUS_CONF_ENT_TYPE_STR 2
//obus_ConfigEntry**
#define OBUS_CONF_ENT_TYPE_ARRAY 3

typedef struct obus_ConfigEntry{
	unsigned char type;
	int refs;
	union{
		int integer;
		char cchar;
		unsigned char uchar;
		struct{
			int len;
			char* str;
		} str;
		struct{
				int len;
				struct obus_ConfigEntry** array;
		} array;
	} data;
} obus_ConfigEntry;

unsigned char obus_loadConfig(char* name);
unsigned char obus_configLoaded();

unsigned char obus_hasConfigEntry(char* name);
obus_ConfigEntry* obus_getConfigEntry(char* name);
void obus_releaseConfigEntry(obus_ConfigEntry* ent);

#endif
