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

#include "obus.h"

#include <stdio.h>

struct json_object* obus_parseMessage(char* str, int len){
	struct json_tokener* tok = NULL;

	tok = json_tokener_new();
	if(!tok){
		return NULL;
	}
	
	struct json_object* jobj = NULL;
	enum json_tokener_error jerr;

	jobj = json_tokener_parse_ex(tok, str, len);
	jerr = json_tokener_get_error(tok);

	if(jerr != json_tokener_success){
		fprintf(stderr, "JSON parse error: %s\n", json_tokener_error_desc(jerr));
		if(jobj){
			json_object_put(jobj);
		}
		json_tokener_free(tok);
		return NULL;
	}

	json_tokener_free(tok);

	return jobj;
}
