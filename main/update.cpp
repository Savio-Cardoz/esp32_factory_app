#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdio>
#include <cstdlib>

static const char *TAG = "FACTORY_UPDATER";

void perform_firmware_update()
{
    ESP_LOGI(TAG, "Starting firmware update from SD card...");

    // 1. Locate the Target Partition (ota_0)
    const esp_partition_t *update_partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_APP,
        ESP_PARTITION_SUBTYPE_APP_OTA_0,
        nullptr);

    if (update_partition == nullptr)
    {
        ESP_LOGE(TAG, "Target partition ota_0 not found in partition table!");
        return;
    }

    // 2. Open the Update File on the SD Card
    FILE *file = fopen("/sdcard/update.bin", "rb");
    if (file == nullptr)
    {
        ESP_LOGE(TAG, "Failed to open /sdcard/update.bin. File missing?");
        return;
    }

    // 3. Begin the OTA Write Process
    esp_ota_handle_t update_handle = 0;
    // OTA_WITH_SEQUENTIAL_WRITES is highly recommended for sequential file reading
    esp_err_t err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
        fclose(file);
        return;
    }

    // 4. Stream Data from SD Card to Flash Memory
    const size_t BUF_SIZE = 4096; // 4KB chunks are optimal for flash operations
    char *ota_write_data = (char *)malloc(BUF_SIZE);
    if (ota_write_data == nullptr)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for OTA buffer");
        esp_ota_abort(update_handle);
        fclose(file);
        return;
    }

    size_t bytes_read;
    size_t total_bytes_written = 0;
    ESP_LOGI(TAG, "Flashing...");

    while ((bytes_read = fread(ota_write_data, 1, BUF_SIZE, file)) > 0)
    {
        err = esp_ota_write(update_handle, ota_write_data, bytes_read);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "esp_ota_write failed at offset %zu (%s)", total_bytes_written, esp_err_to_name(err));
            esp_ota_abort(update_handle);
            free(ota_write_data);
            fclose(file);
            return;
        }
        total_bytes_written += bytes_read;
    }

    free(ota_write_data);
    fclose(file);
    ESP_LOGI(TAG, "Successfully wrote %zu bytes to flash.", total_bytes_written);

    // 5. Finalize the OTA Update (Validates image headers and checksums natively)
    err = esp_ota_end(update_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_end failed (%s) - File may be corrupted or truncated.", esp_err_to_name(err));
        return;
    }

    // 6. Update the 'otadata' Partition to Point to the New App
    err = esp_ota_set_boot_partition(update_partition);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "Update finalized in otadata. Rebooting into new application!");
        // Small delay to ensure logs are flushed over UART
        vTaskDelay(pdMS_TO_TICKS(500));
        esp_restart();
    }
    else
    {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)", esp_err_to_name(err));
    }
}
