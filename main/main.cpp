#include <stdio.h>
#include <string>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdcard_manager.hpp"

static const char* TAG = "APP_MAIN";

extern "C" void app_main(void)
{
    SDCardConfig sd_config;
    sd_config.mountPoint = "/sdcard";
    sd_config.maxOpenFiles = 5;
    sd_config.allocationUnitSize = 16 * 1024;
    sd_config.pinCmd = GPIO_NUM_41;
    sd_config.pinClk = GPIO_NUM_39;
    sd_config.pinD0 = GPIO_NUM_40;

    IFileSystem* sdcard = new SDCardManager(sd_config);
    std::string fileContent;

    if (sdcard->mount()) {
        ESP_LOGI(TAG, "SD card mounted successfully. You can now perform file operations.");
        if (sdcard->readTextFile("/hello.txt", fileContent)) {
            ESP_LOGI(TAG, "Content of /hello.txt:\n%s", fileContent.c_str());
        } else {
            ESP_LOGE(TAG, "Failed to read /hello.txt");
        }
    }

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
