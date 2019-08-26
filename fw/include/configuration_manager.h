#ifndef AWARE_CONFIGURATION_MANAGER_H
#define AWARE_CONFIGURATION_MANAGER_H

#include "mbed.h"
#include "json_parser.h"

namespace aware
{

class ConfigurationManager
{
public:
  ConfigurationManager();
  ~ConfigurationManager();
  int init(const char *config_file_path);

  // Read accessors for configuration parameters
  inline double led_frequency() const
  {
    return _led_frequency;
  }
  inline const char *greeting() const
  {
    return _greeting;
  }
  inline bool pulse() const 
  {
    return _pulse;
  }
  inline int pulse_width() const
  {
    return _pulse_width;
  }
  inline int pause_width() const
  {
    return _pause_width;
  }

private:
  void set(char *target, const char *source, int size = -1);
  int parse_root(JsonObject &p);
  int parse_led(JsonObject &p);
  int parse_pulse_width(JsonObject &p);

  // Configuration parameters
  double _led_frequency;
  char _greeting[32];
  bool _pulse;
  int _pulse_width;
  int _pause_width;
};

} // namespace aware

#endif