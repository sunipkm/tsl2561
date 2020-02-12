#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/i2c-dev.h>
#include <signal.h>
#include <errno.h>

volatile sig_atomic_t done = 0 ;

void catch_sigint()
{
    done = 1;
}

void write8(int fd, uint8_t val)
{
    uint8_t buf = val ;
    int res = write(fd, &buf, 1);
    if ( res != 1 )
        perror(__FUNCTION__);
}

void writecmd8(int fd, uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {0x0};
    buf[0] = reg ; buf[1] = val ;
    int res = write(fd, &buf, 2);
    if ( res != 2 )
        perror(__FUNCTION__);
}

uint8_t read8(int fd, uint8_t reg)
{
    write8(fd, reg);
    uint8_t buf = 0x00 ;
    int res = read(fd, &buf, 1);
    if ( res != 1 )
        perror(__FUNCTION__);
    return buf;
}

void write16(int fd, uint16_t val)
{
    uint8_t buf[2] ;
    buf[0] = val >> 8 ;
    buf[1] = 0x00ff & val ;
    int res = write(fd, buf, 2);
    if ( res != 2 )
        perror(__FUNCTION__);
    return;
}

uint16_t read16(int fd, uint8_t cmd)
{
    uint8_t buf[2] = {0x0};
    write8(fd, cmd);
    int res = read(fd, buf, 2);
    if ( res != 2 )
        perror(__FUNCTION__);
    return buf[0] | ((unsigned short)buf[1]) << 8 ;
}

int i2cinit(char fname[], uint8_t addr)
{
    int fd = open(fname, O_RDWR);
    if ( fd < 0 )
        perror("Open");
    if ( ioctl(fd, I2C_SLAVE, addr) < 0 )
        perror("ioctl");
    return fd ;
}

int main()
{
    struct sigaction sa ;
    sa.sa_handler = &catch_sigint;
    sigaction(SIGINT, &sa, NULL);

    int fd = i2cinit("/dev/i2c-1", 0x39);
    uint8_t x = read8(fd, 0x0a);
    printf("Ident: 0x%02x\n", x);
    printf("Powering on...\n");
    writecmd8(fd, 0x80, 0x03);
    sleep(1);
    printf("Power register: 0x%02x\n", read8(fd, 0x80));
    
    printf("Setting timing and gain...\n");
    writecmd8(fd, 0x81, 0x00);
    usleep(100000);
    printf("Timing register: 0x%02x\n", read8(fd, 0x81));
    usleep(100000);

    printf("Setting timing and gain...\n");
    writecmd8(fd, 0x81, 0x01);
    usleep(100000);
    printf("Timing register: 0x%02x\n", read8(fd, 0x81));
    usleep(100000);

    printf("Setting timing and gain...\n");
    writecmd8(fd, 0x81, 0x00);
    usleep(100000);
    printf("Timing register: 0x%02x\n", read8(fd, 0x81));
    usleep(100000);
    while(!done)
    {
        uint16_t bband, ir ;
        bband = read16(fd, 0xac);
        ir = read16(fd, 0xae);
        printf("Lux: 0x%04x 0x%04x\n", bband, ir);
        usleep(2000000);
    }
    close(fd);
    return 0;
}
