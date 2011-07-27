/*
 * This file is part of unity-2d
 *
 * Copyright 2011 Canonical Ltd.
 *
 * Authors:
 * - Aurélien Gâteau <aurelien.gateau@canonical.com>
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
// Self
#include "indicatorapplet.h"

// Local
#include <debug_p.h>
#include <indicatorsmanager.h>
#include <indicatorwidget.h>

// Qt
#include <QHBoxLayout>

using namespace unity::indicator;

IndicatorApplet::IndicatorApplet(IndicatorsManager* manager)
: m_indicatorsManager(manager)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

    m_indicatorsManager->indicators()->on_object_added.connect(
        sigc::mem_fun(this, &IndicatorApplet::onObjectAdded)
        );
}

void IndicatorApplet::onObjectAdded(Indicator::Ptr const& indicator)
{
    QString name = QString::fromStdString(indicator->name());
    if (name == "libappmenu.so") {
        // appmenu indicator is handled by AppNameApplet
        return;
    }
    IndicatorWidget* widget = new IndicatorWidget(indicator, m_indicatorsManager);
    layout()->addWidget(widget);
}

#include "indicatorapplet.moc"
