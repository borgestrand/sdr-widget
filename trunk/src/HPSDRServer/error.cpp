#include <QDebug>

#include "error.h"

Error::Error() {
    error="";
}

void Error::clear() {
    error="";
}

void Error::set(QString e) {
    error=e;
}

QString Error::get() {
    return error;
}
