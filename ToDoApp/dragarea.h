#ifndef DRAGAREA_H
#define DRAGAREA_H

#include <QWidget>

namespace Ui {
class DragArea;
}

class DragArea : public QWidget
{
    Q_OBJECT

public:
    explicit DragArea(QWidget *parent = nullptr);
    ~DragArea();

private:
    Ui::DragArea *ui;
};

#endif // DRAGAREA_H
