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

#ifndef SPREADVIEW_H
#define SPREADVIEW_H

#include <QDeclarativeView>
#include <QEvent>

#include <unity2ddeclarativeview.h>

class SpreadView : public Unity2DDeclarativeView
{
    Q_OBJECT

public:
    explicit SpreadView(int screen);

public Q_SLOTS:
    void fitToAvailableSpace();

    void preparePreviews();
    void unloadPreviews();
};

Q_DECLARE_METATYPE(SpreadView*)

#endif // SPREADVIEW_H
