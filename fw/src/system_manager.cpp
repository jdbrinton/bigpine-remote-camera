#include "system_manager.h"
#include "key_value_pair.h"
// #include <limits.h>

namespace aware
{
SystemManager::SystemManager() : _rf(), 
                                 _sd(MBED_CONF_SD_SPI_MOSI, MBED_CONF_SD_SPI_MISO,
                                     MBED_CONF_SD_SPI_CLK, MBED_CONF_SD_SPI_CS),
                                 _fs(MBED_CONF_APP_SD_MOUNT_PATH),
                                 _led1(LED1),
                                 _cfg(),
                                 _sm(&_rf, &_fs, MBED_CONF_APP_SD_MOUNT_PATH, MBED_CONF_APP_STORAGE_ROOT),
                                 _img_storage(&_sm, "images"),
                                 _pcl_storage(&_sm, "pointclouds"),
                                 _log_storage(&_sm, "logs")
{
    printf("****************************** START *****************************\n");
    printf("Initializing system\n");
    _sd.init();
    _fs.mount(&_sd);
    _cfg.init("/" MBED_CONF_APP_SD_MOUNT_PATH "/" MBED_CONF_APP_CONFIG_FILE);
    _sm.init();
    _img_storage.init();
    _pcl_storage.init();
    _log_storage.init();
}

SystemManager::~SystemManager()
{
    printf("Deinitializing system\n");
    _fs.unmount();
    _sd.deinit();
}

int SystemManager::run() {
    printf("Starting system\n");
    printf(_cfg.greeting());
    printf("\n");
    while(true)
    {
        if (!_cfg.pulse()) {
            // Blink LED
            _led1 = !_led1;
            wait(_cfg.led_frequency()/2);
        } else {
            // Pulse led
            for (int i=0; i<_cfg.pulse_width() * 2; ++i) {
                _led1 = !_led1;
                wait(_cfg.led_frequency()/2);         
            }
            for (int i=0; i<_cfg.pause_width() * 2; ++i) {
                wait(_cfg.led_frequency()/2); 
            }
        }
    }
    printf("System finished executing\n");
    return 0;
}

} // namespace aware