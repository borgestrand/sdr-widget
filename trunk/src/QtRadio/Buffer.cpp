#include "Buffer.h"

Buffer::Buffer(char* hdr,char* buf) {
    header=hdr;
    buffer=buf;
}

char* Buffer::getHeader() {
    return header;
}

char* Buffer::getBuffer() {
    return buffer;
}
