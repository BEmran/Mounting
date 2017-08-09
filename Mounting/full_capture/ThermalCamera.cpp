#include "ThermalCamera.h"

ThermalCamera::ThermalCamera() {
    printf("Initilize Thermal Camera\n");
    initilize();
}

ThermalCamera::ThermalCamera(const ThermalCamera& orig) {
    printf("Thermal Camera2\n");
}

ThermalCamera::~ThermalCamera() {
    close(fd);
    fclose(fcsv);
}

void ThermalCamera::pabort(const char* s) {
    //perror(s);
    printf("%c\n",s);
    //abort();
}

/******************************************************************************
save_pgm_file: save file with pgm format
 ******************************************************************************/
void ThermalCamera::save_pgm_file() {
    while (transfer() != 59);

    int i, j;
    unsigned int maxval = 0, minval = UINT_MAX;
    char image_name[32];
    int image_index = 0;
    struct stat st = {0};
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    //--------------------------- Create image file ----------------------------
    do {
        sprintf(image_name, "%s/IMG_%d.pgm", dir_name, image_index);
        image_index += 1;
    } while (stat(image_name, &st) == 0);
    FILE *f = fopen(image_name, "w");

    //-------------------------- Create record file -----------------------------
    printf("Calculating min/max values for proper scaling...\n");
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
    fprintf(f, "P2\n80 60\n%u\n", maxval - minval);
    for (i = 0; i < 60; i++) {
        for (j = 0; j < 80; j++) {
            fprintf(f, "%d ", lepton_image[i][j] - minval);
        }
        fprintf(f, "\n");
    }
    fprintf(f, "\n\n");
    //----------------------- write data in record file ------------------------
    fprintf(fcsv, "%d,%d,%d,%d,%u,%u\n", tm.tm_hour, tm.tm_min, tm.tm_sec,
            image_index, maxval, minval);
    //------------------------------ Close files -------------------------------
    fclose(f);
}

int ThermalCamera::transfer() {
    usleep(200);
    int ret;
    int i;
    int frame_number;
    static uint8_t bits = 8;
    static uint32_t speed = 16000000;
    static uint16_t delay;
    uint8_t tx[VOSPI_FRAME_SIZE] = {0,};
    tr.tx_buf = (unsigned long) tx;
    tr.rx_buf = (unsigned long) lepton_frame_packet;
    tr.len = VOSPI_FRAME_SIZE;
    tr.delay_usecs = delay;
    tr.speed_hz = speed;
    tr.bits_per_word = bits;
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

int ThermalCamera::initilize(void) {
    int ret = 0;

    static const char *device = "/dev/spidev0.1";
    static uint8_t mode;
    static uint8_t bits = 8;
    static uint32_t speed = 16000000;
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
    printf(". ret=%d\n",ret);
    if (ret == 0) {
        printf("spi mode: %d\n", mode);
        printf("bits per word: %d\n", bits);
        printf("max speed: %d Hz (%d KHz)\n", speed, speed / 1000);

        //----------------------- Get current local time ---------------------------
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        //--------------------- make new flight directory --------------------------
        sprintf(dir_name, "Flight_%d-%d-%d_%d:%d:%d", tm.tm_year + 1900,
                tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        mkdir(dir_name, 0700);
        //------------------------- make new record file ---------------------------
        sprintf(record_name, "%s/Record.csv", dir_name);
        fcsv = fopen(record_name, "a");
        //--------------------- Write header for record file  ----------------------
        fprintf(fcsv, "Year,Month,Day\n");
        fprintf(fcsv, "%d,%d,%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
        fprintf(fcsv, "Hour,Min,Sec,Image Index,Max Value,Min Value\n");
    }
    return ret;
    
}
