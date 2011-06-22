#ifndef XVTRENTRY_H
#define XVTRENTRY_H

#include <QString>

class XvtrEntry
{
public:
    XvtrEntry();
    XvtrEntry(QString t,long long minF,long long maxF,long long ifF,long long freq,int m,int filt);
    QString getTitle();
    long long getMinFrequency();
    long long getMaxFrequency();
    long long getIFFrequency();
    long long getFrequency();
    int getMode();
    int getFilter();
    void setFrequency(long long f);
    void setMode(int m);
    void setFilter(int f);

private:
    QString title;
    long long minFrequency;
    long long maxFrequency;
    long long ifFrequency;

    long long frequency;
    int mode;
    int filter;
};

#endif // XVTRENTRY_H
