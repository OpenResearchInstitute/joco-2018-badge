/*****************************************************************************
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Contributors:
 * 	@sconklin
 * 	@mustbeart
 * 	@abraxas3d
 *****************************************************************************/
#include "../system.h"

// Seen list, each entry is an array of bytes
#define SEEN_DB_LEN 60

typedef struct {
    uint8_t address[BLE_GAP_ADDR_LEN]; // 6
    uint16_t device_id;                // 2
    uint8_t flags;                     // 1
} ble_badge_seen_entry_t;

ble_badge_seen_entry_t seen_db[SEEN_DB_LEN];

// Active list

// half of original badge db used 416 bytes
// currently, entries use 28 bytes each.
#define BADGE_ACTIVE_LIST_SIZE 14

// We keep a separate index to make sorting faster
ble_badge_active_entry_t active[BADGE_ACTIVE_LIST_SIZE];
uint8_t active_index[BADGE_ACTIVE_LIST_SIZE];


static bool ble_lists_initialized;

void ble_lists_init() {
    for(int i=0; i<BADGE_ACTIVE_LIST_SIZE; i++) {
	active_index[i] = i;
	active[i].first_seen = 0;
    }
    ble_lists_initialized = true;
    // -spc- TODO read as many records as we can
    // from disk DB into the seen list to preload it.
}

//
// Seen list
//

int previous_seen_write = 0; // last write location

int __find_seen_index(uint8_t *address, uint16_t device_id) {
    // return the index into the seen_db, or -1 for not found
    int read_position = previous_seen_write;
    int return_value = -1;

    // run over all entries (backwards)
    for (int i=0; i < SEEN_DB_LEN; i++) {

	
	if (!(seen_db[read_position].flags & SEEN_FLAG_USED)) {
	    // this entry (and all older ones) are unused
	    break;
	}
	if (seen_db[read_position].device_id == device_id) {
	    if (!memcmp(&seen_db[read_position].address, address, BLE_GAP_ADDR_LEN)) {
		// we found a match
		return_value = read_position;
		break;
	    }
	} else {
	    // reposition pointer for next read
	    read_position--;
	    if (read_position < 0)
		read_position = SEEN_DB_LEN-1;
	}
    }
    return return_value;
}

void add_to_seen(uint8_t *address, uint16_t device_id, uint8_t type, uint8_t flags) {
    type &= SEEN_TYPE_MASK;
    flags |= (SEEN_FLAG_MASK & flags);
    flags |= SEEN_FLAG_USED;
    flags |= type;
    // We always add in a circular buffer fashion
    // overwriting the oldest entry
    previous_seen_write++;
    previous_seen_write %= SEEN_DB_LEN;
    memcpy(seen_db[previous_seen_write].address, address, BLE_GAP_ADDR_LEN);
    seen_db[previous_seen_write].device_id = device_id;
    seen_db[previous_seen_write].flags = flags;
}

//
// Check to see if the badge is in the seen list
// if it's not, then see if it's in the db on disk
// If it's in the db, then add it to the seen list
//

uint8_t check_and_add_to_seen(uint8_t *address, uint16_t device_id, uint8_t type) {
    // return zero or the seen_flags field from the entry
    uint8_t return_value = 0;
    int read_position = __find_seen_index(address, device_id);
    if (read_position >= 0) {
	// found it
	return_value =  seen_db[read_position].flags;
    } else {
	// not in the seen list
	if (type & SEEN_TYPE_JOCO) {
	    // if it's a joco badge, check the db on disk
	    if (was_contacted(address, device_id)) {
		add_to_seen(address, device_id, SEEN_TYPE_JOCO, 0);
		return_value = SEEN_FLAG_USED;
	    }
	} else if  (type & SEEN_TYPE_PEER) {
	    add_to_seen(address, device_id, SEEN_TYPE_PEER, 0);
	    return_value = SEEN_FLAG_USED;
	}
    }
    return return_value;
}

int set_seen_flags(uint8_t *address, uint16_t device_id, uint8_t flags) {
    int read_position = __find_seen_index(address, device_id);
    int return_value;
    uint8_t newflags;

    if (read_position < 0) {
	return_value = read_position;
    } else {
	newflags = (flags & SEEN_FLAG_MASK);
	newflags |= SEEN_FLAG_USED;
	seen_db[read_position].flags = newflags;
	return_value = 0;
    }
    return return_value;
}

//
// Active list
//

// Active List functions

int try_to_add_to_active(uint8_t *address, uint16_t device_id, int8_t rssi, char *name) {
    int i;
    ble_badge_active_entry_t *p_entry;

    for (i=0; i<BADGE_ACTIVE_LIST_SIZE; i++) {
	p_entry = &active[active_index[i]];
	if (!p_entry->first_seen) {
	    // we found an unused one
	    uint32_t now = util_local_millis();
	    memcpy(p_entry->address, address, BLE_GAP_ADDR_LEN);
	    strncpy(p_entry->name, name, SETTING_NAME_LENGTH);
	    p_entry->device_id = device_id;
	    p_entry->rssi = rssi;
	    p_entry->first_seen = now;
	    p_entry->last_seen = now;
	    p_entry->said_hello = false;
	    return i;
	}
	
    }
    return -1;
}

ble_badge_active_entry_t *in_active_list(uint8_t *address, uint16_t device_id) {
    int i;
    ble_badge_active_entry_t *p_entry;
    ble_badge_active_entry_t *retval = 0;

    if (!ble_lists_initialized)
	mbp_ui_error("Active list not initialized");

    // return pointer to the badge data, NULL if not found
    // the index is always sorted in order of , with unused at the end
    for (i=0; i<BADGE_ACTIVE_LIST_SIZE; i++) {
	p_entry = &active[active_index[i]];
	if (!p_entry->first_seen)
	    break; // we've reached the first unused p_entry without finding it
	if ((p_entry->device_id == device_id) && !(memcmp(p_entry->address, address, BLE_GAP_ADDR_LEN))) {
	    retval = p_entry;
	    break;
	}
    }
    return retval;
}

int __compare_active_entries(int a, int b, bool ignore_timers) {
    // a - b

    // unused entries always count as 'less'
    if ((active[active_index[a]].first_seen == 0) && (active[active_index[b]].first_seen == 0))
	return 0;
    else if (active[active_index[a]].first_seen == 0)
	return -1;
    else if (active[active_index[a]].first_seen == 0)
	return 1;

    // if we just reset the timers they're all the same
    if (ignore_timers)
	return 0;

    // if both used, sort based on time in contact.
    return ((active[active_index[a]].last_seen - active[active_index[a]].first_seen)
	    - (active[active_index[b]].last_seen - active[active_index[b]].first_seen));
}

void sort_active(bool reset_timers) {
    // sorts the index pointers in order of decreasing contact time
    // if reset_timers set, then sets all first_seen timestamps to now.
    // unused entries are at the end of the list.
    //Lazy bubble sort
    uint32_t now = util_local_millis();

    // First walk them all for maintenance
    for (uint8_t i = 0; i < BADGE_ACTIVE_LIST_SIZE; i++) {
	// if we haven't seen it in a while, flag it unused
	if ((now - active[active_index[i]].last_seen) > VISIT_LOST_TIME_LENGTH) {
	    active[active_index[i]].first_seen = 0;
	}

	if (reset_timers) {
	    // is it's in use then reset first_seen to start visit timing over
	    if (active[active_index[i]].first_seen != 0) {
		active[active_index[i]].first_seen = now;
		active[active_index[i]].last_seen = now; // no time travel allowed
	    }
	}
    }

    // Then sort them in the index
    for (uint8_t i = 0; i < BADGE_ACTIVE_LIST_SIZE; i++) {
        for (uint8_t j = i; j < BADGE_ACTIVE_LIST_SIZE - 1; j++) {
            if (__compare_active_entries(active_index[j], active_index[j + 1], reset_timers) < 0) {
                //Swap
		uint8_t tmp = active_index[j];
		active_index[j] = active_index[j+1];
		active_index[j+1] = tmp;
            }
        }
    }
}

// Return a list into a pre-allocated array of entries

int get_nearby_badge_list(int size, ble_badge_list_menu_text_t *list) {
    int max_count = BADGE_ACTIVE_LIST_SIZE;
    ble_badge_active_entry_t *p_entry;
    int return_count = 0;

    if (size < BADGE_ACTIVE_LIST_SIZE)
	max_count = size;

    // no mutex, perhaps there should be but not a big deal

    // TODO no sorting, perhaps sort by rssi
    for (uint8_t i=0; i<max_count; i++) {
	p_entry = &active[active_index[i]];
	if (!p_entry->first_seen)
	    break; // we've reached the first unused entry without finding it
	sprintf(list[i].text, "%s %ddB", p_entry->name, p_entry->rssi);
	return_count++;
    }

    return return_count;
}
