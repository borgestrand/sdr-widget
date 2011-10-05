#ifndef VFO_H
#define VFO_H

#include <QFrame>
#include <QWheelEvent>
#include <QBasicTimer>
#include <QSettings>

namespace Ui {
    class vfo;
}

class vfo : public QFrame
{
    Q_OBJECT

public:
    explicit vfo(QWidget *parent = 0);
    ~vfo();

    void readSettings(QSettings* settings);
    void writeSettings(QSettings* settings);
    void checkSubRx(long long subRxFrequency);
    void uncheckSubRx();
    void setFrequency(int freq); //Displays "freq" on current vfo according to ptt state
    void setSubRxFrequency(long long subRxFrequency);
    QString rigctlGetvfo();
//    }

public slots:
    void togglePTT(bool pttRq); //Request to toggle ptt & return freq
//    void setFrequency(int freq); //Displays "freq" on current vfo according to ptt state
    void on_pBtnvfoA_clicked();  // moved from private for rigctl
    void on_pBtnvfoB_clicked();
    void checkBandBtn(int band);

signals:
    void sendVfoFreq(int freq);
    void sendTxFreq(int freq, bool ptt);
    void setFreq(int freq, bool ptt);
    void frequencyChanged(long long freq);
    void frequencyMoved(int, int);
    void bandBtnClicked(int band);
    void rightBandClick();
    void subRxButtonClicked();

protected:
    void wheelEvent( QWheelEvent*);
    void mousePressEvent( QMouseEvent*);
//    void closeEvent(QCloseEvent *);

private slots:
    void btnGrpClicked(int);
    void on_pBtnScanDn_clicked();
    void on_pBtnScanUp_clicked();
    void on_pBtnSplit_clicked();
    void on_pBtnExch_clicked();
    void on_pBtnAtoB_clicked();
    void on_pBtnBtoA_clicked();
    void processRIT(int);

    void on_pBtnRIT_clicked();

    void on_pBtnSubRx_clicked();

private:
    Ui::vfo *ui;

    QSettings *settings;
    QBasicTimer timer;
    void writeA(int);
    void writeB(int);
    void vfoEnabled(bool setA, bool setB);
    void storeVFO();
    void setBandButton(int freq);
    void timerEvent(QTimerEvent *event);
    int **bands;
    int readA();
    int readB();
    int getDigit(int x, int y);
    int cur_Band;
    int browsePtr; // Memory browsing pointer
    bool ptt; // ptt on = true, ptt off = false
    char selectedVFO; //'A', 'B', 'S' to indicate which vfo state.
    long long spectrumFrequency;
    long long subRxFrequency;
    enum BandData
        {
            bDat_mem00,
            bDat_mem01,
            bDat_mem02,
            bDat_mem03,
            bDat_cFreq,
            bDat_fqmin,
            bDat_fqmax,
            bDat_modee,
            bDat_filtH,
            bDat_filtL,
            bDat_index
        };
};

#endif // VFO_H
