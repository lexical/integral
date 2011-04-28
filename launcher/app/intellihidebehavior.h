/*
 * This file is part of unity-2d
 *
 * Copyright 2011 Canonical Ltd.
 *
 * Authors:
 * - Aurélien Gâteau <aurelien.gateau@canonical.com>
 *
 * License: GPL v3
 */
#ifndef INTELLIHIDEBEHAVIOR_H
#define INTELLIHIDEBEHAVIOR_H

// Local
#include <abstractvisibilitybehavior.h>

// Qt
#include <QObject>

struct _WnckWindow;

class QTimer;

class Unity2dPanel;
class EdgeMouseArea;

/**
 * This class implements the Intellihide behavior of the launcher
 */
class IntelliHideBehavior : public AbstractVisibilityBehavior
{
Q_OBJECT
public:
    IntelliHideBehavior(Unity2dPanel* panel);
    ~IntelliHideBehavior();

protected:
    bool eventFilter(QObject*, QEvent*);

private Q_SLOTS:
    void updateVisibility();
    void updateActiveWindowConnections();
    void showPanel();
    void hidePanel();

private:
    Q_DISABLE_COPY(IntelliHideBehavior);

    enum PanelVisibility {
        VisiblePanel,
        HiddenPanel
    };
    QTimer* m_updateVisibilityTimer;
    EdgeMouseArea* m_mouseArea;

    struct _WnckWindow* m_activeWindow;

    void disconnectFromGSignals();

    bool isMouseForcingVisibility() const;

    void createMouseArea();
};

#endif /* INTELLIHIDEBEHAVIOR_H */
