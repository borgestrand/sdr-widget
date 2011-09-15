#ifndef AUX_H
#define AUX_H

#include <QFrame>

namespace Ui {
    class Aux;
}

class Aux : public QFrame
{
    Q_OBJECT

public:
    explicit Aux(QWidget *parent = 0);
    ~Aux();

private:
    Ui::Aux *ui;
};

#endif // AUX_H
