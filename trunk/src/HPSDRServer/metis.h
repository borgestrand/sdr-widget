#ifndef METIS_H
#define METIS_H

#include <QString>
#include <QtNetwork>

// command codes
#define PROGRAM_METIS_FLASH 0x01
#define ERASE_METIS_FLASH   0x02
#define READ_METIS_MAC      0x03
#define READ_METIS_IP       0x04
#define WRITE_METIS_IP      0x05
#define GET_JTAG_DEVICE_ID  0x06
#define PROGRAM_MERCURY     0x07
#define PROGRAM_PENELOPE    0x08
#define JTAG_ERASE_FLASH    0x09
#define PROGRAM_FLASH       0x0A

// reply codes
#define INVALID_COMMAND  0x00
#define ERASE_DONE       0x01
#define SEND_MORE        0x02
#define HAVE_MAC_ADDRESS 0x03
#define HAVE_IP_ADDRESS  0x04
#define FPGA_ID          0x05


class Metis {

public:
    Metis(long ipaddr,unsigned char* macaddr);
    QHostAddress* getHostAddress();
    long getIpAddress();
    unsigned char* getMACAddress();
    QString toString();

private:
    long ipaddress;
    unsigned char macaddress[6];
};

#endif // METIS_H
