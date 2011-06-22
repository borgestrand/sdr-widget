#include "Metis.h"

Metis::Metis(long ipaddr,unsigned char* macaddr) {
    ipaddress=ipaddr;
    for(int i=0;i<6;i++) {
        macaddress[i]=macaddr[i];
    }
}


long Metis::getIpAddress() {
    return ipaddress;
}

unsigned char* Metis::getMACAddress() {
    return macaddress;
}

QString Metis::getHostAddress() {
    QString address;
    address.sprintf("%ld.%ld.%ld.%ld",
                 (ipaddress>>24)&0xFF,(ipaddress>>16)&0xFF,(ipaddress>>8)&0xFF,ipaddress&0xFF);
    return address;
}

QString Metis::toString() {
    QString text;
    text.sprintf("%02X:%02X:%02X:%02X:%02x:%02X (%ld.%ld.%ld.%ld)",
                 macaddress[0],macaddress[1],macaddress[2],macaddress[3],macaddress[4],macaddress[5],
                 (ipaddress>>24)&0xFF,(ipaddress>>16)&0xFF,(ipaddress>>8)&0xFF,ipaddress&0xFF);
    return text;
}
