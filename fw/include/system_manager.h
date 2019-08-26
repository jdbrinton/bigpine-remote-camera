#ifndef AWARE_SYSTEM_MANAGER_H
#define AWARE_SYSTEM_MANAGER_H

#include "mbed.h"
#include "SDBlockDevice.h"
#include "FATFileSystem.h"
#include "configuration_manager.h"
#include "storage_manager.h"
#include "random.h"

namespace aware
{

class SystemManager
{
  public:
    SystemManager();
    ~SystemManager();
    int run();

  private:
    AIRandomFactory _rf;
    SDBlockDevice _sd;
    FATFileSystem _fs;
    DigitalOut _led1;
    ConfigurationManager _cfg;
    StorageManager _sm;
    StorageBucket _img_storage;
    StorageBucket _pcl_storage;
    StorageBucket _log_storage;
};

} // namespace aware

#endif