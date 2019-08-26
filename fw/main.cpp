/* 
 * Aware
 */

#include "mbed.h"
#include "system_manager.h"
#include "ov2640.h"
#include "sim7100.h"
#include "base64_encode.h"

using namespace aware;

/*************
 * 
 * remote server for proof-of-concept:
 * root@165.227.21.159
 * password: Aware123!
 * 
 *************/

RawSerial *g_modem_uart;
DigitalOut *g_modem_pwr;
DigitalOut *g_modem_rst;
RawSerial *g_console_uart;
SPI *g_camera_spi;
DigitalOut *g_camera_cs;
I2C *g_camera_i2c;

char g_frame_buffer_base64[200000];


void init()
{
    g_console_uart = new RawSerial(PD_8, PD_9, 115200);

    g_modem_uart = new RawSerial(PD_5, PD_6, 115200);

    g_modem_pwr = new DigitalOut(PA_3);

    g_modem_rst = new DigitalOut(PC_0);

    g_camera_spi = new SPI(PA_7, PA_6, PA_5);

    g_camera_cs = new DigitalOut(PD_14);

    g_camera_i2c = new I2C(PB_9, PB_8);

    sim7100_init();

    ov2640_init();
}

// main() runs in its own thread in the OS
int main()
{
    init();

    /*********************
     * frame capture example
     * 
     * to convert to JPG image
     * 1. save console output to image.txt
     * 2. `cat image.txt | base64 --decode > image.jpg`
     * ******************/
    uint32_t size = 0;
    char* buf;

    while( 1 )
    {
        g_console_uart->printf("menu:\r\n"
                                "1. take photo\r\n"
                                "2. upload it\r\n"
                                "3. dump it\r\n"
                                "4. power cycle modem\r\n\r\n");

        while( 0 == g_console_uart->readable() ) { }
        
        switch( g_console_uart->getc() )
        {
            case '1':
                size = ov2640_capture(&buf);
                break;
            case '2':
                base64_encode(g_frame_buffer_base64, buf + 2, size - 2); // first two bytes are unused
                sim7100_http_post(g_frame_buffer_base64, "joel.jpg.base64", "165.227.21.159");
                break;
            case '3':
                base64_encode(g_frame_buffer_base64, buf + 2, size - 2); // first two bytes are unused
                //g_console_uart->printf(g_frame_buffer_base64);
                break;
            case '4':
                sim7100_power_cycle();
                break;
            case '5':
                g_console_uart->printf("test\r\n");
                break;
            default:
                break;
        }
    }

    // Soft restart system upon failure
    while(true) {
        SystemManager system;
        int status = system.run();
    }
}
