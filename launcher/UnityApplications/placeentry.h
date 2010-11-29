/*
 * Copyright (C) 2010 Canonical, Ltd.
 *
 * Authors:
 *  Olivier Tilloy <olivier.tilloy@canonical.com>
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

#ifndef PLACEENTRY_H
#define PLACEENTRY_H

#include "launcheritem.h"

#include "deelistmodel.h"

#include <QMetaType>
#include <QDBusInterface>
#include <QDBusArgument>

// https://wiki.ubuntu.com/Unity/Places#RendererInfo
struct RendererInfoStruct
{
    QString default_renderer;
    QString groups_model;
    QString results_model;
    QMap<QString, QString> renderer_hints;
};
Q_DECLARE_METATYPE(RendererInfoStruct)

// Marshalling and unmarshalling of RendererInfoStruct
QDBusArgument &operator<<(QDBusArgument &, const RendererInfoStruct &);
const QDBusArgument &operator>>(const QDBusArgument &, RendererInfoStruct &);


// https://wiki.ubuntu.com/Unity/Places#PlaceEntryInfo
struct PlaceEntryInfoStruct
{
    QString dbus_path;
    QString name;
    QString icon;
    uint position;
    QStringList mimetypes;
    bool sensitive;
    QString sections_model;
    QMap<QString, QString> hints;
    RendererInfoStruct entry_renderer_info;
    RendererInfoStruct global_renderer_info;
};
Q_DECLARE_METATYPE(PlaceEntryInfoStruct)
Q_DECLARE_METATYPE(QList<PlaceEntryInfoStruct>)

// Marshalling and unmarshalling of PlaceEntryInfoStruct
QDBusArgument &operator<<(QDBusArgument &, const PlaceEntryInfoStruct &);
const QDBusArgument &operator>>(const QDBusArgument &, PlaceEntryInfoStruct &);


class PlaceEntry : public LauncherItem
{
    Q_OBJECT

    Q_PROPERTY(QString fileName READ fileName WRITE setFileName)
    Q_PROPERTY(QString groupName READ groupName WRITE setGroupName)
    Q_PROPERTY(QString dbusName READ dbusName WRITE setDbusName)
    Q_PROPERTY(QString dbusObjectPath READ dbusObjectPath WRITE setDbusObjectPath)
    Q_PROPERTY(uint position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(QStringList mimetypes READ mimetypes WRITE setMimetypes NOTIFY mimetypesChanged)
    Q_PROPERTY(bool sensitive READ sensitive WRITE setSensitive NOTIFY sensitiveChanged)
    Q_PROPERTY(DeeListModel* sections READ sections WRITE setSections NOTIFY sectionsChanged)
    Q_PROPERTY(QMap hints READ hints WRITE setHints NOTIFY hintsChanged)
    Q_PROPERTY(QString entryRendererName READ entryRendererName WRITE setEntryRendererName NOTIFY entryRendererNameChanged)
    Q_PROPERTY(QString entryGroupsModelName READ entryGroupsModelName WRITE setEntryGroupsModelName NOTIFY entryGroupsModelNameChanged)
    Q_PROPERTY(DeeListModel* entryGroupsModel READ entryGroupsModel WRITE setEntryGroupsModel NOTIFY entryGroupsModelChanged)

public:
    PlaceEntry();
    PlaceEntry(const PlaceEntry& other);
    ~PlaceEntry();

    /* getters */
    virtual bool active() const;
    virtual bool running() const;
    virtual bool urgent() const;
    virtual QString name() const;
    virtual QString icon() const;
    virtual bool launching() const;
    QString fileName() const;
    QString groupName() const;
    QString dbusName() const;
    QString dbusObjectPath() const;
    uint position() const;
    QStringList mimetypes() const;
    bool sensitive() const;
    DeeListModel* sections() const;
    QMap<QString, QVariant> hints() const;
    QString entryRendererName() const;
    QString entryGroupsModelName() const;
    DeeListModel* entryGroupsModel();
    bool online() const;

    /* setters */
    void setName(QString);
    void setIcon(QString);
    void setFileName(QString);
    void setGroupName(QString);
    void setDbusName(QString);
    void setDbusObjectPath(QString);
    void setPosition(uint);
    void setMimetypes(QStringList);
    void setSensitive(bool);
    void setSections(DeeListModel*);
    void setHints(QMap<QString, QVariant>);
    void setEntryRendererName(QString);
    void setEntryGroupsModelName(QString);
    void setEntryGroupsModel(DeeListModel*);

    /* methods */
    Q_INVOKABLE virtual void activate();
    Q_INVOKABLE virtual void createMenuActions();

    /* Connect to the remote representation of the entry on DBus and monitor
       changes. */
    void connectToRemotePlaceEntry();

    void updateInfo(const PlaceEntryInfoStruct& info);

Q_SIGNALS:
    void positionChanged(uint);
    void mimetypesChanged();
    void sensitiveChanged(bool);
    void sectionsChanged();
    void hintsChanged();
    void entryRendererNameChanged();
    void entryGroupsModelNameChanged();
    void entryGroupsModelChanged();

private:
    QString m_fileName;
    QString m_groupName;
    QString m_dbusName;
    QString m_dbusObjectPath;
    QString m_icon;
    QString m_name;
    uint m_position;
    QStringList m_mimetypes;
    bool m_sensitive;
    DeeListModel* m_sections;
    /* m_hints should be a QMap<QString, QString> really, but it has to hold
       QVariant values in order to allow exposing it as a property (see
       http://doc.trolltech.com/properties.html#requirements-for-declaring-properties). */
    QMap<QString, QVariant> m_hints;
    QString m_entryRendererName;
    QString m_entryGroupsModelName;
    DeeListModel* m_entryGroupsModel;
    bool m_online;
    QDBusInterface* m_dbusIface;

    void setSection(const QString&);
    void activateEntry(const int section);

private Q_SLOTS:
    void onRendererInfoChanged(const RendererInfoStruct&);
    void onPlaceEntryInfoChanged(const PlaceEntryInfoStruct&);

    /* Contextual menu callbacks */
    void onSectionTriggered();
};

Q_DECLARE_METATYPE(PlaceEntry*)

#endif // PLACEENTRY_H

