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
#ifndef UTIL_BLE_LISTS_H_
#define UTIL_BLE_LISTS_H_

// There are two lists. The seen list maintains minimal information,
// and the active list maintains more detailed information

// The Seen list is fixed size LIFO. It holds all non-joco badges that we see,
// plus a cache of joco badges we've recently seen which we've already counted
// as visited for scoring.

// The Active list hold only joco badges.
// A Joco badge enters the active list if it hasn't been
// previously counted as visited and also:
//   1. There is room in the active list for it
//     and
//   2. The rssi is above the VISIT_RSSI_MIN threshold
//
// If we hear a new joco badge that we haven't scored as a visit and there's not room
// on the active list, then we ignore it and wait for it to advertise again.
//
// The Active list is reverse sorted by (last_seen - first-seen) so the top entry
// is the next candidate for being counted as 'visited'
// Any badge with a last_seen time longer than VISIT_LOST_TIME_LENGTH is dropped from the active list
// 

void ble_lists_init();

//
// Seen list
//
#define SEEN_FLAG_MASK       0xF0
#define SEEN_FLAG_VISITED    0x10
#define SEEN_FLAG_SAID_HELLO 0x20
//#define SEEN_FLAG_FUTURE_USE 0x40
#define SEEN_FLAG_USED       0x80

#define SEEN_TYPE_MASK 0x0F
#define SEEN_TYPE_JOCO 0x01
#define SEEN_TYPE_PEER 0x02

void add_to_seen(uint8_t *address, uint16_t device_id, char *name, uint8_t type, uint8_t flags);
uint8_t check_and_add_to_seen(uint8_t *address, uint16_t device_id, char *name, uint8_t type);
int set_seen_flags(uint8_t *address, uint16_t device_id, uint8_t flags);

//
// Active list
//
// Used for storage of badge info in the 'active' list
typedef struct {
    char name[SETTING_NAME_LENGTH];    // 9
    int8_t rssi;                       // 1
    uint8_t address[BLE_GAP_ADDR_LEN]; // 6
    uint32_t first_seen;               // 4
    uint32_t last_seen;                // 4
    uint16_t device_id;                // 2
    bool said_hello;                   // 1
} ble_badge_active_entry_t;

extern int try_to_add_to_active(uint8_t *address, uint16_t device_id, int8_t rssi, char *name);
extern ble_badge_active_entry_t *in_active_list(uint8_t *address, uint16_t device_id, char *name);
extern void sort_active(bool reset_timers);

//
// Prepare list of badges for the "Nearby badges" display
//
typedef struct {
    char text[20];
} ble_badge_list_menu_text_t;

// the number of badges we fetch to display in teh nearby menu option
#define NEARBY_BADGE_LIST_LEN 38

extern int get_nearby_badge_list(int size, ble_badge_list_menu_text_t *list); 
extern int get_nearby_badge_count(); 

#endif /* UTIL_BLE_LISTS_H_ */
