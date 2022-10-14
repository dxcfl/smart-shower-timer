/*
SPDX-License-Identifier: GPL-3.0-or-later

iaq-monitor-demo
Copyright (C) 2021  dxcfl

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _BEACON_H
#define _BEACON_H

#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Beacon start ...
 */
void beacon_start();

/*
 * Beacon update ...
 */
void beacon_update_with_time(uint32_t time);

/*
 * Beacon update ...
 */
void beacon_update(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* _BEACON_H */