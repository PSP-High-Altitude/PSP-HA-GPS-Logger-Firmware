/* main.c
Copyright 2021 Carl John Kugler III

Licensed under the Apache License, Version 2.0 (the License); you may not use
this file except in compliance with the License. You may obtain a copy of the
License at

   http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an AS IS BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdint.h>
#include <time.h>
//
#include "pico/stdlib.h"
//
#include "hw_config.h"
#include "f_util.h"
#include "ff.h"

/*

This file should be tailored to match the hardware design.

See 
https://github.com/carlk3/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/tree/main#customizing-for-the-hardware-configuration

*/

#include "hw_config.h"

/* SDIO Interface */
static sd_sdio_if_t sdio_if = {
    /*
    Pins CLK_gpio, D1_gpio, D2_gpio, and D3_gpio are at offsets from pin D0_gpio.
    The offsets are determined by sd_driver\SDIO\rp2040_sdio.pio.
        CLK_gpio = (D0_gpio + SDIO_CLK_PIN_D0_OFFSET) % 32;
        As of this writing, SDIO_CLK_PIN_D0_OFFSET is 30,
            which is -2 in mod32 arithmetic, so:
        CLK_gpio = D0_gpio -2.
        D1_gpio = D0_gpio + 1;
        D2_gpio = D0_gpio + 2;
        D3_gpio = D0_gpio + 3;
    */
    .CMD_gpio = 17,
    .D0_gpio = 18,
    .baud_rate = 15 * 1000 * 1000  // 15 MHz
};

/* Hardware Configuration of the SD Card socket "object" */
static sd_card_t sd_card = {
    /* "pcName" is the FatFs "logical drive" identifier.
    (See http://elm-chan.org/fsw/ff/doc/filename.html#vol) */
    .pcName = "0:",
    .type = SD_IF_SDIO,
    .sdio_if_p = &sdio_if
};

/* Callbacks used by the library: */
size_t sd_get_num() { return 1; }

sd_card_t *sd_get_by_num(size_t num) {
    if (0 == num)
        return &sd_card;
    else
        return NULL;
}

uint64_t buffer_write_us(FIL* fp, uint32_t* buf, uint32_t len) {
    uint64_t start_us = time_us_64();

    // We assume the file is already open
    uint32_t bw;
    FRESULT fr = f_write(fp, buf, len/sizeof(uint32_t), &bw);
    if (fr != FR_OK) {
        panic("f_write error: %s (%d\n)", FRESULT_str(fr), fr);
    }

    uint64_t stop_us = time_us_64();

    return stop_us - start_us;
}

void calc_write_speed(uint32_t* buf, uint32_t len, uint32_t total_mb) {
    FRESULT fr;
    FIL fil;

    printf("Starting write of %u MB with %u B chunks\n",
           total_mb, len * sizeof(uint32_t));

    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    const char* const filename = "dump.b";
    fr = f_open(&fil, filename, FA_WRITE);
    if (FR_OK != fr && FR_EXIST != fr)
        panic("f_open(%s) error: %s (%d)\n", filename, FRESULT_str(fr), fr);

    uint32_t start_us = time_us_64();

    int passes = (total_mb * 1000 * 1000) / (len * sizeof(uint32_t));
    uint32_t total_driver_us = 0;
    for (int i = 0; i < passes; i++) {
        total_driver_us += buffer_write_us(&fil, buf, len);
    }
    uint32_t total_written_bytes = len * sizeof(uint32_t)* passes;

    uint32_t total_elapsed_us = time_us_64() - start_us;

    fr = f_close(&fil);
    if (FR_OK != fr) {
        printf("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
    }
    
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    printf("Wrote %u B in %u us (%u kB/s)\n",
           total_written_bytes, total_elapsed_us,
           total_written_bytes/(total_elapsed_us/1000));

    printf("%.2f%% of time spent in driver code\n",
           ((float)total_driver_us*100)/((float)total_elapsed_us));
}

int main() {
    stdio_init_all();

    printf("Waiting for 5 seconds...\n");
    sleep_ms(5000);

    printf("Hello, world!\n");

    // See FatFs - Generic FAT Filesystem Module, "Application Interface",
    // http://elm-chan.org/fsw/ff/00index_e.html
    sd_card_t *pSD = sd_get_by_num(0);
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if (FR_OK != fr) panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
    FIL fil;
    const char* const filename = "filename.txt";
    fr = f_open(&fil, filename, FA_OPEN_APPEND | FA_WRITE);
    if (FR_OK != fr && FR_EXIST != fr)
        panic("f_open(%s) error: %s (%d)\n", filename, FRESULT_str(fr), fr);
    if (f_printf(&fil, "Hello, world!\n") < 0) {
        printf("f_printf failed\n");
    }
    fr = f_close(&fil);
    if (FR_OK != fr) {
        panic("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
    }

    printf("Goodbye, world!\n");

    // Create buffer to dump data from
    #define BUF_SIZE 128
    uint32_t buf[BUF_SIZE];
    for (int i = 0; i < BUF_SIZE; i++) {
        buf[i] = i;
    }
    
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    printf("Entering test loop\n");

    while (true) {
        calc_write_speed(buf, BUF_SIZE, 10);
        printf("\nSleeping for 5 seconds... ");
        for (int i = 4; i > 0; i--) {
            sleep_ms(1000);
            printf("%d... ", i);
        }
        printf("\n\n");
    }
}
