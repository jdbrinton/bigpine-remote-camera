#ifndef SIM7100_H
#define SIM7100_H

void sim7100_init();
void sim7100_http_post(char* msg, const char* file_name, const char* url);
void sim7100_power_cycle();

#endif
