#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>
#include <QStyle>
#include <QPainter>

#ifdef Q_OS_WIN
#include <dwmapi.h>
#include <windowsx.h>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "user32.lib")

// ---- Undocumented Win32 Acrylic API (not in the standard SDK headers) ----
enum AccentState {
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_ENABLE_HOSTBACKDROP = 5,
    ACCENT_INVALID_STATE = 6
};

struct ACCENT_POLICY {
    AccentState AccentState;
    DWORD AccentFlags;
    DWORD GradientColor; // format: ABGR (alpha in the high byte)
    DWORD AnimationId;
};

enum WINDOWCOMPOSITIONATTRIB {
    WCA_ACCENT_POLICY = 19
};

struct WINDOWCOMPOSITIONATTRIBDATA {
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
};

typedef BOOL(WINAPI *pSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA *);
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupWindowFlags();

    // winId() forces native HWND creation - must happen before any
    // Win32 calls that touch the window handle.
    winId();

#ifdef Q_OS_WIN
    extendFrame();
    enableAcrylicBlur();
    enableRoundedCorners();
#endif

    connect(qApp, &QGuiApplication::applicationStateChanged,
            this, &MainWindow::onApplicationStateChanged);
}

void MainWindow::onApplicationStateChanged(Qt::ApplicationState state) {
    const bool isActive = (state == Qt::ApplicationActive);

    for (auto *btn : findChildren<QPushButton*>()) {
        btn->setProperty("windowActive", isActive);
        // Force the style engine to re-evaluate the widget
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupWindowFlags()
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);

    resize(420, 560);
    setMinimumSize(320, 400);
}

#ifdef Q_OS_WIN
void MainWindow::extendFrame()
{
    HWND hwnd = reinterpret_cast<HWND>(winId());
    MARGINS margins = { 1, 1, 1, 1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);
}

void MainWindow::enableAcrylicBlur()
{
    HWND hwnd = reinterpret_cast<HWND>(winId());

    HMODULE hUser = GetModuleHandleW(L"user32.dll");
    if (!hUser)
        return;

    auto setWindowCompositionAttribute =
        reinterpret_cast<pSetWindowCompositionAttribute>(
            GetProcAddress(hUser, "SetWindowCompositionAttribute"));

    if (!setWindowCompositionAttribute)
        return;

    ACCENT_POLICY accent = {};
    accent.AccentState = ACCENT_ENABLE_ACRYLICBLURBEHIND;
    accent.AccentFlags = 0;

    // GradientColor is ABGR. Alpha controls how much blur/tint shows
    // through - lower alpha = more see-through. Tune to taste;
    // 0x20 = tint color (dark charcoal here), 0xC0 = ~75% alpha.
    const BYTE a = 0xC0;
    const BYTE b = 0x20;
    const BYTE g = 0x20;
    const BYTE r = 0x20;
    accent.GradientColor = (a << 24) | (b << 16) | (g << 8) | r;

    WINDOWCOMPOSITIONATTRIBDATA data;
    data.Attrib = WCA_ACCENT_POLICY;
    data.pvData = &accent;
    data.cbData = sizeof(accent);

    setWindowCompositionAttribute(hwnd, &data);
}

void MainWindow::enableRoundedCorners()
{
    // Windows 11 only (silently ignored/no-op on Windows 10).
    HWND hwnd = reinterpret_cast<HWND>(winId());
    const DWORD DWMWA_WINDOW_CORNER_PREFERENCE = 33;
    const DWORD DWMWCP_ROUND = 2;
    DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE,
                          &DWMWCP_ROUND, sizeof(DWMWCP_ROUND));
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(eventType)
    MSG *msg = static_cast<MSG *>(message);

    switch (msg->message) {
    case WM_NCCALCSIZE: {
        // Returning 0 here strips the native title bar/frame entirely.
        if (msg->wParam) {
            *result = 0;
            return true;
        }
        break;
    }
    case WM_NCHITTEST: {
        *result = 0;

        const LONG borderWidth = 8;
        RECT winrect;
        GetWindowRect(msg->hwnd, &winrect);

        long x = GET_X_LPARAM(msg->lParam);
        long y = GET_Y_LPARAM(msg->lParam);

        bool resizeWidth = minimumWidth() != maximumWidth();
        bool resizeHeight = minimumHeight() != maximumHeight();

        if (resizeWidth) {
            if (x >= winrect.left && x < winrect.left + borderWidth)
                *result = HTLEFT;
            if (x < winrect.right && x >= winrect.right - borderWidth)
                *result = HTRIGHT;
        }
        if (resizeHeight) {
            if (y >= winrect.top && y < winrect.top + borderWidth)
                *result = HTTOP;
            if (y < winrect.bottom && y >= winrect.bottom - borderWidth)
                *result = HTBOTTOM;
        }
        if (resizeWidth && resizeHeight) {
            if (x >= winrect.left && x < winrect.left + borderWidth &&
                y >= winrect.top && y < winrect.top + borderWidth)
                *result = HTTOPLEFT;
            if (x < winrect.right && x >= winrect.right - borderWidth &&
                y >= winrect.top && y < winrect.top + borderWidth)
                *result = HTTOPRIGHT;
            if (x >= winrect.left && x < winrect.left + borderWidth &&
                y < winrect.bottom && y >= winrect.bottom - borderWidth)
                *result = HTBOTTOMLEFT;
            if (x < winrect.right && x >= winrect.right - borderWidth &&
                y < winrect.bottom && y >= winrect.bottom - borderWidth)
                *result = HTBOTTOMRIGHT;
        }

        if (*result != 0)
            return true;

        // Not on a resize border - let it fall through so mouse events
        // still reach your DragArea widget for window dragging.
        return false;
    }
    }

    return QWidget::nativeEvent(eventType, message, result);
}
#endif

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    // Paint a nearly-transparent background so the window receives mouse events
    painter.fillRect(rect(), QColor(0, 0, 0, 1));
    QMainWindow::paintEvent(event);
}