#include "mbed.h"
#include "ov2640.h"

extern I2C *g_camera_i2c;
extern SPI *g_camera_spi;
extern DigitalOut *g_camera_cs;
extern RawSerial *g_console_uart;

char g_frame_buffer[150000];

int write_i2c(char reg, char value)
{
    int ret;
    char cmd[2];

    cmd[0] = reg;
    cmd[1] = value;

    ret = g_camera_i2c->write(OV2640_7BIT_ADDR << 1, cmd, 2);

    return ret;
}

int write_i2c(const struct sensor_reg reglist[])
{
    int ret;
    char reg_addr = 0;
    char reg_val = 0;
    const struct sensor_reg *next = reglist;
    while ((reg_addr != 0xff) | (reg_val != 0xff)) {
        reg_addr = *((unsigned char *)&next->reg);
        reg_val = *((unsigned char *)&next->val);
        ret = write_i2c(reg_addr, reg_val);
        if( 0 != ret) return ret;
        next++;
    }

    return 0;
}

void write_spi(char reg, char val)
{
    char cmd[2];

    cmd[0] = RWBIT | reg;
    cmd[1] = val;

    g_camera_cs->write(0);
    g_camera_spi->write(cmd, 2, 0, 0);
    g_camera_cs->write(1);
}

char read_spi(char reg)
{
    char cmd[2];
    char buf[2];

    cmd[0] = 0x7F & reg;
    cmd[1] = 0;

    g_camera_cs->write(0);
    g_camera_spi->write(cmd, 2, buf, 2);
    g_camera_cs->write(1);

    return buf[1];
}

char read_spi_buf(char reg, char* buf, uint32_t size)
{
    char cmd[1];

    cmd[0] = reg;

    g_camera_cs->write(0);
    g_camera_spi->write(cmd, 1, buf, size);
    g_camera_cs->write(1);
}

void start_capture(void)
{
	write_spi(ARDUCHIP_FIFO, FIFO_START_MASK);
}

void clear_fifo_flag(void)
{
	write_spi(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
}

uint32_t read_fifo_length(void)
{
    uint32_t len1, len2, len3, length;

    len1 = read_spi(FIFO_SIZE1);
    len2 = read_spi(FIFO_SIZE2);
    len3 = read_spi(FIFO_SIZE3) & 0x7f;
    length = ((len3 << 16) | (len2 << 8) | len1) & 0x07fffff;

    return length;
}

uint32_t ov2640_capture(char** buf_in)
{
    uint32_t size;

    wait(1.0);

    clear_fifo_flag();

    start_capture();

#ifdef MBED_DEBUG
    g_console_uart->printf("capturing");
#endif
    
    while( CAP_DONE_MASK != (CAP_DONE_MASK & read_spi(ARDUCHIP_TRIG)) ) {
#ifdef MBED_DEBUG
        g_console_uart->printf(".");
#endif
    }

    size = read_fifo_length();

    size = size>sizeof(g_frame_buffer)?sizeof(g_frame_buffer):size;
    
#ifdef MBED_DEBUG
    g_console_uart->printf("\r\nfifo size: %u\r\n", size);
#endif

    read_spi_buf(0x3C, g_frame_buffer, size);

    *buf_in = g_frame_buffer;

    return size;
}

void ov2640_init()
{
    g_camera_cs->write(1);
    g_camera_spi->format(8, 0); // POL = 0, PHA = 0 (ignore ArduCam datasheet)
    g_camera_spi->frequency(8000000);

    if( 0 != write_i2c(0xff, 0x01) ) {
#ifdef MBED_DEBUG
        g_console_uart->printf("I2C write failure. "
                               "Camera is probably disconnected. "
                               "Going to continue anyway.\r\n");
#endif
    }

    write_i2c(0x12, 0x80);

    wait(0.1f);

    write_i2c(OV2640_JPEG_INIT);
    write_i2c(OV2640_YUV422);
    write_i2c(OV2640_JPEG);
    write_i2c(0xff, 0x01);
    write_i2c(0x15, 0x00);
    write_i2c(OV2640_160x120_JPEG);

    wait(1.0f); // don't know if it has to be this long

#ifdef MBED_DEBUG
    char version;

    version = read_spi(0x40);
    g_console_uart->printf("SPI ArduChip version: %u.%u \r\n", version >> 4, version & 0x0f);
#endif
}
