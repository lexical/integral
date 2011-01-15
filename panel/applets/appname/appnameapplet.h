/*
 * This file is part of unity-2d
 *
 * Copyright 2010 Canonical Ltd.
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

#ifndef APPNAMEAPPLET_H
#define APPNAMEAPPLET_H

// Local
#include <applet.h>

// Qt

class QEvent;

namespace Unity2d
{

struct AppNameAppletPrivate;
class AppNameApplet : public Applet
{
Q_OBJECT
public:
    AppNameApplet();
    ~AppNameApplet();

protected:
    bool event(QEvent*); // reimp
    bool eventFilter(QObject*, QEvent*); // reimp

private Q_SLOTS:
    void updateWidgets();

private:
    Q_DISABLE_COPY(AppNameApplet)
    AppNameAppletPrivate* const d;
};

} // namespace

#endif /* APPNAMEAPPLET_H */

