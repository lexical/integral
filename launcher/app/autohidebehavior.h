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
#ifndef AUTOHIDEBEHAVIOR_H
#define AUTOHIDEBEHAVIOR_H

// Local
#include <abstractvisibilitybehavior.h>

// Qt
#include <QObject>

class QTimer;
class Unity2dPanel;

/**
 * This class implements the classic autohide-on-timeout behavior
 */
class AutoHideBehavior : public AbstractVisibilityBehavior
{
Q_OBJECT
public:
    AutoHideBehavior(VisibilityController* controller, Unity2dPanel* panel);
    ~AutoHideBehavior();

    void mouseIsOverHomeButtonChanged();

protected:
    bool eventFilter(QObject*, QEvent*);

private:
    QTimer* m_autohideTimer;
};

#endif /* AUTOHIDEBEHAVIOR_H */
