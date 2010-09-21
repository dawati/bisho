/*
 * Copyright (C) 2009 Intel Corporation.
 *
 * Author: Ross Burton <ross@linux.intel.com>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SERVICE_INFO_H
#define _SERVICE_INFO_H

#include <glib.h>

typedef struct {
  char *name;
  char *display_name;
  char *description;
  char *link;
  char *auth_type;
  char *icon;
  GKeyFile *keys;
} ServiceInfo;

ServiceInfo * get_info_for_service (const char *name);

#endif /* _SERVICE_INFO_H */
