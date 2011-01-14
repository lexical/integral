/*
 * This file is part of unity-qt
 *
 * Copyright 2010 Canonical Ltd.
 *
 * Authors:
 * - Aurélien Gâteau <aurelien.gateau@canonical.com>
 *
 * License: GPL v3
 */
// Self
#include "datetimeindicator.h"

// Local
#include "indicatorservicemanager.h"

// dbusmenu-qt
#include "dbusmenuimporter.h"

// Qt
#include <QAction>
#include <QDateTime>

// From dbus-shared.h
#define  SERVICE_NAME     "com.canonical.indicator.datetime"
#define  SERVICE_IFACE    "com.canonical.indicator.datetime.service"
#define  SERVICE_OBJ      "/com.canonical/indicator/datetime/service"
#define  SERVICE_VERSION  1

#define  MENU_OBJ      "/com.canonical/indicator/datetime/menu"

#define  DBUSMENU_CALENDAR_MENUITEM_TYPE "x-canonical-calendar-item"
////

DateTimeIndicator::DateTimeIndicator(QObject* parent)
: AbstractIndicator(parent)
, m_action(new QAction(this))
, m_timer(new QTimer(this))
{
    new IndicatorServiceManager(SERVICE_NAME, SERVICE_VERSION, this);

    readConfig();
    setupMenu();
    setupTimer();
    updateText();
}

void DateTimeIndicator::init()
{
    actionAdded(m_action);
}

void DateTimeIndicator::setupMenu()
{
    DBusMenuImporter* importer = new DBusMenuImporter(SERVICE_NAME, MENU_OBJ, this);
    m_action->setMenu(importer->menu());
}

void DateTimeIndicator::setupTimer()
{
    m_timer->setSingleShot(false);
    connect(m_timer, SIGNAL(timeout()), SLOT(updateText()));
    updateTimer();
    m_timer->start();
}

void DateTimeIndicator::updateTimer()
{
    m_timer->setInterval(1000);
}

void DateTimeIndicator::updateText()
{
    QString text = QDateTime::currentDateTime().toString(m_format);
    m_action->setText(text);
}

void DateTimeIndicator::readConfig()
{
    m_format = "hh:mm:ss";
}

#include "datetimeindicator.moc"