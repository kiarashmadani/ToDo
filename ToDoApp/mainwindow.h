#pragma once

#include <QMainWindow>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
#ifdef Q_OS_WIN
    // Intercepts raw Win32 messages - needed for manual resize-border
    // hit-testing and to strip the native non-client frame.
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
#endif
    void paintEvent(QPaintEvent *event) override;

private:
    Ui::MainWindow *ui;

    void setupWindowFlags();
    void enableAcrylicBlur();
    void enableRoundedCorners();
    void extendFrame();

private slots:
    void onApplicationStateChanged(Qt::ApplicationState state);
};