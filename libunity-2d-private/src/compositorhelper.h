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

#ifndef X11COMPOSITORHELPER_H
#define X11COMPOSITORHELPER_H

#include <QObject>

typedef unsigned long Window;

class CompositorHelper : public QObject
{
	Q_OBJECT
public:
    static CompositorHelper* instance();

    void activateComposite();
    void deactivateComposite();

    bool isCompositeSupported() { return m_x11supportsComposite; }
    bool isCompositeActive() { return m_compositeActivated; }

private:
    explicit CompositorHelper();

    bool m_x11supportsComposite;
    bool m_compositeActivated;
};

#endif // X11COMPOSITORHELPER_H
