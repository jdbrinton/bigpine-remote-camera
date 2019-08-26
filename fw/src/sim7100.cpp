#include "sim7100.h"

#include "mbed.h"

extern RawSerial *g_console_uart;
extern RawSerial *g_modem_uart;
extern DigitalOut *g_modem_pwr;
extern DigitalOut *g_modem_rst;

#define BUF_LENGTH 8192
#define QUEUE_LENGTH 100

char buf[BUF_LENGTH * 2] = {0};
int32_t queue[QUEUE_LENGTH] = {0};

int32_t buf_idx = 0;
int32_t queue_head = 0;
int32_t queue_tail = 0;
int32_t queue_cnt = 0;

void modem_recv()
{
    char tmp;

    // get character from UART buffer
    tmp = (char)(g_modem_uart->getc());

    // ignore carriage return characters
    if('\r' == tmp) return;

    // if EOL, double buffer a c-string terminator and buffer new write head
    if( '\n' == tmp ) {

        // ignore duplicate new line characters
        if( '\0' == buf[buf_idx - 1 + BUF_LENGTH] ) return;

        buf[buf_idx] = '\0';
        buf[buf_idx + BUF_LENGTH] = '\0';

        buf_idx = (buf_idx + 1) % BUF_LENGTH;

        queue[++queue_head % QUEUE_LENGTH] = buf_idx;
    }
    // if not EOL then double buffer character
    else {
        buf[buf_idx] = tmp;
        buf[buf_idx + BUF_LENGTH] = tmp;

        buf_idx = (buf_idx + 1) % BUF_LENGTH;
    }
}

char* sim7100_block_for_reply(uint32_t timeout_ms)
{
    uint32_t n = 0;

    // if timeout is set to 0 then block indefinitely on EOL
    while( queue_head == queue_tail )
        if( 0 == timeout_ms ) continue;
        else if( n++ > timeout_ms ) return 0; // timeout
        else wait_ms(1);

    // return last unread write head from queue
    return buf + queue[queue_tail++ % QUEUE_LENGTH];
}

int32_t sim7100_block_for_ok(uint32_t timeout_ms = 5000)
{
    char* tmp;

    while( 1 ) {
        tmp = sim7100_block_for_reply(timeout_ms);

        if( 0 == tmp ) {
            g_console_uart->printf("Modem did not acknowledge with OK\r\n");
            return -1;
        }
        g_console_uart->printf("%s\r\n", tmp);
        if( 0 == strncmp( tmp, "OK", 2) ) return 0;
    }
}

int32_t sim7100_send(const char* cmd, uint32_t timeout_ms = 5000)
{
    g_modem_uart->printf("%s\r\n", cmd);

    return sim7100_block_for_ok(timeout_ms);
}

#define BOUNDARY "---abc---"

void sim7100_http_post(char* msg, const char* file_name, const char* url)
{
    uint32_t length, length_body, length_post;
    char length_body_str[10];

    const char post[] = "POST / HTTP/1.1\r\n"
                        "Host: %s:80\r\n"
                        "Content-Type: multipart/form-data; boundary=---abc---\r\n"
                        "Content-Length: %s\r\n"
                        "\r\n";
    const char body[] = BOUNDARY "\r\n"
                        "Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n"
                        "Content-Type: text/plain\r\n"
                        "\r\n"
                        "%s\r\n"
                        "\r\n"
                        BOUNDARY "\r\n";

    length_body = strlen(file_name) + strlen(msg) + strlen(body) - 2;
    snprintf(length_body_str, sizeof(length_body_str) - 1, "%ld", length_body);

    length_post = strlen(url) + strlen(length_body_str) + strlen(post) - 2;

    length = length_body + length_post;

    if( length > 4096 )
    {
        g_console_uart->printf("packet too big (%ld bytes) to send with HTTPS application (4096 bytes)\r\n", length);
        return;
    }

    g_modem_uart->printf("AT+CHTTPSSTART\r\n");
    sim7100_block_for_ok();

    g_modem_uart->printf("AT+CHTTPSOPSE=\"%s\",80,1\r\n", url);
    sim7100_block_for_ok();

    g_modem_uart->printf("AT+CHTTPSSEND=%ld\r\n", length);
    wait_ms(500); // this is a hacky thing to wait for HTTPS application prompt
    //while( '>' !=  buf[buf_idx - 1 + BUF_LENGTH] ) // this is a hacky thing to wait for HTTPS application prompt

    g_modem_uart->printf(post, url, length_body_str);
    g_modem_uart->printf(body, file_name, msg);
    sim7100_block_for_ok();

    g_modem_uart->printf("AT+CHTTPSRECV=4000\r\n");
    sim7100_block_for_ok();

    g_modem_uart->printf("AT+CHTTPSCLSE\r\n");
    sim7100_block_for_ok();

    g_modem_uart->printf("AT+CHTTPSSTOP\r\n");
    sim7100_block_for_ok();
}

void sim7100_power_cycle()
{
    g_modem_pwr->write(1);
    wait(5.1f);
    g_modem_pwr->write(0);
}

void sim7100_init()
{
    char* tmp;
    
    g_modem_uart->attach(&modem_recv, Serial::RxIrq);

    g_console_uart->printf("booting modem...\r\n");

    // see if modem is off and turn it on

    if( 0 > sim7100_send("AT") ) {
        g_console_uart->printf("toggling power button\r\n");

        g_modem_pwr->write(1);
        wait(5.1f);
        g_modem_pwr->write(0);
        wait(25.0f);

        // see if it responds now
        if( 0 > sim7100_send("AT") ) {;
        
            // try turning it on again
            g_console_uart->printf("toggling power button again\r\n");
            g_modem_pwr->write(1);
            wait(5.1f);
            g_modem_pwr->write(0);
            wait(25.0f);

            if( 0 > sim7100_send("AT") ) {
                g_console_uart->printf("Modem did not boot. "
                                        "It's probably disconnected. "
                                        "Going to continue anyway.\r\n");
                return;
            }
        }
    }

    g_console_uart->printf("modem booted\r\n");

    sim7100_send("ATE0");
    sim7100_send("AT+CFUN=1");
    sim7100_send("ATI"); // modem info
    sim7100_send("AT+CGMI"); // manufacturer
    sim7100_send("AT+CGMM"); // model number
    sim7100_send("AT+CGSN"); // IMEI number
    sim7100_send("AT+CGMR"); // software version
    sim7100_send("AT+CNUM"); // MSISDN
    sim7100_send("AT+CIMI"); // IMSI
    sim7100_send("AT+CPAS"); // mobile activity status
    sim7100_send("AT+CREG"); // mobile network registration status
    sim7100_send("AT+CSQ"); // radio signal strength
    sim7100_send("AT+CBC"); // battery charging status
}

