#ifndef SAMPLERATE_H
#define SAMPLERATE_H

#include <QGlobalStatic>
#include "sample.h"
#include <QtCore/qmath.h>

#define SAMPLES_50HZ_80_PER_CYCLE   4000
#define SAMPLES_60HZ_80_PER_CYCLE   4800
#define SAMPLES_50HZ_256_PER_CYCLE  12800
#define SAMPLES_60HZ_256_PER_CYCLE  15360                       // 256 samples/cycle * 60 cycles
#define MAX_SAMPLES                 SAMPLES_60HZ_256_PER_CYCLE

class SampleRate
{
public:
    enum NominalFrequency {NominalFreq50Hz = 50, NominalFreq60Hz = 60};
    enum SamplesPerCycle {Samples80 = 80, Samples256 = 256};

    SampleRate();
    SampleRate(NominalFrequency nominalFrequency, SamplesPerCycle samplesPerCycle);

    void setSampleRate(SampleRate::NominalFrequency nominalFrequency, SampleRate::SamplesPerCycle samplesPerCycle);
    quint32 getSamplesPerSecond();
    NominalFrequency getNominalFrequency();
    SamplesPerCycle getSamplesPerCycle();
    bool isKnown();
    qreal getTimestep();
    quint32 getLargestPowerOfTwo() {
        if (this->isKnown()) {
            qreal sampleRate = (qreal) this->getSamplesPerSecond();

            return qPow(2.0, (qreal) qFloor(qLn(sampleRate) / qLn(2.0)));
        }

        return 8;
    }

private:
    bool known;
    NominalFrequency nominalFrequency;
    SamplesPerCycle samplesPerCycle;
};

#endif // SAMPLERATE_H
