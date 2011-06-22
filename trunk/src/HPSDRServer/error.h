#ifndef ERROR_H
#define ERROR_H

#include <QString>

class Error {
public:
    Error();
    void clear();
    void set(QString e);
    QString get();

private:
    QString error;
};

#endif // ERROR_H
