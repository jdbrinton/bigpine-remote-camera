#include <stdlib.h>
#include "configuration_manager.h"

/*
{
    "greeting" : "Hello World!",
    "led" : {
        "frequency" : 0.1e01,
        "pulse_width" : 3,
        "pause_width" : 5,
        "pulse" : true    
    }
}
*/

namespace aware
{
ConfigurationManager::ConfigurationManager()
{
    printf("Initializing configuration manager\n");

    // Initialize all configuration parameters to their default values here;
    _led_frequency = MBED_CONF_APP_DEFAULT_LED_FREQUENCY;
    set(_greeting, MBED_CONF_APP_DEFAULT_GREETING, sizeof(_greeting));
    _pulse_width = 3;
    _pause_width = 9;
    _pulse = false;
}

ConfigurationManager::~ConfigurationManager()
{
    printf("Deinitializing configuration manager\n");
}

int ConfigurationManager::parse_root(JsonObject &p) {
    while (p.has_next())
    {
        if (p.match_string())
        {
            Substring s = p.consume_string();
            if (s == "led" && p.match_object())
            {
                JsonObject obj = p.consume_object();
                parse_led(obj);
            }
            else if (s == "greeting" && p.match_string())
            {
                p.consume_string().copy_to_c_string(_greeting, sizeof(_greeting));
                printf("greeting: %s\n", _greeting);
            }
            else
            {
                p.consume_next();
            }
        }
        else
        {
            // Parse error, expecting label but found a non-string element
            return -1;
        }
    }

    return 0;
}

int ConfigurationManager::parse_pulse_width(JsonObject &p) {
    if(p.has_next()) {
        if (p.match_number()) {
            _pulse_width = p.consume_int();
            printf("pulse width: %d\n", _pulse_width);
        } else {
            p.consume_next();
        }
    } else {
        return -1;
    }
    if(p.has_next()) {
        if (p.match_number()) {
            _pause_width = p.consume_int();
            printf("pause width: %d\n", _pause_width);
        } else {
            p.consume_next();
        }
    }  else {
        return -2;
    }
    return 0;
}


int ConfigurationManager::parse_led(JsonObject &p) {
    while (p.has_next())
    {
        if (p.match_string())
        {
            Substring s = p.consume_string();
            if (s == "frequency" && p.match_number())
            {
                _led_frequency = p.consume_double();
                printf("led_frequency: %f\n", _led_frequency);
            }
            else if (s == "pulse" && p.match_boolean())
            {
                _pulse = p.consume_boolean();
                printf("pulse: %d\n", _pulse);
            }
            else if (s == "width" && p.match_array())
            {
                JsonObject arr = p.consume_array();
                parse_pulse_width(arr);
            }
            else
            {
                p.consume_next();
            }
        }
        else
        {
            // Parse error, expecting label but found a non-string element
            return -1;
        }
    }

    return 0;
}


int ConfigurationManager::init(const char *config_file_path)
{

    printf("Reading configuration from file: ");
    printf(config_file_path);
    printf("\n");

    JsonParser p;
    JsonObject root = p.parseJsonFile(config_file_path);

    int status = parse_root(root);

    return status;
}

void ConfigurationManager::set(char *target, const char *source, int size)
{
    if (size >= 0 && strlen(source) + 1 <= size)
    {
        strcpy(target, source);
    }
}

} // namespace aware