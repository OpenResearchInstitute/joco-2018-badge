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

#include "system.h"

#define DB_PREFIX "DB/"
#define DB_PREFIX_LEN 3
// Path length is length of prefix plus 12 for filename, plus a terminator
#define DB_PATH_LEN 16

typedef struct {
    uint8_t gap_addr[BLE_GAP_ADDR_LEN]; // six bytes
    uint16_t device_id;
} contact_db_data_t;

typedef struct {
    char filename[DB_PATH_LEN];
    contact_db_data_t data;
} contact_db_fileinfo_t;

static contact_db_fileinfo_t contact_db_write_data;

static bool __db_data_valid(contact_db_data_t *record, uint8_t *gap_addr, uint16_t device_id) {
    if (memcmp(gap_addr, &record->gap_addr, BLE_GAP_ADDR_LEN))
	return false;
    if (memcmp(&device_id, &record->device_id, sizeof(device_id)))
	return false;
    return true;
}

/*
 * Each entry is stored in a file in the "DB" Directory
 *
 * Entries are only added, never deleted
 * 
 * In order to make it fast and light to see whether an entry has been created before,
 * we maintain files containing lists of the file names we've saved.
 * 
 * We have two pieces of information for each contact we want to save. They are a 6 byte GAP address,
 * And a two byte device_id. Together, those form the eight byte identifier.
 * 
 * We use 8.3 FAT file names. Each file hold multiple records. The file name is generated as follows:
 *   ascii encode the GAP address into a 12 byte string.
 *   set the 9th character of that string to '.'
 *
 * The records in each file contain the GAP address and device_id.
 * 
 */

void gen_filename(char *filename, uint8_t *gap_address, uint16_t device_id) {
    // assumes that filename has 13 bytes plus the length of the prefix available
    strcpy(filename, DB_PREFIX);
    util_hex_encode((uint8_t *) &filename[DB_PREFIX_LEN], gap_address, BLE_GAP_ADDR_LEN);
    filename[DB_PREFIX_LEN+8] = '.';
    filename[DB_PREFIX_LEN+12] = 0;
}

static void __db_save_schedule_handler(void *p_data, uint16_t length) {
    // We're going to assume that the caller has already insured that the record
    // does not exist and add it without checking.
    FRESULT result;
    FIL file;
    UINT count;

    result = f_open(&file, contact_db_write_data.filename, FA_WRITE | FA_OPEN_APPEND);
    if (result != FR_OK) {
	mbp_ui_error("Could not open db w.");
	return;
    }

    result = f_write(&file, (void *) &contact_db_write_data.data, sizeof(contact_db_data_t), &count);
    if (result != FR_OK) {
	mbp_ui_error("Could not write to db.");
	return;
    }

    result = f_close(&file);
    if (result != FR_OK) {
	mbp_ui_error("Could not close db 3.");
	return;
    }
}

bool was_contacted(uint8_t *address, uint16_t device_id) {
    FRESULT result;
    FIL file;
    FILINFO info;
    UINT count;

    char filename[DB_PATH_LEN];
    contact_db_data_t record;

    // convert the address to a filename
    gen_filename(filename, address, device_id);

    // if the file doesn't exist, we know we haven't contacted
    if (f_stat(filename, &info) != FR_OK) {
	return false;
    }

    // If it exists we have to check the records in the file
    result = f_open(&file, filename, FA_READ | FA_OPEN_EXISTING);
    if (result != FR_OK) {
	return false;
    }

    result = f_read(&file, (void *) &record, sizeof(contact_db_data_t), &count);
    while ((result == FR_OK) && (count == sizeof(contact_db_data_t))) {
	// see if this block of data matches
	// -spc- TODO decrypt here in the future
	if (__db_data_valid(&record, address, device_id)) {
	    result = f_close(&file);
	    if (result != FR_OK)
		mbp_ui_error("Could not close db 1.");
	    return true;
	}
	// if we haven't found the matching record, try again
	result = f_read(&file, (void *) &record, sizeof(contact_db_data_t), &count);
    }

    // if we get here, we have reached end of file without finding a record
    result = f_close(&file);
    if (result != FR_OK) {
	mbp_ui_error("Could not close db 2.");
	return false;
    }
    return false;
}

void save_contact(uint8_t *address, uint16_t device_id) {
    // We're going to assume that the caller has already insured that the record
    // does not exist and add it without checking.
    int idx;

#ifdef NO_DB_SAVE
    return;
#endif

    gen_filename(contact_db_write_data.filename, address, device_id);

    // save the information to hand off to the scheduled write
    for(idx=0; idx<BLE_GAP_ADDR_LEN; idx++)
	contact_db_write_data.data.gap_addr[idx] = address[idx];
    contact_db_write_data.data.device_id = device_id;

    // schedule the write
    app_sched_event_put(NULL, 0, __db_save_schedule_handler);
}

uint16_t count_db_entries() {
    FRESULT result;
    DIR dir;
    static FILINFO fno;
    uint16_t count = 0;

    result = f_opendir(&dir, "DB");
    if (result == FR_OK) {
	for (;;) {
	    result = f_readdir(&dir, &fno); /* Read a directory item */
	    if (result != FR_OK || fno.fname[0] == 0)
		break; /* Break on error or end of dir */
	    if (fno.fattrib & AM_DIR) { /* It is a directory */
                //ignore
	    } else { /* It is a file. */
		if (!strcmp(fno.fname, "KEEPTHIS.FIL"))
		    continue;
		count++;
	    }
	}
	f_closedir(&dir);
    }
    return count;
}
