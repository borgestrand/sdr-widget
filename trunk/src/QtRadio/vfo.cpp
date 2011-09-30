#include "vfo.h"
#include "ui_vfo.h"
#include "UI.h"
#include <QDebug>
#include "UI.h"

vfo::vfo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::vfo)
{
    ui->setupUi(this);

    cur_Band = 20; // Initialise to a value which is != to any band.
    browsePtr = 0;
    selectedVFO = 'A';
    ptt = false;

    bands = new int*[12];  // Create 12 rows (there are 12 buttons)
    for (int nCount=0; nCount < 12; nCount++)
        bands[nCount] = new int[bDat_index + 1];  // Create 8 columns

    for (int row = 0; row < 12; row ++) {
        for (int col = 0; col == bDat_index; col++) {
            bands[row][col] = 10000000;
        }
        bands[row][bDat_index] = 0; // Overwrite the index to zero for each row
    }
//    readSettings();
//gvj    setBandButton(readA());
    ui->btnGrpBand->addButton(ui->bandBtn_12);
    connect(ui->btnGrpBand, SIGNAL(buttonClicked(int)),
                this, SLOT(btnGrpClicked(int)));
    connect(ui->hSlider, SIGNAL(valueChanged(int)),
                this, SLOT(processRIT(int)));
//    connect(ui->pBtnSubRx, SIGNAL(toggled(bool)),
//            this, SLOT(togglePTT(bool)));
}

vfo::~vfo()
{
    delete ui;
}

void vfo::setFrequency(int freq)
{
    spectrumFrequency = freq;
    if (selectedVFO == 'A') {
        writeA(freq);
    } else if (selectedVFO == 'B') {
        if (!ui->pBtnSubRx->isChecked()) writeB(freq);
    } else writeA(freq);
}


void vfo::togglePTT(bool pttRq)
{
    int freq;
    bool vfoUsed; // true = vfoA, false = vfoB.

    if (selectedVFO == 'A') {
        freq = readA();
        vfoUsed = true;
    } else if (selectedVFO == 'B') {
        freq = readB();
        vfoUsed = false;
    } else if (pttRq == true) { // Must be split and
        freq = readB();         //we will Tx on vfoB
        vfoUsed = false;
    } else {                // We are receiving so we
        freq = readA();     // will Rx on vfoA.
        vfoUsed = true;
    }

    if (spectrumFrequency != freq)  emit frequencyChanged((long long) freq);

    // We have decided on vfo to use and got basic freq. Lets now see if we
       // are doing a valid changeover and if it is Rx to Tx or Tx to Rx.
    if (pttRq == true && ptt == false) { // Going from Rx to Tx
        ptt = true;
        if (ui->pBtnRIT->isChecked()) {
            if (selectedVFO == 'A' || selectedVFO == 'B'){
                freq = freq + ui->hSlider->value() * -1;
                ui->pBtnRIT->setEnabled(false);
                ui->hSlider->setEnabled(false);
            } // We don't modify the vfo frequencies if on split
        }     // and of course no modification if RIT not checked.
        if (vfoUsed == true) { // Using vfoA for transmit frequency
            writeA(freq);
            ui->pBtnvfoA->setStyleSheet("background-color: rgb(255, 0, 0)"); //Red
        } else {
            writeB(freq);
            //TODO set vfoA background to light green and disable display vfoEnabled(false, true)
            vfoEnabled(false, true);
            ui->pBtnvfoA->setStyleSheet("background-color: normal"); //Red
            ui->pBtnvfoB->setStyleSheet("background-color: rgb(255, 0, 0)"); //Red
        }
    } else if (pttRq == false && ptt == true) { //Going from Tx to Rx
        ptt = false;
        if (ui->pBtnRIT->isChecked()) {
            if (selectedVFO == 'A' || selectedVFO == 'B'){
                freq = freq + ui->hSlider->value();
                ui->pBtnRIT->setEnabled(true);
                ui->hSlider->setEnabled(true);
            } // We don't modify the vfo frequencies if on split
        }     // and again no modification if RIT not checked.
        if (selectedVFO == 'A') { // Using vfoA for receive frequency
            writeA(freq);
            ui->pBtnvfoA->setStyleSheet("background-color: rgb(85, 255, 0)"); //Green
        } else  if (selectedVFO == 'B'){
            writeB(freq);
            ui->pBtnvfoB->setStyleSheet("background-color: rgb(85, 255, 0)"); //Green
        } else {
            writeA(freq);
            ui->pBtnvfoB->setStyleSheet("background-color: rgb(255, 155, 155)"); //Light Red
            ui->pBtnvfoA->setStyleSheet("background-color: rgb(85, 255, 0)"); //Green
            vfoEnabled(true, false);
        }
    }
}

void vfo::processRIT(int rit)
{
    static int freq = 0;

    freq = rit - freq; // freq now holds difference between last rit and this.
    if (selectedVFO != 'B') { // Using vfoA or Split if 'B' is not selectedVFO.
        freq += readA();
        if (ui->pBtnRIT->isChecked()) writeA(freq);
    } else {
        freq += readB();
        if (ui->pBtnRIT->isChecked()) writeB(freq);
    }
    freq = rit;
}

void vfo::on_pBtnRIT_clicked()
{
    int chkd = -1;

    if (ui->pBtnRIT->isChecked()) {
        chkd = 1;
    }
    if (selectedVFO != 'B') { // Using vfoA or Split if 'B' is not selectedVFO.
        writeA(readA() + ui->hSlider->value() * chkd);
    } else {
        writeB(readB() + ui->hSlider->value() * chkd);
    }
}

void vfo::btnGrpClicked(int btn)
{
    btn = -1 * (btn + 2); //Map buttons (-2 .. -15) to (0 .. 12)
    emit bandBtnClicked(btn);
}

// We arrive here from a mousePressEvent detecting aright button click on the bandButtons area the variable
// cur_Band will hold the index to the required band button
void vfo::storeVFO()
{
    int retrievedFreq;
    int cnt;

    qDebug() << "From storeVFO(), band button = " << cur_Band;

    if (selectedVFO != 'B') {
        retrievedFreq = readA(); //Using vfoA
    } else {
        retrievedFreq = readB();
    }
    // Search to see if the freq is already stored
    for (cnt = 0; cnt < 4; cnt++) {       //If the freq is in one of the memories then set
        if (retrievedFreq == bands[cur_Band][cnt]) break;  // the browsePtr to point to it.
    }
    if (cnt != 4) { //cnt will be 4 if no matching memory freq.
        browsePtr = cnt; // Points to matching memory position & we don't need to store it.
    }   else {
        bands[cur_Band][bDat_index]--;
        bands[cur_Band][bDat_index] &= 0x03;
        bands[cur_Band][bands[cur_Band][bDat_index]] = retrievedFreq;
        browsePtr = bands[cur_Band][bDat_index]; //Point to last stored freq.
    }
    timer.start(500,this);
    ui->btnGrpBand->checkedButton()->setStyleSheet("background-color: yellow");
}

void vfo::timerEvent(QTimerEvent *event)
 {
    if (event->timerId() == timer.timerId()) {
        timer.stop();
        ui->btnGrpBand->checkedButton()->setStyleSheet("background-color: normal");
    } else {
        QWidget::timerEvent(event);
    }
}

void vfo::mousePressEvent(QMouseEvent *event)
{
    bool isVFOa = false;
    int digit, cnt;
    QString myStr = "";

    if (event->button() == Qt::RightButton) {

// qDebug() << (QString::number(event->x()) + "/" + QString::number(event->y()));
        //Check to see if we have right clicked on the band button group
        if ((event->x() > 414) && (event->x() < 573) && (event->y() > 6) && (event->y() < 111)) {
            qDebug() << "Band buttons have been rightClicked";
            emit rightBandClick();
        } else if ((event->x() > 189) && (event->x() < 403) && (event->y() > 89) && (event->y() < 111)) {
                ui->hSlider->setValue(0); // Check to see if we have right clicked the RIT slider
        } else { // We have clicked either on the display or somewhere else on the widget
            digit = getDigit(event->x(), event->y());
            if (digit != 9) {  // getDigit returns 9 if click was outside display area.
                if (digit < 9) { // getDigit returns 0 ... 8 if we clicked on vfoA
                    isVFOa = true;
                    myStr = ui->lbl_Amhz->text() + ui->lbl_Akhz->text() + ui->lbl_Ahz->text();
                } else {                  // getDigit returns 10 ... 18 if we clicked on vfoB
                    digit = digit - 10; // so convert to 1 ... 8.
                    myStr = ui->lbl_Bmhz->text() + ui->lbl_Bkhz->text() + ui->lbl_Bhz->text();
                }
                for (cnt = myStr.length(); cnt < 9; cnt++) {
                    myStr = "0" + myStr;
                }
                for (cnt = digit; cnt < 9; cnt++) {
                    myStr[cnt] = QChar('0');
                    ui->hSlider->setValue(0);
                }
                if (isVFOa) {
                    writeA(myStr.toInt());
                } else writeB(myStr.toInt());
            }
        }
    }
}

int vfo::getDigit(int x, int y)
{
    static const int idx[] = {64,87,107,137,159,178,207,226,241,257}; //These are the LHS of each digit of the frequency.
    int digit;

    if ((y < 12 || y > 84) || (x <64 || x > 257)) {
            digit = 9; //digit will = 9 if outside x or y dial display range.
    }
    else {
        for (digit = 0; idx[digit] != 257; digit++) {  // we exit this loop with digit at the wheeled digit.
            if ((x >= idx[digit])  && (x <= idx[digit + 1])) break;
        } // digit will be 0 ... 9 if inside x range or 9 if under or over range.

     //   if (y < 12 || y > 84) digit = 9; //digit will = 9 if outside x or y dial display range.
        if (y > 48 && y < 85) digit = digit + 10; //10 to 18 signifies vfoB
    }
    return digit; //0 ... 8 = vfoA; 9 = mouse pointer is outside x or y range; 10 ... 18 = vfoB.
}

void vfo::wheelEvent(QWheelEvent *event)
{
    QString str;
    int x, digit;
    int direction = 1;
    static const int mult[2][9] = {
        {100000000,10000000,1000000,100000,10000,1000,100,10,1},            // Retrieve with mult[0][0 ... 8]
        {-100000000,-10000000,-1000000,-100000,-10000,-1000,-100,-10,-1}    // Retrieve with mult[1][0 ... 8]
    };

    digit = getDigit(event->x(), event->y());
    if (digit != 9) {  // getDigit returns 9 if click was outside display area so we just fall through.
        if (event->delta() < 0) direction = 0;  // x becomes pos or neg depending on wheel rotation.
        if (digit < 9) { // getDigit returns 0 ... 8 if we clicked on vfoA
            x = mult[direction][digit];
//            x = x + readA();
//            if (x >= 0 && x <= 999999999)  // Safe to update the display without overflow or underflow.
//                writeA(x);
        } else {                  // getDigit returns 10 ... 18 if we clicked on vfoB
            digit = digit - 10; // so convert to 1 ... 8.
            x = mult[direction][digit];
//            x = x + readB();
//            if (x > 0 && x <= 999999999)  // Safe to update the display without overflow or underflow.
//                writeB(x);
        }
        emit frequencyMoved(x, 1);
    }  //We fall through to here without processing the wheel if getDigit returns 9.
}

void vfo::writeA(int freq)
{
    QString myStr;
    int cnt = 0;
    int stgChrs;

    myStr.setNum(freq);
    stgChrs = myStr.size() -1;
    ui->lbl_Ahz->setText("");  // Clear the screen for VFO A
    ui->lbl_Akhz->setText("");
    ui->lbl_Amhz->setText("");
    for (cnt = stgChrs; cnt > -1; cnt--) {
        if (stgChrs - cnt < 3) ui->lbl_Ahz->setText(myStr.at(cnt)+ui->lbl_Ahz->text());
        else if (stgChrs - cnt < 6) ui->lbl_Akhz->setText(myStr.at(cnt)+ui->lbl_Akhz->text());
        else ui->lbl_Amhz->setText(myStr.at(cnt)+ui->lbl_Amhz->text());
    }
    if (selectedVFO != 'B') {  // i.e. selectedVFO is 'A' or 'S'
//gvj        setBandButton(freq);
    }
    if (ptt) {
        if (selectedVFO == 'A') {
            emit setFreq(freq, ptt);
            if (spectrumFrequency != freq)  emit frequencyChanged((long long) freq);

qDebug() << "1. Using vfoA, freq = " << freq << ", ptt = " << ptt << ", readA = " << readA();
        }
    } else if (selectedVFO != 'B') {
        emit setFreq(freq, ptt);
        if (spectrumFrequency != freq)  emit frequencyChanged((long long) freq);
qDebug() << "2. Using vfoA, freq = " << freq << ", ptt = " << ptt << ", readA = " << readA();
    }
}

void vfo::writeB(int freq)
{
    QString myStr;
    int cnt = 0;
    int stgChrs;

    myStr.setNum(freq);
    stgChrs = myStr.size() -1;
    ui->lbl_Bhz->setText("");  // Clear the screen for VFO B
    ui->lbl_Bkhz->setText("");
    ui->lbl_Bmhz->setText("");
    for (cnt = stgChrs; cnt > -1; cnt--)
    {
        if (stgChrs - cnt < 3) ui->lbl_Bhz->setText(myStr.at(cnt)+ui->lbl_Bhz->text());
        else if (stgChrs - cnt < 6) ui->lbl_Bkhz->setText(myStr.at(cnt)+ui->lbl_Bkhz->text());
        else ui->lbl_Bmhz->setText(myStr.at(cnt)+ui->lbl_Bmhz->text());
    }
    if (selectedVFO == 'B') {
//gvj        setBandButton(freq);
    }
    if (ptt) {
        if (selectedVFO != 'A') {
            emit setFreq(freq, ptt);
            if (spectrumFrequency != freq)  emit frequencyChanged((long long) freq);
qDebug() << "Using vfoB, freq = " << freq << ", ptt = " << ptt;
        }
    } else if (selectedVFO == 'B') {
        if (!ui->pBtnSubRx->isChecked()) {
            emit setFreq(freq, ptt);
            if (spectrumFrequency != freq)  emit frequencyChanged((long long) freq);
        }
qDebug() << "Using vfoB, freq = " << freq << ", ptt = " << ptt;
    }
}

void vfo::checkBandBtn(int band)
{
    ui->btnGrpBand->button((band+2)*-1)->setChecked(true);
}

void vfo::setBandButton(int freq)
{
    int cnt;

    for (cnt = 0; cnt < 12; cnt++) { // 12 buttons (0 ... 11)
        if (bands[cnt][bDat_fqmin] <= freq && bands[cnt][bDat_fqmax] >= freq) {
            cur_Band = cnt;
            ui->btnGrpBand->button((cnt+2)*-1)->setChecked(true);
            bands[cnt][bDat_cFreq] = freq;
            break;
        }
    }
}

int vfo::readA()
{
    QString myStr;

    myStr = (ui->lbl_Amhz->text() + ui->lbl_Akhz->text() + ui->lbl_Ahz->text());
    return myStr.toInt();
}

int vfo::readB()
{
    QString myStr;

    myStr = ui->lbl_Bmhz->text() + ui->lbl_Bkhz->text() + ui->lbl_Bhz->text();
    return myStr.toInt();
}

void vfo::on_pBtnvfoA_clicked()
{
    if (selectedVFO != 'A') {
        if (ui->pBtnRIT->isChecked()) {
            ui->pBtnRIT->setChecked(false);
            on_pBtnRIT_clicked();
        }
        selectedVFO = 'A';
        ui->pBtnvfoA->setStyleSheet("background-color: rgb(85, 255, 0)"); //Set button to Green
        ui->pBtnvfoB->setStyleSheet("background-color: normal");
        ui->pBtnSplit->setStyleSheet("background-color: normal");
        vfoEnabled(true, false);
//gvj        setBandButton(readA());
        writeA(readA());
    }
}

void vfo::on_pBtnvfoB_clicked()
{
    if (selectedVFO != 'B') {
        if (ui->pBtnRIT->isChecked()) {
            ui->pBtnRIT->setChecked(false);
            on_pBtnRIT_clicked();
        }
        selectedVFO = 'B';
        ui->pBtnvfoB->setStyleSheet("background-color: rgb(85, 255, 0)");
        ui->pBtnvfoA->setStyleSheet("background-color: normal");
        ui->pBtnSplit->setStyleSheet("background-color: normal");
        vfoEnabled(false, true);
//gvj        setBandButton(readB());
        writeB(readB());
    }
}

void vfo::on_pBtnSplit_clicked()
{
    if (selectedVFO != 'S') {
        if (ui->pBtnRIT->isChecked()) {
            ui->pBtnRIT->setChecked(false);
            on_pBtnRIT_clicked();
        }
        selectedVFO = 'S';
        ui->pBtnvfoA->setStyleSheet("background-color: rgb(85, 255, 0)");
        ui->pBtnvfoB->setStyleSheet("background-color: rgb(255, 155, 155)");
        ui->pBtnSplit->setStyleSheet("background-color: rgb(0, 170, 255)");
        vfoEnabled(true, false);
//gvj        setBandButton(readA());
        if (ptt == true) {
            writeB(readB());
        } else {
            writeA(readA());
        }
    }
}

void vfo::on_pBtnScanDn_clicked()
{
    ui->pBtnScanUp->setChecked(false);
}

void vfo::on_pBtnScanUp_clicked()
{
    ui->pBtnScanDn->setChecked(false);
}

void vfo::on_pBtnExch_clicked()
{
    int exchTemp;

    exchTemp = readA();
    writeA(readB());
    writeB(exchTemp);
}

void vfo::on_pBtnAtoB_clicked()
{
    writeB(readA());
}

void vfo::on_pBtnBtoA_clicked()
{
    writeA(readB());
}


void vfo::vfoEnabled(bool setA, bool setB)
// Set the screen info for the vfo in use to enabled.
{
    ui->lbl_Ahz->setEnabled(setA); //These are the labels used
    ui->lbl_Akhz->setEnabled(setA);//to draw the frequency display.
    ui->lbl_Amhz->setEnabled(setA);
    ui->label->setEnabled(setA);   //The MHz decimal
    ui->label_2->setEnabled(setA); //The Khz comma
    ui->lbl_Bhz->setEnabled(setB);
    ui->lbl_Bkhz->setEnabled(setB);
    ui->lbl_Bmhz->setEnabled(setB);
    ui->label_3->setEnabled(setB);
    ui->label_4->setEnabled(setB);
}

void vfo::readSettings(QSettings* settings)
{
    settings->beginGroup("vfo");
/*
    bands[0][bDat_mem00] = (settings->value("Band0_Mem00",1850000).toInt());
    bands[0][bDat_mem01] = (settings->value("Band0_Mem01",1860000).toInt());
    bands[0][bDat_mem02] = (settings->value("Band0_Mem02",1870000).toInt());
    bands[0][bDat_mem03] = (settings->value("Band0_Mem03",1880000).toInt());
    bands[0][bDat_cFreq] = (settings->value("Band0_Cfreq",1840000).toInt());
    bands[0][bDat_fqmin] = (settings->value("Band0_Fqmin",1800000).toInt());
    bands[0][bDat_fqmax] = (settings->value("Band0_Fqmax",1950000).toInt());
    bands[0][bDat_modee] = (settings->value("Band0_Modee",1).toInt());
    bands[0][bDat_filtH] = (settings->value("Band0_FiltH",2800).toInt());
    bands[0][bDat_filtL] = (settings->value("Band0_FiltL",200).toInt());
    bands[0][bDat_index] = (settings->value("Band0_Index",0).toInt());

    bands[1][bDat_mem00] = (settings->value("Band1_Mem00",3550000).toInt());
    bands[1][bDat_mem01] = (settings->value("Band1_Mem01",3560000).toInt());
    bands[1][bDat_mem02] = (settings->value("Band1_Mem02",3570000).toInt());
    bands[1][bDat_mem03] = (settings->value("Band1_Mem03",3580000).toInt());
    bands[1][bDat_cFreq] = (settings->value("Band1_Cfreq",3540000).toInt());
    bands[1][bDat_fqmin] = (settings->value("Band1_Fqmin",3500000).toInt());
    bands[1][bDat_fqmax] = (settings->value("Band1_Fqmax",3900000).toInt());
    bands[1][bDat_modee] = (settings->value("Band1_Modee",1).toInt());
    bands[1][bDat_filtH] = (settings->value("Band1_FiltH",2800).toInt());
    bands[1][bDat_filtL] = (settings->value("Band1_FiltL",200).toInt());
    bands[1][bDat_index] = (settings->value("Band1_Index",0).toInt());

    bands[2][bDat_mem00] = (settings->value("Band2_Mem00",7050000).toInt());
    bands[2][bDat_mem01] = (settings->value("Band2_Mem01",7060000).toInt());
    bands[2][bDat_mem02] = (settings->value("Band2_Mem02",7070000).toInt());
    bands[2][bDat_mem03] = (settings->value("Band2_Mem03",7080000).toInt());
    bands[2][bDat_cFreq] = (settings->value("Band2_Cfreq",7040000).toInt());
    bands[2][bDat_fqmin] = (settings->value("Band2_Fqmin",7000000).toInt());
    bands[2][bDat_fqmax] = (settings->value("Band2_Fqmax",7300000).toInt());
    bands[2][bDat_modee] = (settings->value("Band2_Modee",1).toInt());
    bands[2][bDat_filtH] = (settings->value("Band2_FiltH",2800).toInt());
    bands[2][bDat_filtL] = (settings->value("Band2_FiltL",200).toInt());
    bands[2][bDat_index] = (settings->value("Band2_Index",0).toInt());

    bands[3][bDat_mem00] = (settings->value("Band3_Mem00",10110000).toInt());
    bands[3][bDat_mem01] = (settings->value("Band3_Mem01",10120000).toInt());
    bands[3][bDat_mem02] = (settings->value("Band3_Mem02",10130000).toInt());
    bands[3][bDat_mem03] = (settings->value("Band3_Mem03",10140000).toInt());
    bands[3][bDat_cFreq] = (settings->value("Band3_Cfreq",10125000).toInt());
    bands[3][bDat_fqmin] = (settings->value("Band3_Fqmin",10100000).toInt());
    bands[3][bDat_fqmax] = (settings->value("Band3_Fqmax",10150000).toInt());
    bands[3][bDat_modee] = (settings->value("Band3_Modee",1).toInt());
    bands[3][bDat_filtH] = (settings->value("Band3_FiltH",2800).toInt());
    bands[3][bDat_filtL] = (settings->value("Band3_FiltL",200).toInt());
    bands[3][bDat_index] = (settings->value("Band3_Index",0).toInt());

    bands[4][bDat_mem00] = (settings->value("Band4_Mem00",14020000).toInt());
    bands[4][bDat_mem01] = (settings->value("Band4_Mem01",14120000).toInt());
    bands[4][bDat_mem02] = (settings->value("Band4_Mem02",14130000).toInt());
    bands[4][bDat_mem03] = (settings->value("Band4_Mem03",14140000).toInt());
    bands[4][bDat_cFreq] = (settings->value("Band4_Cfreq",14125000).toInt());
    bands[4][bDat_fqmin] = (settings->value("Band4_Fqmin",14000000).toInt());
    bands[4][bDat_fqmax] = (settings->value("Band4_Fqmax",14350000).toInt());
    bands[4][bDat_modee] = (settings->value("Band4_Modee",1).toInt());
    bands[4][bDat_filtH] = (settings->value("Band4_FiltH",2800).toInt());
    bands[4][bDat_filtL] = (settings->value("Band4_FiltL",200).toInt());
    bands[4][bDat_index] = (settings->value("Band4_Index",0).toInt());

    bands[5][bDat_mem00] = (settings->value("Band5_Mem00",18010000).toInt());
    bands[5][bDat_mem01] = (settings->value("Band5_Mem01",18020000).toInt());
    bands[5][bDat_mem02] = (settings->value("Band5_Mem02",18030000).toInt());
    bands[5][bDat_mem03] = (settings->value("Band5_Mem03",18040000).toInt());
    bands[5][bDat_cFreq] = (settings->value("Band5_Cfreq",18025000).toInt());
    bands[5][bDat_fqmin] = (settings->value("Band5_Fqmin",18068000).toInt());
    bands[5][bDat_fqmax] = (settings->value("Band5_Fqmax",18168000).toInt());
    bands[5][bDat_modee] = (settings->value("Band5_Modee",1).toInt());
    bands[5][bDat_filtH] = (settings->value("Band5_FiltH",2800).toInt());
    bands[5][bDat_filtL] = (settings->value("Band5_FiltL",200).toInt());
    bands[5][bDat_index] = (settings->value("Band5_Index",0).toInt());

    bands[6][bDat_mem00] = (settings->value("Band6_Mem00",21010000).toInt());
    bands[6][bDat_mem01] = (settings->value("Band6_Mem01",21020000).toInt());
    bands[6][bDat_mem02] = (settings->value("Band6_Mem02",21130000).toInt());
    bands[6][bDat_mem03] = (settings->value("Band6_Mem03",21140000).toInt());
    bands[6][bDat_cFreq] = (settings->value("Band6_Cfreq",21225000).toInt());
    bands[6][bDat_fqmin] = (settings->value("Band6_Fqmin",21000000).toInt());
    bands[6][bDat_fqmax] = (settings->value("Band6_Fqmax",21450000).toInt());
    bands[6][bDat_modee] = (settings->value("Band6_Modee",1).toInt());
    bands[6][bDat_filtH] = (settings->value("Band6_FiltH",2800).toInt());
    bands[6][bDat_filtL] = (settings->value("Band6_FiltL",200).toInt());
    bands[6][bDat_index] = (settings->value("Band6_Index",0).toInt());

    bands[7][bDat_mem00] = (settings->value("Band7_Mem00",24900000).toInt());
    bands[7][bDat_mem01] = (settings->value("Band7_Mem01",24920000).toInt());
    bands[7][bDat_mem02] = (settings->value("Band7_Mem02",24930000).toInt());
    bands[7][bDat_mem03] = (settings->value("Band7_Mem03",24940000).toInt());
    bands[7][bDat_cFreq] = (settings->value("Band7_Cfreq",24925000).toInt());
    bands[7][bDat_fqmin] = (settings->value("Band7_Fqmin",24890000).toInt());
    bands[7][bDat_fqmax] = (settings->value("Band7_Fqmax",24990000).toInt());
    bands[7][bDat_modee] = (settings->value("Band7_Modee",1).toInt());
    bands[7][bDat_filtH] = (settings->value("Band7_FiltH",2800).toInt());
    bands[7][bDat_filtL] = (settings->value("Band7_FiltL",200).toInt());
    bands[7][bDat_index] = (settings->value("Band7_Index",0).toInt());

    bands[8][bDat_mem00] = (settings->value("Band8_Mem00",26955000).toInt());
    bands[8][bDat_mem01] = (settings->value("Band8_Mem01",26960000).toInt());
    bands[8][bDat_mem02] = (settings->value("Band8_Mem02",26970000).toInt());
    bands[8][bDat_mem03] = (settings->value("Band8_Mem03",26980000).toInt());
    bands[8][bDat_cFreq] = (settings->value("Band8_Cfreq",26990000).toInt());
    bands[8][bDat_fqmin] = (settings->value("Band8_Fqmin",26950000).toInt());
    bands[8][bDat_fqmax] = (settings->value("Band8_Fqmax",27300000).toInt());
    bands[8][bDat_modee] = (settings->value("Band8_Modee",1).toInt());
    bands[8][bDat_filtH] = (settings->value("Band8_FiltH",2800).toInt());
    bands[8][bDat_filtL] = (settings->value("Band8_FiltL",200).toInt());
    bands[8][bDat_index] = (settings->value("Band8_Index",0).toInt());

    bands[9][bDat_mem00] = (settings->value("Band9_Mem00",28010000).toInt());
    bands[9][bDat_mem01] = (settings->value("Band9_Mem01",28420000).toInt());
    bands[9][bDat_mem02] = (settings->value("Band9_Mem02",28430000).toInt());
    bands[9][bDat_mem03] = (settings->value("Band9_Mem03",28440000).toInt());
    bands[9][bDat_cFreq] = (settings->value("Band9_Cfreq",28425000).toInt());
    bands[9][bDat_fqmin] = (settings->value("Band9_Fqmin",28000000).toInt());
    bands[9][bDat_fqmax] = (settings->value("Band9_Fqmax",29700000).toInt());
    bands[9][bDat_modee] = (settings->value("Band9_Modee",1).toInt());
    bands[9][bDat_filtH] = (settings->value("Band9_FiltH",2800).toInt());
    bands[9][bDat_filtL] = (settings->value("Band9_FiltL",200).toInt());
    bands[9][bDat_index] = (settings->value("Band9_Index",0).toInt());

    bands[10][bDat_mem00] = (settings->value("Band10_Mem00",51010000).toInt());
    bands[10][bDat_mem01] = (settings->value("Band10_Mem01",51060000).toInt());
    bands[10][bDat_mem02] = (settings->value("Band10_Mem02",51070000).toInt());
    bands[10][bDat_mem03] = (settings->value("Band10_Mem03",51080000).toInt());
    bands[10][bDat_cFreq] = (settings->value("Band10_Cfreq",51940000).toInt());
    bands[10][bDat_fqmin] = (settings->value("Band10_Fqmin",51000000).toInt());
    bands[10][bDat_fqmax] = (settings->value("Band10_Fqmax",53000000).toInt());
    bands[10][bDat_modee] = (settings->value("Band10_Modee",1).toInt());
    bands[10][bDat_filtH] = (settings->value("Band10_FiltH",2800).toInt());
    bands[10][bDat_filtL] = (settings->value("Band10_FiltL",200).toInt());
    bands[10][bDat_index] = (settings->value("Band10_Index",0).toInt());

    bands[11][bDat_mem00] = (settings->value("Band11_Mem00",8050000).toInt());
    bands[11][bDat_mem01] = (settings->value("Band11_Mem01",9060000).toInt());
    bands[11][bDat_mem02] = (settings->value("Band11_Mem02",10070000).toInt());
    bands[11][bDat_mem03] = (settings->value("Band11_Mem03",11080000).toInt());
    bands[11][bDat_cFreq] = (settings->value("Band11_Cfreq",15040000).toInt());
    bands[11][bDat_fqmin] = (settings->value("Band11_Fqmin",000000).toInt());
    bands[11][bDat_fqmax] = (settings->value("Band11_Fqmax",999000000).toInt());
    bands[11][bDat_modee] = (settings->value("Band11_Modee",1).toInt());
    bands[11][bDat_filtH] = (settings->value("Band11_FiltH",2800).toInt());
    bands[11][bDat_filtL] = (settings->value("Band11_FiltL",200).toInt());
    bands[11][bDat_index] = (settings->value("Band11_Index",0).toInt());
*/

//    writeA(settings->value("vfoA_f",3502000).toInt());  // These need to be done after all the frequency
    writeB(settings->value("vfoB_f",14234567).toInt()); // settings have been read and set.
    settings->endGroup();
}

void vfo::writeSettings(QSettings* settings)
{
    settings->beginGroup("vfo");
//    settings->setValue("vfoA_f", readA());
    settings->setValue("vfoB_f", readB());
/*
    settings->setValue("Band0_Mem00",bands[0][bDat_mem00]);
    settings->setValue("Band0_Mem01",bands[0][bDat_mem01]);
    settings->setValue("Band0_Mem02",bands[0][bDat_mem02]);
    settings->setValue("Band0_Mem03",bands[0][bDat_mem03]);
    settings->setValue("Band0_Cfreq",bands[0][bDat_cFreq]);
    settings->setValue("Band0_Fqmin",bands[0][bDat_fqmin]);
    settings->setValue("Band0_Fqmax",bands[0][bDat_fqmax]);
    settings->setValue("Band0_Modee",bands[0][bDat_modee]);
    settings->setValue("Band0_FiltH",bands[0][bDat_filtH]);
    settings->setValue("Band0_FiltL",bands[0][bDat_filtL]);
    settings->setValue("Band0_Index",bands[0][bDat_index]);

    settings->setValue("Band1_Mem00",bands[1][bDat_mem00]);
    settings->setValue("Band1_Mem01",bands[1][bDat_mem01]);
    settings->setValue("Band1_Mem02",bands[1][bDat_mem02]);
    settings->setValue("Band1_Mem03",bands[1][bDat_mem03]);
    settings->setValue("Band1_Cfreq",bands[1][bDat_cFreq]);
    settings->setValue("Band1_Fqmin",bands[1][bDat_fqmin]);
    settings->setValue("Band1_Fqmax",bands[1][bDat_fqmax]);
    settings->setValue("Band1_Modee",bands[1][bDat_modee]);
    settings->setValue("Band1_FiltH",bands[1][bDat_filtH]);
    settings->setValue("Band1_FiltL",bands[1][bDat_filtL]);
    settings->setValue("Band1_Index",bands[1][bDat_index]);

    settings->setValue("Band2_Mem00",bands[2][bDat_mem00]);
    settings->setValue("Band2_Mem01",bands[2][bDat_mem01]);
    settings->setValue("Band2_Mem02",bands[2][bDat_mem02]);
    settings->setValue("Band2_Mem03",bands[2][bDat_mem03]);
    settings->setValue("Band2_Cfreq",bands[2][bDat_cFreq]);
    settings->setValue("Band2_Fqmin",bands[2][bDat_fqmin]);
    settings->setValue("Band2_Fqmax",bands[2][bDat_fqmax]);
    settings->setValue("Band2_Modee",bands[2][bDat_modee]);
    settings->setValue("Band2_FiltH",bands[2][bDat_filtH]);
    settings->setValue("Band2_FiltL",bands[2][bDat_filtL]);
    settings->setValue("Band2_Index",bands[2][bDat_index]);

    settings->setValue("Band3_Mem00",bands[3][bDat_mem00]);
    settings->setValue("Band3_Mem01",bands[3][bDat_mem01]);
    settings->setValue("Band3_Mem02",bands[3][bDat_mem02]);
    settings->setValue("Band3_Mem03",bands[3][bDat_mem03]);
    settings->setValue("Band3_Cfreq",bands[3][bDat_cFreq]);
    settings->setValue("Band3_Fqmin",bands[3][bDat_fqmin]);
    settings->setValue("Band3_Fqmax",bands[3][bDat_fqmax]);
    settings->setValue("Band3_Modee",bands[3][bDat_modee]);
    settings->setValue("Band3_FiltH",bands[3][bDat_filtH]);
    settings->setValue("Band3_FiltL",bands[3][bDat_filtL]);
    settings->setValue("Band3_Index",bands[3][bDat_index]);

    settings->setValue("Band4_Mem00",bands[4][bDat_mem00]);
    settings->setValue("Band4_Mem01",bands[4][bDat_mem01]);
    settings->setValue("Band4_Mem02",bands[4][bDat_mem02]);
    settings->setValue("Band4_Mem03",bands[4][bDat_mem03]);
    settings->setValue("Band4_Cfreq",bands[4][bDat_cFreq]);
    settings->setValue("Band4_Fqmin",bands[4][bDat_fqmin]);
    settings->setValue("Band4_Fqmax",bands[4][bDat_fqmax]);
    settings->setValue("Band4_Modee",bands[4][bDat_modee]);
    settings->setValue("Band4_FiltH",bands[4][bDat_filtH]);
    settings->setValue("Band4_FiltL",bands[4][bDat_filtL]);
    settings->setValue("Band4_Index",bands[4][bDat_index]);

    settings->setValue("Band5_Mem00",bands[5][bDat_mem00]);
    settings->setValue("Band5_Mem01",bands[5][bDat_mem01]);
    settings->setValue("Band5_Mem02",bands[5][bDat_mem02]);
    settings->setValue("Band5_Mem03",bands[5][bDat_mem03]);
    settings->setValue("Band5_Cfreq",bands[5][bDat_cFreq]);
    settings->setValue("Band5_Fqmin",bands[5][bDat_fqmin]);
    settings->setValue("Band5_Fqmax",bands[5][bDat_fqmax]);
    settings->setValue("Band5_Modee",bands[5][bDat_modee]);
    settings->setValue("Band5_FiltH",bands[5][bDat_filtH]);
    settings->setValue("Band5_FiltL",bands[5][bDat_filtL]);
    settings->setValue("Band5_Index",bands[5][bDat_index]);

    settings->setValue("Band6_Mem00",bands[6][bDat_mem00]);
    settings->setValue("Band6_Mem01",bands[6][bDat_mem01]);
    settings->setValue("Band6_Mem02",bands[6][bDat_mem02]);
    settings->setValue("Band6_Mem03",bands[6][bDat_mem03]);
    settings->setValue("Band6_Cfreq",bands[6][bDat_cFreq]);
    settings->setValue("Band6_Fqmin",bands[6][bDat_fqmin]);
    settings->setValue("Band6_Fqmax",bands[6][bDat_fqmax]);
    settings->setValue("Band6_Modee",bands[6][bDat_modee]);
    settings->setValue("Band6_FiltH",bands[6][bDat_filtH]);
    settings->setValue("Band6_FiltL",bands[6][bDat_filtL]);
    settings->setValue("Band6_Index",bands[6][bDat_index]);

    settings->setValue("Band7_Mem00",bands[7][bDat_mem00]);
    settings->setValue("Band7_Mem01",bands[7][bDat_mem01]);
    settings->setValue("Band7_Mem02",bands[7][bDat_mem02]);
    settings->setValue("Band7_Mem03",bands[7][bDat_mem03]);
    settings->setValue("Band7_Cfreq",bands[7][bDat_cFreq]);
    settings->setValue("Band7_Fqmin",bands[7][bDat_fqmin]);
    settings->setValue("Band7_Fqmax",bands[7][bDat_fqmax]);
    settings->setValue("Band7_Modee",bands[7][bDat_modee]);
    settings->setValue("Band7_FiltH",bands[7][bDat_filtH]);
    settings->setValue("Band7_FiltL",bands[7][bDat_filtL]);
    settings->setValue("Band7_Index",bands[7][bDat_index]);

    settings->setValue("Band8_Mem00",bands[8][bDat_mem00]);
    settings->setValue("Band8_Mem01",bands[8][bDat_mem01]);
    settings->setValue("Band8_Mem02",bands[8][bDat_mem02]);
    settings->setValue("Band8_Mem03",bands[8][bDat_mem03]);
    settings->setValue("Band8_Cfreq",bands[8][bDat_cFreq]);
    settings->setValue("Band8_Fqmin",bands[8][bDat_fqmin]);
    settings->setValue("Band8_Fqmax",bands[8][bDat_fqmax]);
    settings->setValue("Band8_Modee",bands[8][bDat_modee]);
    settings->setValue("Band8_FiltH",bands[8][bDat_filtH]);
    settings->setValue("Band8_FiltL",bands[8][bDat_filtL]);
    settings->setValue("Band8_Index",bands[8][bDat_index]);

    settings->setValue("Band9_Mem00",bands[9][bDat_mem00]);
    settings->setValue("Band9_Mem01",bands[9][bDat_mem01]);
    settings->setValue("Band9_Mem02",bands[9][bDat_mem02]);
    settings->setValue("Band9_Mem03",bands[9][bDat_mem03]);
    settings->setValue("Band9_Cfreq",bands[9][bDat_cFreq]);
    settings->setValue("Band9_Fqmin",bands[9][bDat_fqmin]);
    settings->setValue("Band9_Fqmax",bands[9][bDat_fqmax]);
    settings->setValue("Band9_Modee",bands[9][bDat_modee]);
    settings->setValue("Band9_FiltH",bands[9][bDat_filtH]);
    settings->setValue("Band9_FiltL",bands[9][bDat_filtL]);
    settings->setValue("Band9_Index",bands[9][bDat_index]);

    settings->setValue("Band10_Mem00",bands[10][bDat_mem00]);
    settings->setValue("Band10_Mem01",bands[10][bDat_mem01]);
    settings->setValue("Band10_Mem02",bands[10][bDat_mem02]);
    settings->setValue("Band10_Mem03",bands[10][bDat_mem03]);
    settings->setValue("Band10_Cfreq",bands[10][bDat_cFreq]);
    settings->setValue("Band10_Fqmin",bands[10][bDat_fqmin]);
    settings->setValue("Band10_Fqmax",bands[10][bDat_fqmax]);
    settings->setValue("Band10_Modee",bands[10][bDat_modee]);
    settings->setValue("Band10_FiltH",bands[10][bDat_filtH]);
    settings->setValue("Band10_FiltL",bands[10][bDat_filtL]);
    settings->setValue("Band10_Index",bands[10][bDat_index]);

    settings->setValue("Band11_Mem00",bands[11][bDat_mem00]);
    settings->setValue("Band11_Mem01",bands[11][bDat_mem01]);
    settings->setValue("Band11_Mem02",bands[11][bDat_mem02]);
    settings->setValue("Band11_Mem03",bands[11][bDat_mem03]);
    settings->setValue("Band11_Cfreq",bands[11][bDat_cFreq]);
    settings->setValue("Band11_Fqmin",bands[11][bDat_fqmin]);
    settings->setValue("Band11_Fqmax",bands[11][bDat_fqmax]);
    settings->setValue("Band11_Modee",bands[11][bDat_modee]);
    settings->setValue("Band11_FiltH",bands[11][bDat_filtH]);
    settings->setValue("Band11_FiltL",bands[11][bDat_filtL]);
    settings->setValue("Band11_Index",bands[11][bDat_index]);
*/
    settings->endGroup();
}


void vfo::on_pBtnSubRx_clicked()
{
    if(selectedVFO == 'B') {
        on_pBtnBtoA_clicked();
        on_pBtnvfoA_clicked();
    }
    emit subRxButtonClicked();
}

//Called when subRx is checked in main menu via actionSubRx()
void vfo::checkSubRx(long long f)
{
    subRxFrequency = f;
    writeB(subRxFrequency);
    on_pBtnvfoB_clicked();
    ui->pBtnvfoB->setText("subRx");
    ui->pBtnvfoA->setText("mainRx");
    ui->pBtnvfoA->setEnabled(FALSE);
    ui->pBtnSubRx->setChecked(TRUE);
}

//Called when subRx is checked in main menu via actionSubRx()
void vfo::uncheckSubRx()
{
    ui->pBtnSubRx->setChecked(FALSE);
    ui->pBtnvfoB->setText("VFO B");
    ui->pBtnvfoA->setText("VFO A");
    ui->pBtnvfoA->setEnabled(TRUE);
    on_pBtnvfoA_clicked(); //Return to vfoA = default vfo
}

// Called by UI::frequencyMoved() in response to spectra freq move action.
void vfo::setSubRxFrequency(long long f)
{
    subRxFrequency = f;
    if(ui->pBtnSubRx->isChecked()) {
        writeB(f);
    }
}
