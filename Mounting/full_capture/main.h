#ifndef MAIN_H
#define MAIN_H

#include <cstdio>   // printf
#include "SamplingTime.h"         // Sampling Time
#include <signal.h>
#include "capture.h"
#include <pthread.h>

#include "lidar_lite.h"

pthread_t _thermalThread;

// Define structures:
struct dataStruct {
    bool isThreadon;
};

void *thermalThread(void *data) {
    // Define and connect data through threads
    struct dataStruct *my_data;
    my_data = (struct dataStruct *) data;
    my_data->isThreadon = true;
    capture();
    my_data->isThreadon = false;
    pthread_exit(NULL);
}

long calTime(){
    struct timeval tval;
    gettimeofday(&tval, NULL);
    return 1000000 * tval.tv_sec + tval.tv_usec;  // return current time in nsec
}
#endif /* MAIN_H */
