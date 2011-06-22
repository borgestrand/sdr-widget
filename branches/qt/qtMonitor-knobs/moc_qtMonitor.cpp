/****************************************************************************
** Meta object code from reading C++ file 'qtMonitor.h'
**
** Created: Fri Aug 13 11:59:55 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "qtMonitor.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtMonitor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_qtMonitor[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      31,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x0a,
      36,   10,   10,   10, 0x0a,
      60,   10,   10,   10, 0x0a,
      84,   10,   10,   10, 0x0a,
     108,   10,   10,   10, 0x0a,
     132,   10,   10,   10, 0x0a,
     156,   10,   10,   10, 0x0a,
     180,   10,   10,   10, 0x0a,
     204,   10,   10,   10, 0x0a,
     228,   10,   10,   10, 0x0a,
     252,   10,   10,   10, 0x0a,
     275,   10,   10,   10, 0x0a,
     300,   10,   10,   10, 0x0a,
     325,   10,   10,   10, 0x0a,
     350,   10,   10,   10, 0x0a,
     375,   10,   10,   10, 0x0a,
     400,   10,   10,   10, 0x0a,
     425,   10,   10,   10, 0x0a,
     449,   10,   10,   10, 0x0a,
     485,  473,   10,   10, 0x0a,
     527,   10,   10,   10, 0x0a,
     539,   10,   10,   10, 0x0a,
     552,   10,   10,   10, 0x0a,
     566,  561,   10,   10, 0x0a,
     588,  579,   10,   10, 0x0a,
     617,  607,   10,   10, 0x0a,
     643,  641,   10,   10, 0x0a,
     664,  662,   10,   10, 0x0a,
     683,  662,   10,   10, 0x0a,
     710,  705,   10,   10, 0x0a,
     730,  723,   10,   10, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_qtMonitor[] = {
    "qtMonitor\0\0band_160_buttonPressed()\0"
    "band_80_buttonPressed()\0band_60_buttonPressed()\0"
    "band_40_buttonPressed()\0band_30_buttonPressed()\0"
    "band_20_buttonPressed()\0band_17_buttonPressed()\0"
    "band_15_buttonPressed()\0band_12_buttonPressed()\0"
    "band_10_buttonPressed()\0band_6_buttonPressed()\0"
    "band_gen_buttonPressed()\0"
    "mode_lsb_buttonPressed()\0"
    "mode_usb_buttonPressed()\0"
    "mode_dsb_buttonPressed()\0"
    "mode_cwl_buttonPressed()\0"
    "mode_cwu_buttonPressed()\0"
    "mode_am_buttonPressed()\0connect_buttonPressed()\0"
    "socketError\0socketError(QAbstractSocket::SocketError)\0"
    "connected()\0socketData()\0update()\0"
    "mode\0setMode(int)\0low,high\0"
    "setFilter(int,int)\0frequency\0"
    "setFrequency(long long)\0f\0moveFrequency(int)\0"
    "i\0vfo_dialMoved(int)\0afgain_dialMoved(int)\0"
    "gain\0setGain(int)\0choice\0audioChanged(int)\0"
};

const QMetaObject qtMonitor::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_qtMonitor,
      qt_meta_data_qtMonitor, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &qtMonitor::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *qtMonitor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *qtMonitor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_qtMonitor))
        return static_cast<void*>(const_cast< qtMonitor*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int qtMonitor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: band_160_buttonPressed(); break;
        case 1: band_80_buttonPressed(); break;
        case 2: band_60_buttonPressed(); break;
        case 3: band_40_buttonPressed(); break;
        case 4: band_30_buttonPressed(); break;
        case 5: band_20_buttonPressed(); break;
        case 6: band_17_buttonPressed(); break;
        case 7: band_15_buttonPressed(); break;
        case 8: band_12_buttonPressed(); break;
        case 9: band_10_buttonPressed(); break;
        case 10: band_6_buttonPressed(); break;
        case 11: band_gen_buttonPressed(); break;
        case 12: mode_lsb_buttonPressed(); break;
        case 13: mode_usb_buttonPressed(); break;
        case 14: mode_dsb_buttonPressed(); break;
        case 15: mode_cwl_buttonPressed(); break;
        case 16: mode_cwu_buttonPressed(); break;
        case 17: mode_am_buttonPressed(); break;
        case 18: connect_buttonPressed(); break;
        case 19: socketError((*reinterpret_cast< QAbstractSocket::SocketError(*)>(_a[1]))); break;
        case 20: connected(); break;
        case 21: socketData(); break;
        case 22: update(); break;
        case 23: setMode((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 24: setFilter((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 25: setFrequency((*reinterpret_cast< long long(*)>(_a[1]))); break;
        case 26: moveFrequency((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 27: vfo_dialMoved((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 28: afgain_dialMoved((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 29: setGain((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 30: audioChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 31;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
