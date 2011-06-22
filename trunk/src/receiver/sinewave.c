#include <math.h>
#include "ozy.h"
#include "sinewave.h"

#define PI 3.14159265358979323846

double cwSignal(float* left_buf,float* right_buf, int samples, double phase, double freq) {
    double phase_step = freq/sampleRate*2.0*PI;
    double cosval = cos(phase);
    double sinval = sin(phase);
    double cosdelta = cos(phase_step);
    double sindelta = sin(phase_step);
    int i;

    for(i=0; i<samples; i++ )
    {
        left_buf[i] = (float)sin(phase);
        right_buf[i] = (float)cos(phase);
        phase += phase_step;
    }

    return phase;
}

double sineWave(float* buf, int samples, double phase, double freq) {
    double phase_step = freq/sampleRate*2.0*PI;
    double cosval = cos(phase);
    double sinval = sin(phase);
    double cosdelta = cos(phase_step);
    double sindelta = sin(phase_step);
    double tmpval;
    int i;

    for(i=0; i<samples; i++ )
    {
        tmpval = cosval*cosdelta - sinval*sindelta;
        sinval = cosval*sindelta + sinval*cosdelta;
        cosval = tmpval;
        
        buf[i] = (float)(sinval);
        
        phase += phase_step;
    }

    return phase;
}

void sineWave2Tone(float* buf, int samples, 
                        double phase1, double phase2, 
                        double freq1, double freq2,
                        double* updated_phase1, double* updated_phase2)
{
    double phase_step1 = freq1/sampleRate*2*PI;
    double cosval1 = cos(phase1);
    double sinval1 = sin(phase1);
    double cosdelta1 = cos(phase_step1);
    double sindelta1 = sin(phase_step1);

    double phase_step2 = freq2/sampleRate*2*PI;
    double cosval2 = cos(phase2);
    double sinval2 = sin(phase2);
    double cosdelta2 = cos(phase_step2);
    double sindelta2 = sin(phase_step2);
    double tmpval;

    int i;

    for(i=0; i<samples; i++ ) {
        tmpval = cosval1*cosdelta1 - sinval1*sindelta1;
        sinval1 = cosval1*sindelta1 + sinval1*cosdelta1;
        cosval1 = tmpval;

        tmpval = cosval2*cosdelta2 - sinval2*sindelta2;
        sinval2 = cosval2*sindelta2 + sinval2*cosdelta2;
        cosval2 = tmpval;
                                                
        buf[i] = (float)(sinval1*0.5 + sinval2*0.5);
        
        phase1 += phase_step1;
        phase2 += phase_step2;
    }

    *updated_phase1 = phase1;
    *updated_phase2 = phase2;
}

