/* File:   vfo.cpp
 * Author: Graeme Jury, ZL2APV
 *
 * Created on 21 August 2011, 20:00
 */

/* Copyright (C)
* 2011 - Graeme Jury, ZL2APV
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/
#include "vfo.h"
#include "ui_vfo.h"
#include <QDebug>
#include "Band.h"
#include "UI.h"

vfo::vfo(QWidget *parent) :
    QFrame(parent),
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
//  setBandButton group ID numbers;
    ui->btnGrpBand->setId(ui->bandBtn_00, 0); // 160
    ui->btnGrpBand->setId(ui->bandBtn_01, 1); // 80
    ui->btnGrpBand->setId(ui->bandBtn_02, 2);
    ui->btnGrpBand->setId(ui->bandBtn_03, 3);
    ui->btnGrpBand->setId(ui->bandBtn_04, 4);
    ui->btnGrpBand->setId(ui->bandBtn_05, 5);
    ui->btnGrpBand->setId(ui->bandBtn_06, 6); // etc.
    ui->btnGrpBand->setId(ui->bandBtn_07, 7);
    ui->btnGrpBand->setId(ui->bandBtn_08, 8);
    ui->btnGrpBand->setId(ui->bandBtn_09, 9);
    ui->btnGrpBand->setId(ui->bandBtn_10, 10); // 6
    ui->btnGrpBand->setId(ui->bandBtn_11, 11); // GEN
    ui->btnGrpBand->setId(ui->bandBtn_12, 12); // WWV
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
    emit getBandFrequency();
    if (bandFrequency != freq)  emit frequencyChanged((long long) freq);

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

/*
    freq = rit - freq; // freq now holds difference between last rit and this.
    if (selectedVFO != 'B') { // Using vfoA or Split if 'B' is not selectedVFO.
        freq += readA();
        if (ui->pBtnRIT->isChecked()) writeA(freq);
    } else {
        freq += readB();
        if (ui->pBtnRIT->isChecked()) writeB(freq);
    }
*/
    if (ui->pBtnRIT->isChecked()) {
        emit frequencyMoved(rit - freq, 1);
    }
    freq = rit;
}

void vfo::on_pBtnRIT_clicked()
{
    int chkd = -1;

    if (ui->pBtnRIT->isChecked()) {
        chkd = 1;
    }
/*
    if (selectedVFO != 'B') { // Using vfoA or Split if 'B' is not selectedVFO.
        writeA(readA() + ui->hSlider->value() * chkd);
    } else {
        writeB(readB() + ui->hSlider->value() * chkd);
    }
*/
    emit frequencyMoved(ui->hSlider->value() * chkd, 1);
}

void vfo::btnGrpClicked(int btn)
{
//    btn = -1 * (btn + 2); //Map buttons (-2 .. -15) to (0 .. 12)
    emit bandBtnClicked(btn);
}

// We arrive here from a mousePressEvent detecting aright button click on the bandButtons area the variable
// cur_Band will hold the index to the required band button
void vfo::storeVFO()
{
    int retrievedFreq;
    int cnt;

//    qDebug() << "From storeVFO(), band button = " << cur_Band; //gjfix

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
    emit getBandFrequency();
    if (ptt) {
        if (selectedVFO == 'A') {
            emit setFreq(freq, ptt);
//qDebug()<<Q_FUNC_INFO<<": The value of Band.getFrequency() is ... "<<Band::getFrequency();
            if (bandFrequency != freq)  emit frequencyChanged((long long) freq);

qDebug() << "1. Using vfoA, freq = " << freq << ", ptt = " << ptt << ", readA = " << readA();
        }
    } else if (selectedVFO != 'B') {
        emit setFreq(freq, ptt);
//qDebug()<<Q_FUNC_INFO<<": The value of Band.getFrequency() is ... "<<band.getFrequency();
//        if(band.getFrequency()!=freq) emit frequencyChanged((long long) freq);
        if (bandFrequency != freq)  emit frequencyChanged((long long) freq);
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
    emit getBandFrequency();
    if (ptt) {
        if (selectedVFO != 'A') {
            emit setFreq(freq, ptt);
            if (bandFrequency != freq)  emit frequencyChanged((long long) freq);
qDebug() << "Using vfoB, freq = " << freq << ", ptt = " << ptt;
        }
    } else if (selectedVFO == 'B') {
        if (!ui->pBtnSubRx->isChecked()) {
            emit setFreq(freq, ptt);
            if (bandFrequency != freq)  emit frequencyChanged((long long) freq);
        }
qDebug() << "Using vfoB, freq = " << freq << ", ptt = " << ptt;
    }
}

void vfo::checkBandBtn(int band)
{
    qDebug()<<Q_FUNC_INFO<<": Value of band button is ... "<<band;
    ui->btnGrpBand->button(band)->setChecked(TRUE);
}

void vfo::setBandButton(int freq)
{
    int cnt;

    for (cnt = 0; cnt < 12; cnt++) { // 12 buttons (0 ... 11)
        if (bands[cnt][bDat_fqmin] <= freq && bands[cnt][bDat_fqmax] >= freq) {
            cur_Band = cnt;
            ui->btnGrpBand->button(cnt)->setChecked(TRUE);
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
        if(!ui->pBtnRIT->isChecked()){
            writeB(readB());
        }
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
    writeB(settings->value("vfoB_f",14234567).toInt()); // vfoA initial settings from band.ccp
    settings->endGroup();
}

void vfo::writeSettings(QSettings* settings)
{
    settings->beginGroup("vfo");
    settings->setValue("vfoB_f", readB());
    settings->endGroup();
}


void vfo::on_pBtnSubRx_clicked()
{
//qDebug()<<Q_FUNC_INFO<<"VFO A readA() = "<<readA();
    if((selectedVFO == 'B')&(ui->pBtnSubRx->isChecked())) {
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

//Called when subRx is unchecked in main menu via actionSubRx()
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

QString vfo::rigctlGetvfo()
{
    QString vfo;
    vfo = selectedVFO;
    return "VFO" + vfo;
}
