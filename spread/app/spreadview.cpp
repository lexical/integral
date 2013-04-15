/*
 * Copyright (C) 2010 Canonical, Ltd.
 *
 * Authors:
 *  Ugo Riboni <ugo.riboni@canonical.com>
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

#include "spreadview.h"

#include <QDesktopWidget>
#include <QApplication>
#include <QMouseEvent>
#include <QDeclarativeItem>

#include "screeninfo.h"
#include "config.h"
#include "compositorhelper.h"

SpreadView::SpreadView(int screen)
: Unity2DDeclarativeView()
{
    m_screenInfo = new ScreenInfo(screen, this);
    connect(m_screenInfo, SIGNAL(panelsFreeGeometryChanged(QRect)), SLOT(fitToAvailableSpace()));

    bool jitComposite = spread2dConfiguration().property("jit-composite").toBool();
    if (CompositorHelper::instance()->isCompositeSupported()
        && ! jitComposite) {
        CompositorHelper::instance()->activateComposite();
    }
}

void SpreadView::fitToAvailableSpace()
{
    QRect geometry = m_screenInfo->panelsFreeGeometry();
    setGeometry(geometry);
    setFixedSize(geometry.size());
    const int width = geometry.width();
    const int height = geometry.height();
    rootObject()->setWidth(width);
    rootObject()->setHeight(height);
    setSceneRect(QRectF(0, 0, width, height));
}

void SpreadView::preparePreviews()
{
    /* deferred activation of window compositing to preserve
       server side memory usage (important on low-end ARM systems)
    */
    if (CompositorHelper::instance()->isCompositeSupported()
        && ! CompositorHelper::instance()->isCompositeActive()) {
        CompositorHelper::instance()->activateComposite();
    }

}

void SpreadView::unloadPreviews()
{
    bool jitComposite = spread2dConfiguration().property("jit-composite").toBool();

    if (CompositorHelper::instance()->isCompositeSupported()
        && CompositorHelper::instance()->isCompositeActive()
        && jitComposite) {
        CompositorHelper::instance()->deactivateComposite();
    }
}

bool SpreadView::previewsAvailable() const
{
    return (CompositorHelper::instance()->isCompositeSupported()
            && CompositorHelper::instance()->isCompositeActive());
}
