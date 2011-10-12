#ifndef CTL_H
#define CTL_H

#include <QFrame>

namespace Ui {
    class Ctl;
}

class Ctl : public QFrame
{
    Q_OBJECT

public:
    explicit Ctl(QWidget *parent = 0);
    ~Ctl();

private:
    Ui::Ctl *ui;
};

#endif // CTL_H
