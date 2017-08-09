
#ifndef SAMPLINGTIME_H
#define SAMPLINGTIME_H

#include <cstdio>       // printf
#include <iostream>     // localtime
#include <sys/time.h>   // gettimeofday
#include <unistd.h>     // usleep

class SamplingTime {
public:
    SamplingTime(float Freq);
    SamplingTime(const SamplingTime& orig);
    virtual ~SamplingTime();
    float ts, freq;
    long ptime, ctime;
    float tsCalculat(void);
    long calTime(void);
    void setFreq(float Freq);
    float getTS(void) const;  
private:
    time_t t;
    struct timeval tval;
};

#endif /* SAMPLINGTIME_H */