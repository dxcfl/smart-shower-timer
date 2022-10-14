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

#include <device.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include <drivers/sensor/ccs811.h>
#include <drivers/display.h>
#include <zephyr.h>
#include <stdio.h>
#include <string.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(app);

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

/* Auxiliary function: Format time string.
 */
static const char *time_str(uint32_t time, bool with_millis)
{
	static char buf[16]; /* ...HH:MM:SS.MMM */
	unsigned int ms = time % MSEC_PER_SEC;
	unsigned int s;
	unsigned int min;
	unsigned int h;

	time /= MSEC_PER_SEC;
	s = time % 60U;
	time /= 60U;
	min = time % 60U;
	time /= 60U;
	h = time;
	if (with_millis)
		snprintf(buf, sizeof(buf), "%u:%02u:%02u.%03u",
				 h, min, s, ms);
	else
		snprintf(buf, sizeof(buf), "%u:%02u:%02u",
				 h, min, s);

	return buf;
}

static const char *now_str()
{
	return time_str(k_uptime_get_32(), true);
}

/* Bluetooth beacon setup ...
 * "stolen" from the Zephyr bluetooth beacon example
 * (zephyr/samples/bluetooth/beacon/main.c).
 * Setup a non-connectable Eddystone beacon.
 * Later we will "abuse" the name data in the scan
 * resonse to transport our IAQ rating.
 */

/*
 * Set Advertisement data. Based on the Eddystone specification:
 * https://github.com/google/eddystone/blob/master/protocol-specification.md
 * https://github.com/google/eddystone/tree/master/eddystone-url
 */
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0xaa, 0xfe),
	BT_DATA_BYTES(BT_DATA_SVC_DATA16,
				  0xaa, 0xfe, /* Eddystone UUID */
				  0x10,		  /* Eddystone-URL frame type */
				  0x00,		  /* Calibrated Tx power at 0m */
				  0x00,		  /* URL Scheme Prefix http://www. */
				  'e', 'x', 'a', 'm', 'p', 'l', 'e',
				  0x08) /* .org */
};

/* Set Scan Response data */
static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
	BT_DATA(BT_DATA_NAME_SHORTENED, DEVICE_NAME, DEVICE_NAME_LEN),
};

static void bt_ready(int err)
{

	char addr_s[BT_ADDR_LE_STR_LEN];
	bt_addr_le_t addr = {0};
	size_t count = 1;

	if (err)
	{
		printk("\n[%s]: Bluetooth init failed (err %d)\n", now_str(), err);
		return;
	}
	printk("\n[%s]: Bluetooth initialized\n", now_str());

	/* Start advertising */
	err = bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY, ad, ARRAY_SIZE(ad),
						  sd, ARRAY_SIZE(sd));
	if (err)
	{
		printk("\n[%s]: Advertising failed to start (err %d)\n", now_str(), err);
		return;
	}

	/* For connectable advertising you would use
	 * bt_le_oob_get_local().  For non-connectable non-identity
	 * advertising an non-resolvable private address is used;
	 * there is no API to retrieve that.
	 */

	bt_id_get(&addr, &count);
	bt_addr_le_to_str(&addr, addr_s, sizeof(addr_s));

	printk("\n[%s]: Beacon started, advertising as %s\n", now_str(), addr_s);
}

/*
 * Beacon start ...
 */
void beacon_start()
{
	/* Setup and start Bluetooth beacon
	 */
	int bt_err;
	printk("\n[%s]: BT: Starting beacon ...\n", now_str());
	bt_err = bt_enable(bt_ready);
	if (bt_err)
	{
		printk("\n[%s]: BT: Initiialization failed (err %d)\n", now_str(), bt_err);
	}
}

/*
 * Beacon update ...
 */
void beacon_update_with_time(uint32_t time)
{
	/* Update the scan reponse data for the Bluetooth beacon: 'Misuse' the name data for
	 *  transporting the time.
	 */
	beacon_update(time_str(time, true));
}

/*
 * Beacon update ...
 */
void beacon_update(const char *name)
{
	int bt_err;

	struct bt_data new_sd[] = {
		BT_DATA(BT_DATA_NAME_COMPLETE, name, strlen(name)),
		BT_DATA(BT_DATA_NAME_SHORTENED, DEVICE_NAME, DEVICE_NAME_LEN),
	};
	bt_err = bt_le_adv_update_data(ad, ARRAY_SIZE(ad),
								   new_sd, ARRAY_SIZE(new_sd));
	if (bt_err)
	{
		printk("\n[%s]: BT: Advertising update failed (err %d)\n", now_str(), bt_err);
	}
}