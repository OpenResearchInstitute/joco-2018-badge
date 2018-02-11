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

//Badge to badge hellos
#define HELLO_INTERVAL                (1000 * 60) // Limit hellos to 1 per minute

static uint32_t m_next_hello;

void hello_init() {
    m_next_hello = util_local_millis() + HELLO_INTERVAL;
}

bool try_to_hello(uint16_t company_id, char *name) {
    uint32_t now;
    now = util_local_millis();
    if (now < m_next_hello) {
	return false;
    } else {
	m_next_hello = now + HELLO_INTERVAL;
	switch (company_id) {
	case COMPANY_ID_JOCO:
	    APP_ERROR_CHECK(app_sched_event_put(name, strlen(name), mbp_bling_hello_joco_schedule_handler));
	    break;
	case COMPANY_ID:
	    APP_ERROR_CHECK(app_sched_event_put(name, strlen(name), mbp_bling_hello_bender_schedule_handler));
	    break;
	case COMPANY_ID_CPV:
	    APP_ERROR_CHECK(app_sched_event_put(NULL, 0, mbp_bling_hello_cpv_schedule_handler));
	    break;
	case COMPANY_ID_DC503:
	    APP_ERROR_CHECK(app_sched_event_put(NULL, 0, mbp_bling_hello_dc503_schedule_handler));
	    break;
	case COMPANY_ID_DC801:
	    APP_ERROR_CHECK(app_sched_event_put(NULL, 0, mbp_bling_hello_dc801_schedule_handler));
	    break;
	case COMPANY_ID_QUEERCON:
	    APP_ERROR_CHECK(app_sched_event_put(NULL, 0, mbp_bling_hello_queercon_schedule_handler));
	    break;
	default:
	    break;
	}
	return true;
    }
}

