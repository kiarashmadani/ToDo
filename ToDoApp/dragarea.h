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

    QWidget *mainWindow = this->window();

private slots:
    void closeWindow();
    void minimizeWindow();
    void expandWindow();
};

#endif // DRAGAREA_H
