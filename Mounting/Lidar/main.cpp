#include <cstdlib>
#include "lidar_lite.h"
#include <cstdio>
#include <unistd.h>

using namespace std;

int readLidar(Lidar_Lite* Lidar);

int main(int argc, char** argv) {
    Lidar_Lite l1(1);
    int err = l1.connect();
    if (err < 0) {
        printf("%d\n", l1.err);
    } else {
        l1.freeRun();

        while (l1.err >= 0) {

            int dist = readLidar(&l1);
            printf("%d\n", dist);
            usleep(1000);
        }
    }
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
    // }
}
