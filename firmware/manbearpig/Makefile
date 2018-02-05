PROJECT_NAME     := man_bear_pig_dc25
TARGETS          := nrf52832_xxaa
OUTPUT_DIRECTORY := _build

SDK_ROOT ?= SET_SDK_ROOT_ENV_VARIABLE
PROJ_DIR :=
NRFJ_BIN ?= SET_NRFJ_BIN_ENV_VARIABLE

$(OUTPUT_DIRECTORY)/nrf52832_xxaa.out: \
  LINKER_SCRIPT  := gcc_nrf52.ld
# Source files common to all targets
SRC_FILES += \
	$(SDK_ROOT)/components/ble/ble_advertising/ble_advertising.c \
	$(SDK_ROOT)/components/ble/ble_db_discovery/ble_db_discovery.c \
	$(SDK_ROOT)/components/ble/ble_services/ble_nus/ble_nus.c \
	$(SDK_ROOT)/components/ble/common/ble_advdata.c \
	$(SDK_ROOT)/components/ble/common/ble_conn_params.c \
	$(SDK_ROOT)/components/ble/common/ble_conn_state.c \
	$(SDK_ROOT)/components/ble/common/ble_srv_common.c \
	$(SDK_ROOT)/components/ble/nrf_ble_gatt/nrf_ble_gatt.c \
	$(SDK_ROOT)/components/ble/peer_manager/gatt_cache_manager.c \
	$(SDK_ROOT)/components/ble/peer_manager/gatts_cache_manager.c \
	$(SDK_ROOT)/components/ble/peer_manager/id_manager.c \
	$(SDK_ROOT)/components/ble/peer_manager/peer_database.c \
	$(SDK_ROOT)/components/ble/peer_manager/peer_manager.c \
	$(SDK_ROOT)/components/ble/peer_manager/peer_data_storage.c \
	$(SDK_ROOT)/components/ble/peer_manager/peer_id.c \
	$(SDK_ROOT)/components/ble/peer_manager/pm_buffer.c \
	$(SDK_ROOT)/components/ble/peer_manager/pm_mutex.c \
	$(SDK_ROOT)/components/ble/peer_manager/security_dispatcher.c \
	$(SDK_ROOT)/components/ble/peer_manager/security_manager.c \
	$(SDK_ROOT)/components/drivers_nrf/clock/nrf_drv_clock.c \
	$(SDK_ROOT)/components/drivers_nrf/common/nrf_drv_common.c \
	$(SDK_ROOT)/components/drivers_nrf/comp/nrf_drv_comp.c \
	$(SDK_ROOT)/components/drivers_nrf/gpiote/nrf_drv_gpiote.c \
	$(SDK_ROOT)/components/drivers_nrf/hal/nrf_saadc.c \
	$(SDK_ROOT)/components/drivers_nrf/ppi/nrf_drv_ppi.c \
	$(SDK_ROOT)/components/drivers_nrf/rng/nrf_drv_rng.c \
	${SDK_ROOT}/components/drivers_nrf/rtc/nrf_drv_rtc.c \
	$(SDK_ROOT)/components/drivers_nrf/saadc/nrf_drv_saadc.c \
	$(SDK_ROOT)/components/drivers_nrf/spi_master/nrf_drv_spi.c \
	$(SDK_ROOT)/components/drivers_nrf/timer/nrf_drv_timer.c \
	$(SDK_ROOT)/components/drivers_nrf/twi_master/nrf_drv_twi.c \
	$(SDK_ROOT)/components/libraries/block_dev/sdc/nrf_block_dev_sdc.c \
	$(SDK_ROOT)/components/libraries/button/app_button.c \
	$(SDK_ROOT)/components/libraries/csense/nrf_csense.c \
	$(SDK_ROOT)/components/libraries/csense_drv/nrf_drv_csense.c \
	$(SDK_ROOT)/components/libraries/fds/fds.c \
	$(SDK_ROOT)/components/libraries/fifo/app_fifo.c \
	$(SDK_ROOT)/components/libraries/fstorage/fstorage.c \
	$(SDK_ROOT)/components/libraries/gpiote/app_gpiote.c \
	$(SDK_ROOT)/components/libraries/hardfault/hardfault_implementation.c \
	$(SDK_ROOT)/components/libraries/queue/nrf_queue.c \
	$(SDK_ROOT)/components/libraries/scheduler/app_scheduler.c \
	$(SDK_ROOT)/components/libraries/sdcard/app_sdcard.c \
	$(SDK_ROOT)/components/libraries/timer/app_timer.c \
	$(SDK_ROOT)/components/libraries/util/app_error.c \
	$(SDK_ROOT)/components/libraries/util/app_error_weak.c \
	$(SDK_ROOT)/components/libraries/util/app_util_platform.c \
	$(SDK_ROOT)/components/libraries/util/nrf_assert.c \
	$(SDK_ROOT)/components/libraries/util/sdk_mapped_flags.c \
	$(SDK_ROOT)/components/nfc/t2t_lib/hal_t2t/hal_nfc_t2t.c \
	$(SDK_ROOT)/components/nfc/ndef/generic/message/nfc_ndef_msg.c \
	$(SDK_ROOT)/components/nfc/ndef/text/nfc_text_rec.c \
	$(SDK_ROOT)/components/nfc/ndef/generic/record/nfc_ndef_record.c \
	$(SDK_ROOT)/components/softdevice/common/softdevice_handler/softdevice_handler.c \
	$(SDK_ROOT)/components/toolchain/gcc/gcc_startup_nrf52.S \
	$(SDK_ROOT)/components/toolchain/system_nrf52.c \
	$(SDK_ROOT)/external/fatfs/port/diskio_blkdev.c \
	$(SDK_ROOT)/external/fatfs/src/ff.c \
	$(SDK_ROOT)/external/segger_rtt/RTT_Syscalls_GCC.c \
	$(SDK_ROOT)/external/segger_rtt/SEGGER_RTT.c \
	$(SDK_ROOT)/external/segger_rtt/SEGGER_RTT_printf.c \
	bling/mbp_bling.c \
	bling/mbp_custom_bling.c \
	ntshell/core/ntlibc.c \
	ntshell/core/ntshell.c \
	ntshell/core/text_editor.c \
	ntshell/core/text_history.c \
	ntshell/core/vtrecv.c \
	ntshell/core/vtsend.c \
	ntshell/util/ntopt.c \
	partcl/tcl.c \
	util/util.c \
	util/util_ble.c \
	util/util_hello.c \
	util/util_ble_lists.c \
	util/util_nfc.c \
	util/util_button.c \
	util/util_gfx.c \
	util/util_led.c \
	util/util_math.c \
	util/util_sd.c \
	util/util_tilt.c \
	util/util_crypto.c \
	beacon_ble.c \
	chip8.c \
	drv_apa102.c \
	drv_st7735.c \
	drv_ws2812b.c \
	joco_gameplay.c \
	joco_db.c \
	score_ble.c \
	main.c \
	mbp_medea.c \
	mbp_menu.c \
	mbp_state.c \
	mbp_system.c \
	mbp_tcl.c \
	mbp_term.c \
	mbp_ui.c \
	skifree.c

# Include folders common to all targets
INC_FOLDERS += \
	$(SDK_ROOT)/components \
	$(SDK_ROOT)/components/ble/ble_db_discovery \
	$(SDK_ROOT)/components/ble/ble_advertising \
	$(SDK_ROOT)/components/ble/ble_dtm \
	$(SDK_ROOT)/components/ble/ble_services/ble_ancs_c \
	$(SDK_ROOT)/components/ble/ble_services/ble_ans_c \
	$(SDK_ROOT)/components/ble/ble_services/ble_bas \
	$(SDK_ROOT)/components/ble/ble_services/ble_bas_c \
	$(SDK_ROOT)/components/ble/ble_services/ble_cscs \
	$(SDK_ROOT)/components/ble/ble_services/ble_cts_c \
	$(SDK_ROOT)/components/ble/ble_services/ble_dfu \
	$(SDK_ROOT)/components/ble/ble_services/ble_dis \
	$(SDK_ROOT)/components/ble/ble_services/ble_gls \
	$(SDK_ROOT)/components/ble/ble_services/ble_hids \
	$(SDK_ROOT)/components/ble/ble_services/ble_hrs \
	$(SDK_ROOT)/components/ble/ble_services/ble_hrs_c \
	$(SDK_ROOT)/components/ble/ble_services/ble_hts \
	$(SDK_ROOT)/components/ble/ble_services/ble_ias \
	$(SDK_ROOT)/components/ble/ble_services/ble_ias_c \
	$(SDK_ROOT)/components/ble/ble_services/ble_lbs \
	$(SDK_ROOT)/components/ble/ble_services/ble_lbs_c \
	$(SDK_ROOT)/components/ble/ble_services/ble_lls \
	$(SDK_ROOT)/components/ble/ble_services/ble_nus \
	$(SDK_ROOT)/components/ble/ble_services/ble_nus_c \
	$(SDK_ROOT)/components/ble/ble_services/ble_rscs \
	$(SDK_ROOT)/components/ble/ble_services/ble_rscs_c \
	$(SDK_ROOT)/components/ble/ble_services/ble_tps \
	$(SDK_ROOT)/components/ble/ble_racp \
	$(SDK_ROOT)/components/ble/common \
	$(SDK_ROOT)/components/ble/nrf_ble_gatt \
	$(SDK_ROOT)/components/ble/nrf_ble_qwr \
	$(SDK_ROOT)/components/ble/peer_manager \
	$(SDK_ROOT)/components/device \
	$(SDK_ROOT)/components/drivers_nrf/adc \
	$(SDK_ROOT)/components/drivers_nrf/clock \
	$(SDK_ROOT)/components/drivers_nrf/common \
	$(SDK_ROOT)/components/drivers_nrf/comp \
	$(SDK_ROOT)/components/drivers_nrf/delay \
	$(SDK_ROOT)/components/drivers_nrf/gpiote \
	$(SDK_ROOT)/components/drivers_nrf/hal \
	$(SDK_ROOT)/components/drivers_nrf/i2s \
	$(SDK_ROOT)/components/drivers_nrf/lpcomp \
	$(SDK_ROOT)/components/drivers_nrf/pdm \
	$(SDK_ROOT)/components/drivers_nrf/ppi \
	$(SDK_ROOT)/components/drivers_nrf/pwm \
	$(SDK_ROOT)/components/drivers_nrf/qdec \
	$(SDK_ROOT)/components/drivers_nrf/rng \
	$(SDK_ROOT)/components/drivers_nrf/rtc \
	$(SDK_ROOT)/components/drivers_nrf/saadc \
	$(SDK_ROOT)/components/drivers_nrf/spi_master \
	$(SDK_ROOT)/components/drivers_nrf/spi_slave \
	$(SDK_ROOT)/components/drivers_nrf/swi \
	$(SDK_ROOT)/components/drivers_nrf/timer \
	$(SDK_ROOT)/components/drivers_nrf/twi_master \
	$(SDK_ROOT)/components/drivers_nrf/uart \
	$(SDK_ROOT)/components/drivers_nrf/wdt \
	$(SDK_ROOT)/components/libraries/block_dev \
	$(SDK_ROOT)/components/libraries/block_dev/sdc \
	$(SDK_ROOT)/components/libraries/button \
	$(SDK_ROOT)/components/libraries/crc16 \
	$(SDK_ROOT)/components/libraries/crc32 \
	$(SDK_ROOT)/components/libraries/csense \
	$(SDK_ROOT)/components/libraries/csense_drv \
	$(SDK_ROOT)/components/libraries/experimental_section_vars \
	$(SDK_ROOT)/components/libraries/fds \
	$(SDK_ROOT)/components/libraries/fifo \
	$(SDK_ROOT)/components/libraries/fstorage \
	$(SDK_ROOT)/components/libraries/gpiote \
	$(SDK_ROOT)/components/libraries/hardfault \
	$(SDK_ROOT)/components/libraries/hci \
	$(SDK_ROOT)/components/libraries/led_softblink \
	$(SDK_ROOT)/components/libraries/log \
	$(SDK_ROOT)/components/libraries/log/src \
	$(SDK_ROOT)/components/libraries/low_power_pwm \
	$(SDK_ROOT)/components/libraries/mailbox \
	$(SDK_ROOT)/components/libraries/mem_manager \
	$(SDK_ROOT)/components/libraries/pwm \
	$(SDK_ROOT)/components/libraries/queue \
	$(SDK_ROOT)/components/libraries/scheduler \
	$(SDK_ROOT)/components/libraries/sdcard \
	$(SDK_ROOT)/components/libraries/slip \
	$(SDK_ROOT)/components/libraries/timer \
	$(SDK_ROOT)/components/libraries/twi \
	$(SDK_ROOT)/components/libraries/uart \
	$(SDK_ROOT)/components/libraries/util \
	$(SDK_ROOT)/components/softdevice/common/softdevice_handler \
	$(SDK_ROOT)/components/softdevice/s132/headers \
	$(SDK_ROOT)/components/toolchain \
	$(SDK_ROOT)/components/toolchain/cmsis/include \
	$(SDK_ROOT)/components/toolchain/gcc \
	$(SDK_ROOT)/components/nfc/t2t_lib/hal_t2t \
	$(SDK_ROOT)/components/nfc/t2t_lib \
	$(SDK_ROOT)/components/nfc/ndef/generic/message \
	$(SDK_ROOT)/components/nfc/ndef/generic/record \
	$(SDK_ROOT)/components/nfc/ndef/text \
	$(SDK_ROOT)/external/fatfs/port \
	$(SDK_ROOT)/external/fatfs/src \
	$(SDK_ROOT)/external/protothreads \
	$(SDK_ROOT)/external/protothreads/pt-1.4 \
	$(SDK_ROOT)/external/segger_rtt \
	$(SDK_ROOT)/components/softdevice/s132/headers/nrf52 \
	adafruit \
	bling \
	config \
	fonts \
	ntshell/core \
	ntshell/util \
	partcl \
	util \
	$(PROJ_ROOT)

# Libraries common to all targets
LIB_FILES += \
  $(SDK_ROOT)/components/nfc/t2t_lib/nfc_t2t_lib_gcc.a \

# C flags common to all targets
CFLAGS += -DDEBUG
CFLAGS += -DDEBUG_NRF
CFLAGS += -DNRF52
CFLAGS += -DBOARD_PCA10040
CFLAGS += -DNRF52832
CFLAGS += -DNRF52_PAN_12
CFLAGS += -DNRF52_PAN_15
CFLAGS += -DNRF52_PAN_20
CFLAGS += -DNRF52_PAN_54
CFLAGS += -DNRF52_PAN_58
CFLAGS += -DNRF52_PAN_55
CFLAGS += -DNRF52_PAN_62
CFLAGS += -DNRF52_PAN_63
CFLAGS += -DNRF52_PAN_64
CFLAGS += -DNRF52_PAN_31
CFLAGS += -DNRF52_PAN_30
CFLAGS += -DNRF52_PAN_51
CFLAGS += -DNRF52_PAN_36
CFLAGS += -DNRF52_PAN_53
CFLAGS += -DS132
#CFLAGS += -DCONFIG_GPIO_AS_PINRESET
#CFLAGS += -DCONFIG_NFCT_PINS_AS_GPIOS
CFLAGS += -DBLE_STACK_SUPPORT_REQD
CFLAGS += -DNRF_SD_BLE_API_VERSION=3
CFLAGS += -DSWI_DISABLE0
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs --std=gnu99
CFLAGS +=  -Wall -Werror -O0 -g3
# This appears to erroneously trip with uint8_t, so make it a warning
CFLAGS += -Wno-error=char-subscripts
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums
# generate dependency output file
CFLAGS += -MP -MD

# C++ flags common to all targets
CXXFLAGS += \

# Assembler flags common to all targets
ASMFLAGS += -x assembler-with-cpp
ASMFLAGS += -DNRF52
ASMFLAGS += -DBOARD_PCA10040
ASMFLAGS += -DNRF52832
ASMFLAGS += -DNRF52_PAN_64
ASMFLAGS += -DNRF52_PAN_12
ASMFLAGS += -DNRF52_PAN_15
ASMFLAGS += -DNRF52_PAN_58
ASMFLAGS += -DNRF52_PAN_55
ASMFLAGS += -DNRF52_PAN_54
ASMFLAGS += -DNRF52_PAN_31
ASMFLAGS += -DNRF52_PAN_30
ASMFLAGS += -DNRF52_PAN_51
ASMFLAGS += -DNRF52_PAN_36
ASMFLAGS += -DNRF52_PAN_53
ASMFLAGS += -DS132
#ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DCONFIG_NFCT_PINS_AS_GPIOS
ASMFLAGS += -DBLE_STACK_SUPPORT_REQD
ASMFLAGS += -DNRF_SD_BLE_API_VERSION=3
ASMFLAGS += -DSWI_DISABLE0
ASMFLAGS += -DNRF52_PAN_20
ASMFLAGS += -DSOFTDEVICE_PRESENT
ASMFLAGS += -DNRF52_PAN_62
ASMFLAGS += -DNRF52_PAN_63
ASMFLAGS += -D__STARTUP_CONFIG

# Linker flags
LDFLAGS += -mthumb -mabi=aapcs -L $(TEMPLATE_PATH) -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs -lc -lnosys
# Enable printf float support
LDFLAGS += -u _printf_float

.PHONY: $(TARGETS) default all clean help flash  flash_softdevice
# Default target - first one defined
default: nrf52832_xxaa

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo 	nrf52832_xxaa
	@echo 	flash_softdevice

TEMPLATE_PATH := $(SDK_ROOT)/components/toolchain/gcc

include $(TEMPLATE_PATH)/Makefile.common
$(foreach target, $(TARGETS), $(call define_target, $(target)))
-include $(foreach target, $(TARGETS), $($(target)_dependencies))

# Flash the program
flash: $(OUTPUT_DIRECTORY)/nrf52832_xxaa.hex
	@echo Flashing: $<
	$(NRFJ_BIN)/nrfjprog/nrfjprog --program $< -f nrf52 --sectorerase
	$(NRFJ_BIN)/nrfjprog/nrfjprog --reset -f nrf52
# Flash softdevice
flash_softdevice:
	@echo Flashing: s132_nrf52_3.0.0_softdevice.hex
	$(NRFJ_BIN)/nrfjprog/nrfjprog --program $(SDK_ROOT)/components/softdevice/s132/hex/s132_nrf52_3.0.0_softdevice.hex -f nrf52 --chiperase
	$(NRFJ_BIN)/nrfjprog/nrfjprog --reset -f nrf52

full_erase:
	@echo ***Erasing NRF52! Soft Device will need to be reflashed!***
	$(NRFJ_BIN)/nrfjprog/nrfjprog -e -f nrf52
