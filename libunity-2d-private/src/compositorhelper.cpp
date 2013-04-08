/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QX11Info>
#include <QTimer>

#include "compositorhelper.h"
#include <debug_p.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xcomposite.h>

/* Do NOT activate the composite mode: this disables live windows previews,
   as used in the spread and workspace switcher views
*/
#define DISABLE_COMPOSITE_MODE 0

CompositorHelper::CompositorHelper():
	QObject(),
	m_x11supportsComposite(false),
	m_compositeActivated(false)
{
    int event_base, error_base;

    if (XCompositeQueryExtension(QX11Info::display(), &event_base, &error_base)) {
        int major = 0;
        int minor = 2;
        XCompositeQueryVersion(QX11Info::display(), &major, &minor);

        if (major > 0 || minor >= 2) {
            m_x11supportsComposite = true;
            (UQ_DEBUG).nospace() << "Server supports the Composite extension (ver "
                                 << major << "." << minor << ")";
        }
        else {
            (UQ_DEBUG).nospace() << "Server supports the Composite extension, but "
                "version is < 0.2 (ver " << major << "." << minor << ")";
        }
    } else {
        UQ_DEBUG << "Server doesn't support the Composite extension.";
    }
}

/* Activate composite, so we can capture windows that are partially obscured
   Ideally we want to activate it only when QX11Info::isCompositingManagerRunning()
   is false, but in my experience it is not reliable at all.
   The only downside when calling this is that there's a small visual glitch at the
   moment when it's called on the entire desktop, and the same thing when the app
   terminates. This happens regardless if the WM has activated composite already or
   not.

   This updated version activates compositing selectively on individual windows
   to optimize memory usage in the X server.
*/
void CompositorHelper::activateComposite(Window windowId)
{
#if DISABLE_COMPOSITE_MODE
    return;
#endif
	
    Display *display = QX11Info::display();

	if (windowId != QX11Info::appRootWindow()) {
		/* the parent is most probably the root window */
		windowId = QX11Info::appRootWindow();
		XCompositeRedirectSubwindows(display, windowId,
									 CompositeRedirectAutomatic);

		(UQ_DEBUG).nospace() << "composite mode activated for window "
							 << QString("0x%1").arg(windowId, 0, 16);
	} else {

		/* Ask the X Composite extension (if supported) to redirect all
		   windows on all screens to backing storage. This does not have
		   any effect if another application already requested the same
		   thing (typically the window manager does this).
		*/
		int screens = ScreenCount(display);
		for (int i = 0; i < screens; ++i) {
			XCompositeRedirectSubwindows(display, RootWindow(display, i),
										 CompositeRedirectAutomatic);
		}
	
		(UQ_DEBUG).nospace() << "composite mode activated for the whole desktop";
	}

	m_compositeActivated = true;

	// TODO: program timeout
	QTimer::singleShot(5000, this, SLOT(timeout()));
}

void CompositorHelper::deactivateComposite(Window windowId)
{
    Display *display = QX11Info::display();

	if (windowId != QX11Info::appRootWindow()) {
		/* the parent is most probably the root window */
		windowId = QX11Info::appRootWindow();
		XCompositeUnredirectSubwindows(display, windowId,
									   CompositeRedirectAutomatic);
		(UQ_DEBUG).nospace() << "composite mode deactivated for window "
							 << QString("0x%1").arg(windowId, 0, 16);
	} else {
		int screens = ScreenCount(display);
		for (int i = 0; i < screens; ++i) {
			XCompositeUnredirectSubwindows(display, RootWindow(display, i),
										   CompositeRedirectAutomatic);
		}
		(UQ_DEBUG).nospace() << "composite mode deactivated for the whole desktop";
	}

	m_compositeActivated = false;
}

void CompositorHelper::timeout()
{
	deactivateComposite(QX11Info::appRootWindow());
}


#include "compositorhelper.moc"
