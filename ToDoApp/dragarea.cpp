#include "dragarea.h"
#include "ui_dragarea.h"

DragArea::DragArea(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DragArea)
{
    ui->setupUi(this);
}

DragArea::~DragArea()
{
    delete ui;
}
