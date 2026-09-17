#include "dragarea.h"
#include "ui_dragarea.h"

DragArea::DragArea(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DragArea)
{
    ui->setupUi(this);

    connect(ui->closeButton,    &QPushButton::clicked, this, &DragArea::closeWindow);
    connect(ui->minimizeButton, &QPushButton::clicked, this, &DragArea::minimizeWindow);
    connect(ui->expandButton,   &QPushButton::clicked, this, &DragArea::expandWindow);
}

DragArea::~DragArea()
{
    delete ui;
}

void DragArea::closeWindow()
{
    mainWindow->close();
}

void DragArea::minimizeWindow()
{
    mainWindow->showMinimized();
}

void DragArea::expandWindow()
{
    if (!mainWindow->isMaximized())
        mainWindow->showMaximized();
    else
        mainWindow->showNormal();
}