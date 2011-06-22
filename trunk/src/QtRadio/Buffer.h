#ifndef BUFFER_H
#define BUFFER_H

class Buffer
{
public:
    Buffer(char* hdr,char* buf);
    char* getHeader();
    char* getBuffer();
private:
    char* header;
    char* buffer;
};

#endif // BUFFER_H
