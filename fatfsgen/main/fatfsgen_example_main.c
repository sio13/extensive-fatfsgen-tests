/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"
#include "sdkconfig.h"

#if CONFIG_EXAMPLE_FATFS_MODE_READ_ONLY
#define EXAMPLE_FATFS_MODE_READ_ONLY true
#else
#define EXAMPLE_FATFS_MODE_READ_ONLY false
#endif

#if CONFIG_FATFS_LFN_NONE
#define EXAMPLE_FATFS_LONG_NAMES false
#else
#define EXAMPLE_FATFS_LONG_NAMES true
#endif

static const char *TAG = "example";


// Mount path for the partition
const char *base_path = "/spiflash";

// Handle of the wear levelling library instance
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

void app_main(void)
{
    ESP_LOGI(TAG, "Mounting FAT filesystem");
    // To mount device we need name of device partition, define base_path
    // and allow format partition in case if it is new one and was not formatted before
    const esp_vfs_fat_mount_config_t mount_config = {
            .max_files = 4,
            .format_if_mount_failed = false,
            .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
    };
    esp_err_t err;
    err = esp_vfs_fat_spiflash_mount_rw_wl(base_path, "storage", &mount_config, &s_wl_handle);


    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return;
    }

    for(int i=0; i<100; i++){

        ESP_LOGI(TAG, "Opening file");

        char file[50] = "/spiflash/helloooooooo";
        char compa[50] = "/spiflash/";
        char str[10];
        sprintf(str, "%d", i);
        strcat(file, str);

        FILE *f;

        // Open file for reading
        ESP_LOGI(TAG, "Reading file");
        f = fopen(file, "rb");
        if (f == NULL) {
            ESP_LOGE(TAG, "Failed to open file for reading");
            return;
        }
        char line[128];
        fgets(line, sizeof(line), f);
        fclose(f);
        // strip newline
        char *pos = strchr(line, '\n');
        if (pos) {
            *pos = '\0';
        }
        strcat(compa, line);
        if(strcmp(compa, file)){
            ESP_LOGE(TAG, "Wrong content!");
            return;
        }
        ESP_LOGI(TAG, "Read from file: '%s'", line);
    }


    ESP_ERROR_CHECK(esp_vfs_fat_spiflash_unmount_rw_wl(base_path, s_wl_handle));

    ESP_LOGI(TAG, "Done");
}
