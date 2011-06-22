#include "XvtrEntry.h"

XvtrEntry::XvtrEntry() {
}

XvtrEntry::XvtrEntry(QString t,long long minF,long long maxF,long long ifF,long long freq,int m,int filt) {
    title=t;
    minFrequency=minF;
    maxFrequency=maxF;
    ifFrequency=ifF;
    frequency=freq;
    mode=m;
    filter=filt;
}

QString XvtrEntry::getTitle() {
    return title;
}

long long XvtrEntry::getMinFrequency() {
    return minFrequency;
}

long long XvtrEntry::getMaxFrequency() {
    return maxFrequency;
}

long long XvtrEntry::getIFFrequency() {
    return ifFrequency;
}

long long XvtrEntry::getFrequency() {
    return frequency;
}

int XvtrEntry::getMode() {
    return mode;
}

int XvtrEntry::getFilter() {
    return filter;
}

void XvtrEntry::setFrequency(long long f) {
    frequency=f;
}

void XvtrEntry::setMode(int m) {
    mode=m;
}

void XvtrEntry::setFilter(int f) {
    filter=f;
}
