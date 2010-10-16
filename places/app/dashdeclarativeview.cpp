#include "dashdeclarativeview.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QCloseEvent>
#include <QDeclarativeContext>
#include <QX11Info>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

DashDeclarativeView::DashDeclarativeView() :
    QDeclarativeView(), m_active(false)
{
}

void
DashDeclarativeView::fitToAvailableSpace(int screen)
{
    QDesktopWidget *desktop = QApplication::desktop();    
    int current_screen = desktop->screenNumber(this);

    if(screen == current_screen)
    {
        QRect geometry = desktop->availableGeometry(this);
        setGeometry(geometry);
        setFixedSize(geometry.size());
    }
}

void
DashDeclarativeView::closeEvent(QCloseEvent* event)
{
    event->ignore();
}

void
DashDeclarativeView::setActive(bool active)
{
    m_active = active;

    if(active)
    {
        setAttribute(Qt::WA_X11NetWmWindowTypeDesktop, false);
        raise();
        activateWindow();
        forceActivateWindow();
        setAttribute(Qt::WA_X11NetWmWindowTypeDock, true);
    }
    else
    {
        setAttribute(Qt::WA_X11NetWmWindowTypeDock, false);
        lower();
        clearFocus();
        setAttribute(Qt::WA_X11NetWmWindowTypeDesktop, true);
    }

    emit activeChanged(active);
}

bool
DashDeclarativeView::active() const
{
    return m_active;
}

void
DashDeclarativeView::forceActivateWindow()
{
    /* Workaround focus stealing prevention implemented by some window
       managers such as Compiz. This is the exact same code you will find in
       libwnck::wnck_window_activate().

       ref.: http://permalink.gmane.org/gmane.comp.lib.qt.general/4733
    */
    Display* display = QX11Info::display();
    Atom net_wm_active_window = XInternAtom(display, "_NET_ACTIVE_WINDOW",
                                            False);
    XEvent xev;
    xev.xclient.type = ClientMessage;
    xev.xclient.send_event = True;
    xev.xclient.display = display;
    xev.xclient.window = this->effectiveWinId();
    xev.xclient.message_type = net_wm_active_window;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 2;
    xev.xclient.data.l[1] = CurrentTime;
    xev.xclient.data.l[2] = 0;
    xev.xclient.data.l[3] = 0;
    xev.xclient.data.l[4] = 0;

    XSendEvent(display, QX11Info::appRootWindow(), False,
               SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}
