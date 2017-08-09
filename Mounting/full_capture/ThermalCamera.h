#ifndef THERMALCAMERA_H
#define THERMALCAMERA_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <limits.h>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>   // to check if file exists
#include <iostream>     // std::cout
#include <fstream>      // file
#include <sys/time.h>   // gettimeofday
#include <unistd.h>     // usleep
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define VOSPI_FRAME_SIZE (164)

class ThermalCamera {
public:
    ThermalCamera();
    ThermalCamera(const ThermalCamera& orig);
    virtual ~ThermalCamera();
    int initilize(void);
    void pabort(const char *s);
    void save_pgm_file(void);
    int transfer(void);

private:
    uint8_t lepton_frame_packet[VOSPI_FRAME_SIZE];
    char dir_name[32], record_name[32];
    struct spi_ioc_transfer tr;
    unsigned int lepton_image[80][80];
    int fd;
    FILE *fcsv;
};

#endif /* THERMALCAMERA_H */

