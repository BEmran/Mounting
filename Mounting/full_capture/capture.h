#ifndef CAPTURE_H
#define CAPTURE_H

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>   // to check if file exists
#include <iostream>     // std::cout
#include <fstream>      // file
#include "lidar_lite.h"
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static void pabort(const char *s) {
    perror(s);
    abort();
}

static const char *device = "/dev/spidev0.1";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 16000000;
static uint16_t delay;

#define VOSPI_FRAME_SIZE (164)
uint8_t lepton_frame_packet[VOSPI_FRAME_SIZE];
static unsigned int lepton_image[80][80];


char dir_name[32], record_name[32];
FILE *fcsv;

Lidar_Lite lidar(1);

static void save_pgm_file(void);
int transfer(int fd);
int capture(void);
void init(void);
int readLidar(Lidar_Lite* Lidar);

static void save_pgm_file(void) {
    int i;
    int j;
    unsigned int maxval = 0;
    unsigned int minval = UINT_MAX;
    char image_name[32];
    static int image_index = 0;
    int dist = 0;
    //--------------------------- Create image file ----------------------------
    do {
        image_index += 1;
        sprintf(image_name, "%s/IMG_%.4d.pgm", dir_name, image_index);
    } while (access(image_name, F_OK) == 0);
    FILE *f = fopen(image_name, "w");

    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }

    //printf("Calculating min/max values for proper scaling...\n");
    for (i = 0; i < 60; i++) {
        for (j = 0; j < 80; j++) {
            if (lepton_image[i][j] > maxval) {
                maxval = lepton_image[i][j];
            }
            if (lepton_image[i][j] < minval) {
                minval = lepton_image[i][j];
            }
        }
    }
    //printf("maxval = %u\n", maxval);
    //printf("minval = %u\n", minval);

    fprintf(f, "P2\n80 60\n%u\n", maxval - minval);
    for (i = 0; i < 60; i++) {
        for (j = 0; j < 80; j++) {
            fprintf(f, "%d ", lepton_image[i][j] - minval);
        }
        fprintf(f, "\n");
    }
    fprintf(f, "\n\n");

    fclose(f);
    //----------------------------- Lidar reading ------------------------------
    if (lidar.err >= 0) {
        dist = readLidar(&lidar);
        printf("%d\n", dist);
    }
    //----------------------- write data in record file -----------------------
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    //printf("%d,%d,%d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
    fprintf(fcsv, "%d,%d,%d,%d,%u,%u,%d\n", tm.tm_hour, tm.tm_min, tm.tm_sec,
            image_index, maxval, minval, dist);
}

int transfer(int fd) {
    usleep(100);
    int ret;
    int i;
    int frame_number;
    uint8_t tx[VOSPI_FRAME_SIZE] = {0,};

    struct spi_ioc_transfer tr {
        (unsigned long) tx,
        (unsigned long) lepton_frame_packet,
        VOSPI_FRAME_SIZE,
        speed,
        delay,
        bits
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
        pabort("can't send spi message");

    if (((lepton_frame_packet[0]&0xf) != 0x0f)) {
        frame_number = lepton_frame_packet[1];

        if (frame_number < 60) {
            for (i = 0; i < 80; i++) {
                lepton_image[frame_number][i] = (lepton_frame_packet[2 * i + 4] << 8 | lepton_frame_packet[2 * i + 5]);
            }
        }
    }
    return frame_number;
}

int capture() {
    int ret = 0;
    int fd;


    fd = open(device, O_RDWR);
    if (fd < 0) {
        pabort("can't open device");
    }

    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1) {
        pabort("can't set spi mode");
    }

    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1) {
        pabort("can't get spi mode");
    }

    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1) {
        pabort("can't set bits per word");
    }

    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1) {
        pabort("can't get bits per word");
    }

    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1) {
        pabort("can't set max speed hz");
    }

    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1) {
        pabort("can't get max speed hz");
    }

    //printf("spi mode: %d\n", mode);
    //printf("bits per word: %d\n", bits);
    //printf("max speed: %d Hz (%d KHz)\n", speed, speed / 1000);


    while (transfer(fd) != 59);

    close(fd);

    save_pgm_file();

    return ret;
}

void init() {

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    //--------------------- make new flight directory --------------------------
    struct stat st = {0};
    int flight_index = 0;
    do {
        flight_index += 1;
        sprintf(dir_name, "Flight_%d-%d-%d_%.2d", tm.tm_year + 1900,
                tm.tm_mon + 1, tm.tm_mday, flight_index);

    } while (stat(dir_name, &st) == 0);
    mkdir(dir_name, 0700);
    //------------------------- make new record file ---------------------------
    sprintf(record_name, "%s/Record.csv", dir_name);
    fcsv = fopen(record_name, "a");
    //--------------------- Write header for record file  ----------------------
    fprintf(fcsv, "Year,Month,Day\n");
    fprintf(fcsv, "%d,%d,%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    fprintf(fcsv, "Hour,Min,Sec,Index,Max Value,Min Value,Distance(cm)\n");
}

void closeFiles() {
    fclose(fcsv);
}

int readLidar(Lidar_Lite* Lidar) {
    int dist = 0, sum = 0, count = 0;
    for (int i = 0; i < 10; i++) {
        //dist = Lidar->getDistance();
        dist = Lidar->getDistanceFreeRun();
        sum += dist;
        count++;
    }
    return dist;
}

#endif /* CAPTURE_H */

