#include "main.h"

//ThermalCamera TC;

void ctrC(int s) {
    printf("\nClose program\n");
    closeFiles();
    exit(1);
}

int main(int argc, char** argv) {
    printf("Start program with %d arguments\n",argc); // Welcome msg
    int imax = 1000000;
    if (argc > 1)
        imax = atoi(argv[1]);
    signal(SIGINT, &ctrC);

    int i = 0;
    int err = lidar.connect();
    if (err < 0)
        printf("%d\n", 11);
    else
        lidar.freeRun();

    dataStruct data;
    init();
    data.isThreadon = false;
    //------------------------------- Main Loop --------------------------------
    long time0, time;
    float dt;
    while (1) {
        i++;        
        time0 = calTime(); // Calculate current time
        data.isThreadon = true;
        pthread_create(&_thermalThread, NULL, *thermalThread, (void *) &data);
        while (data.isThreadon) {
            time = calTime(); // Calculate current time
            dt = (time - time0) / 1000000.0;
            if (dt > 0.24) {
                printf("too much time\n");
                data.isThreadon = false;
                pthread_cancel(_thermalThread);
                break;
            } else {
                usleep(50);
            }
        }
        usleep((0.25 - dt) * 1000000);
        printf("i = %d    dt:%+03.3f   def:%+03.3f\n", i, dt , (0.25 - dt));
        if (i == imax)
            break;
    }
    ctrC(0);
    return 0;
}
