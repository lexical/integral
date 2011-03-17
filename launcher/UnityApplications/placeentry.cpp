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

#include "placeentry.h"
#include "place.h"
#include "launchermenu.h"

#include <QDBusMetaType>
#include <QAction>
#include <QDebug>
#include <QDBusServiceWatcher>
#include <QDBusConnectionInterface>
#include <QDBusReply>

// Marshall the RendererInfoStruct data into a D-Bus argument
QDBusArgument &operator<<(QDBusArgument &argument, const RendererInfoStruct &r)
{
    argument.beginStructure();
    argument << r.default_renderer;
    argument << r.groups_model;
    argument << r.results_model;
    argument.beginMap(QVariant::String, QVariant::String);
    QMap<QString, QString>::const_iterator i;
    for (i = r.renderer_hints.constBegin(); i != r.renderer_hints.constEnd(); ++i) {
        argument.beginMapEntry();
        argument << i.key() << i.value();
        argument.endMapEntry();
    }
    argument.endMap();
    argument.endStructure();
    return argument;
}

// Retrieve the RendererInfoStruct data from the D-Bus argument
const QDBusArgument &operator>>(const QDBusArgument &argument, RendererInfoStruct &r)
{
    argument.beginStructure();
    argument >> r.default_renderer;
    argument >> r.groups_model;
    argument >> r.results_model;
    r.renderer_hints.clear();
    argument.beginMap();
    while (!argument.atEnd()) {
        QString key;
        QString value;
        argument.beginMapEntry();
        argument >> key >> value;
        argument.endMapEntry();
        r.renderer_hints[key] = value;
    }
    argument.endMap();
    argument.endStructure();
    return argument;
}


// Marshall the PlaceEntryInfoStruct data into a D-Bus argument
QDBusArgument &operator<<(QDBusArgument &argument, const PlaceEntryInfoStruct &p)
{
    argument.beginStructure();
    argument << p.dbus_path;
    argument << p.name;
    argument << p.icon;
    argument << p.position;
    argument.beginArray(QVariant::String);
    QStringList::const_iterator i;
    for (i = p.mimetypes.constBegin(); i != p.mimetypes.constEnd(); ++i) {
        argument << *i;
    }
    argument.endArray();
    argument << p.sensitive;
    argument << p.sections_model;
    argument.beginMap(QVariant::String, QVariant::String);
    QMap<QString, QString>::const_iterator j;
    for (j = p.hints.constBegin(); j != p.hints.constEnd(); ++j) {
        argument.beginMapEntry();
        argument << j.key() << j.value();
        argument.endMapEntry();
    }
    argument.endMap();
    argument << p.entry_renderer_info;
    argument << p.global_renderer_info;
    argument.endStructure();
    return argument;
}

// Retrieve the PlaceEntryInfoStruct data from the D-Bus argument
const QDBusArgument &operator>>(const QDBusArgument &argument, PlaceEntryInfoStruct &p)
{
    argument.beginStructure();
    argument >> p.dbus_path;
    argument >> p.name;
    argument >> p.icon;
    argument >> p.position;
    argument.beginArray();
    p.mimetypes.clear();
    while (!argument.atEnd()) {
        QString mimetype;
        argument >> mimetype;
        p.mimetypes.append(mimetype);
    }
    argument.endArray();
    argument >> p.sensitive;
    argument >> p.sections_model;
    p.hints.clear();
    argument.beginMap();
    while (!argument.atEnd()) {
        QString key;
        QString value;
        argument.beginMapEntry();
        argument >> key >> value;
        argument.endMapEntry();
        p.hints[key] = value;
    }
    argument.endMap();
    /* The PlaceEntryInfoChanged signal on the com.canonical.Unity.PlaceEntry
       interface omits the two RenderingInfo structs. */
    if (!argument.atEnd()) {
        argument >> p.entry_renderer_info;
        argument >> p.global_renderer_info;
    }
    argument.endStructure();
    return argument;
}


static const char* UNITY_PLACE_ENTRY_INTERFACE = "com.canonical.Unity.PlaceEntry";
static const char* SECTION_PROPERTY = "section";

static const char* DASH_DBUS_SERVICE = "com.canonical.Unity2d.Dash";
static const char* DASH_DBUS_PATH = "/Dash";
static const char* DASH_DBUS_INTERFACE = "com.canonical.Unity2d.Dash";

PlaceEntry::PlaceEntry(QObject* parent) :
    LauncherItem(parent),
    m_position(0),
    m_sensitive(false),
    m_sections(NULL),
    m_activeSection(-1),
    m_active(false),
    m_place((Place*)parent),
    m_entryGroupsModel(NULL),
    m_entryResultsModel(NULL),
    m_globalGroupsModel(NULL),
    m_globalResultsModel(NULL),
    m_dbusIface(NULL),
    m_dashDbusIface(NULL),
    m_dashActive(false)
{
    qDBusRegisterMetaType<RendererInfoStruct>();
    qDBusRegisterMetaType<PlaceEntryInfoStruct>();
    qDBusRegisterMetaType<QList<PlaceEntryInfoStruct> >();
    qDBusRegisterMetaType<QHash<QString, QString>>();

    /* Check if the dash is already up and running by asking the bus instead of
       trying to create an instance of the interface. Creating an instance would
       cause D-Bus to activate the dash and we don’t want this to happen, the
       dash should be started on demand only. */
    QDBusConnectionInterface* sessionBusIFace = QDBusConnection::sessionBus().interface();
    QDBusReply<bool> reply = sessionBusIFace->isServiceRegistered(DASH_DBUS_SERVICE);
    if (reply.isValid() && reply.value()) {
        connectToDash();
    } else {
        /* The dash is not running: monitor its registration on the bus so we
           can connect to it when it comes up. */
        QDBusServiceWatcher* watcher = new QDBusServiceWatcher(DASH_DBUS_SERVICE,
                                                               QDBusConnection::sessionBus(),
                                                               QDBusServiceWatcher::WatchForRegistration,
                                                               this);
        connect(watcher, SIGNAL(serviceRegistered(QString)), SLOT(connectToDash()));
    }
}

PlaceEntry::PlaceEntry(const PlaceEntry& other) :
    m_fileName(other.m_fileName),
    m_groupName(other.m_groupName),
    m_dbusName(other.m_dbusName),
    m_dbusObjectPath(other.m_dbusObjectPath),
    m_icon(other.m_icon),
    m_name(other.m_name),
    m_position(other.m_position),
    m_mimetypes(other.m_mimetypes),
    m_sensitive(other.m_sensitive),
    m_activeSection(other.m_activeSection),
    m_active(other.m_active),
    m_place(other.m_place),

    m_entrySearchQuery(other.m_entrySearchQuery),
    m_entryRendererName(other.m_entryRendererName),
    m_entryGroupsModelName(other.m_entryGroupsModelName),
    m_entryResultsModelName(other.m_entryResultsModelName),

    m_globalSearchQuery(other.m_globalSearchQuery),
    m_globalRendererName(other.m_globalRendererName),
    m_globalGroupsModelName(other.m_globalGroupsModelName),
    m_globalResultsModelName(other.m_globalResultsModelName)
{
    setSections(other.m_sections);
    setHints(other.m_hints);

    setEntryGroupsModel(other.m_entryGroupsModel);
    setEntryResultsModel(other.m_entryResultsModel);
    setEntryRendererHints(other.m_entryRendererHints);

    setGlobalGroupsModel(other.m_globalGroupsModel);
    setGlobalResultsModel(other.m_globalResultsModel);
    setGlobalRendererHints(other.m_globalRendererHints);
}

PlaceEntry::~PlaceEntry()
{
    delete m_sections;
    delete m_dbusIface;
}

bool
PlaceEntry::running() const
{
    return false;
}

int
PlaceEntry::windowCount() const
{
    return 0;
}

bool
PlaceEntry::urgent() const
{
    return false;
}

QString
PlaceEntry::name() const
{
    return m_name;
}

void
PlaceEntry::setName(QString name)
{
    m_name = name;
}

QString
PlaceEntry::icon() const
{
    return m_icon;
}

void
PlaceEntry::setIcon(QString icon)
{
    m_icon = icon;
}

bool
PlaceEntry::launching() const
{
    /* This basically means no launching animation when opening the device.
       Unity behaves likes this. */
    return false;
}

QString
PlaceEntry::fileName() const
{
    return m_fileName;
}

void
PlaceEntry::setFileName(QString fileName)
{
    m_fileName = fileName;
}

QString
PlaceEntry::groupName() const
{
    return m_groupName;
}

void
PlaceEntry::setGroupName(QString groupName)
{
    m_groupName = groupName;
}

QString
PlaceEntry::dbusName() const
{
    return m_dbusName;
}

QString
PlaceEntry::dbusObjectPath() const
{
    return m_dbusObjectPath;
}

uint
PlaceEntry::position() const
{
    return m_position;
}

QStringList
PlaceEntry::mimetypes() const
{
    return m_mimetypes;
}

bool
PlaceEntry::sensitive() const
{
    return m_sensitive;
}

DeeListModel*
PlaceEntry::sections() const
{
    return m_sections;
}

QMap<QString, QVariant>
PlaceEntry::hints() const
{
    return m_hints;
}

int
PlaceEntry::activeSection() const
{
    return m_activeSection;
}

bool
PlaceEntry::active() const
{
    return m_active;
}

Place*
PlaceEntry::place() const
{
    return m_place;
}

QString
PlaceEntry::entrySearchQuery() const
{
    return m_entrySearchQuery;
}

QString
PlaceEntry::entryRendererName() const
{
    return m_entryRendererName;
}

QString
PlaceEntry::entryGroupsModelName() const
{
    return m_entryGroupsModelName;
}

DeeListModel*
PlaceEntry::entryGroupsModel()
{
    if (m_entryGroupsModel == NULL) {
        if (!m_entryGroupsModelName.isNull()) {
            m_entryGroupsModel = new DeeListModel;
            m_entryGroupsModel->setName(m_entryGroupsModelName);
        }
    }
    return m_entryGroupsModel;
}

QString
PlaceEntry::entryResultsModelName() const
{
    return m_entryResultsModelName;
}

DeeListModel*
PlaceEntry::entryResultsModel()
{
    if (m_entryResultsModel == NULL) {
        if (!m_entryResultsModelName.isNull()) {
            m_entryResultsModel = new DeeListModel;
            m_entryResultsModel->setName(m_entryResultsModelName);
        }
    }
    return m_entryResultsModel;
}

QMap<QString, QVariant>
PlaceEntry::entryRendererHints() const
{
    return m_entryRendererHints;
}

QString
PlaceEntry::globalSearchQuery() const
{
    return m_globalSearchQuery;
}

QString
PlaceEntry::globalRendererName() const
{
    return m_globalRendererName;
}

QString
PlaceEntry::globalGroupsModelName() const
{
    return m_globalGroupsModelName;
}

DeeListModel*
PlaceEntry::globalGroupsModel()
{
    if (m_globalGroupsModel == NULL) {
        if (!m_globalGroupsModelName.isNull()) {
            m_globalGroupsModel = new DeeListModel;
            m_globalGroupsModel->setName(m_globalGroupsModelName);
        }
    }
    return m_globalGroupsModel;
}

QString
PlaceEntry::globalResultsModelName() const
{
    return m_globalResultsModelName;
}

DeeListModel*
PlaceEntry::globalResultsModel()
{
    if (m_globalResultsModel == NULL) {
        if (!m_globalResultsModelName.isNull()) {
            m_globalResultsModel = new DeeListModel;
            m_globalResultsModel->setName(m_globalResultsModelName);
        }
    }
    return m_globalResultsModel;
}

QMap<QString, QVariant>
PlaceEntry::globalRendererHints() const
{
    return m_globalRendererHints;
}

void
PlaceEntry::setDbusName(QString dbusName)
{
    m_dbusName = dbusName;
}

void
PlaceEntry::setDbusObjectPath(QString dbusObjectPath)
{
    m_dbusObjectPath = dbusObjectPath;
}

void
PlaceEntry::setPosition(uint position)
{
    if (position != m_position) {
        m_position = position;
        Q_EMIT positionChanged(position);
    }
}

void
PlaceEntry::setMimetypes(QStringList mimetypes)
{
    m_mimetypes = mimetypes;
    Q_EMIT mimetypesChanged();
}

void
PlaceEntry::setSensitive(bool sensitive)
{
    if (sensitive != m_sensitive) {
        m_sensitive = sensitive;
        Q_EMIT sensitiveChanged(sensitive);
    }
}

void
PlaceEntry::setSections(DeeListModel* sections)
{
    if (sections == NULL) {
        return;
    }
    if (m_sections != NULL) {
        delete m_sections;
    }
    m_sections = sections;
    Q_EMIT sectionsChanged();
}

void
PlaceEntry::setHints(QMap<QString, QVariant> hints)
{
    m_hints = hints;
    Q_EMIT hintsChanged();
}

void
PlaceEntry::setActiveSection(int activeSection)
{
    if (activeSection != m_activeSection) {
        m_activeSection = activeSection;
        if (m_dbusIface != NULL) {
            /* the cast to uint is necessary for the D-Bus call to succeed as the
               interface expects that type */
            m_dbusIface->call("SetActiveSection", (uint)m_activeSection);
        }
        Q_EMIT activeSectionChanged();
    }
}

void
PlaceEntry::setActive(bool active)
{
    if (active != m_active) {
        m_active = active;
        if (m_dbusIface != NULL) {
            m_dbusIface->call("SetActive", m_active);
        }
        Q_EMIT activeChanged();

        if (m_dbusIface && m_active) {
            /* SetActiveSection needs to be called after SetActive(true)
               in order for it to have an effect. */
            m_dbusIface->call("SetActiveSection", m_activeSection);
        }
    }
}

void
PlaceEntry::setEntrySearchQuery(QString entrySearchQuery)
{
    if (entrySearchQuery != m_entrySearchQuery) {
        m_entrySearchQuery = entrySearchQuery;
        if (m_dbusIface != NULL) {
            QHash<QString, QString> searchHints;
            m_dbusIface->call("SetSearch", m_entrySearchQuery, qVariantFromValue(searchHints));
        }
        Q_EMIT entrySearchQueryChanged();
    }
}

void
PlaceEntry::setEntryRendererName(QString entryRendererName)
{
    if (entryRendererName != m_entryRendererName) {
        m_entryRendererName = entryRendererName;
        Q_EMIT entryRendererNameChanged();
    }
}

void
PlaceEntry::setEntryGroupsModelName(QString entryGroupsModelName)
{
    if (entryGroupsModelName != m_entryGroupsModelName) {
        m_entryGroupsModelName = entryGroupsModelName;
        delete m_entryGroupsModel;
        m_entryGroupsModel = NULL;
        Q_EMIT entryGroupsModelNameChanged();
        Q_EMIT entryGroupsModelChanged();
    }
}

void
PlaceEntry::setEntryGroupsModel(DeeListModel* entryGroupsModel)
{
    if (entryGroupsModel == NULL) {
        return;
    }
    if (m_entryGroupsModel != NULL) {
        delete m_entryGroupsModel;
    }
    m_entryGroupsModel = entryGroupsModel;
    Q_EMIT entryGroupsModelChanged();
}

void
PlaceEntry::setEntryResultsModelName(QString entryResultsModelName)
{
    if (entryResultsModelName != m_entryResultsModelName) {
        m_entryResultsModelName = entryResultsModelName;
        delete m_entryResultsModel;
        m_entryResultsModel = NULL;
        Q_EMIT entryResultsModelNameChanged();
        Q_EMIT entryResultsModelChanged();
    }
}

void
PlaceEntry::setEntryResultsModel(DeeListModel* entryResultsModel)
{
    if (entryResultsModel == NULL) {
        return;
    }
    if (m_entryResultsModel != NULL) {
        delete m_entryResultsModel;
    }
    m_entryResultsModel = entryResultsModel;
    Q_EMIT entryResultsModelChanged();
}

void
PlaceEntry::setEntryRendererHints(QMap<QString, QVariant> entryRendererHints)
{
    m_entryRendererHints = entryRendererHints;
    Q_EMIT entryRendererHintsChanged();
}

void
PlaceEntry::setGlobalSearchQuery(QString globalSearchQuery)
{
    if (globalSearchQuery != m_globalSearchQuery) {
        m_globalSearchQuery = globalSearchQuery;
        if (m_dbusIface != NULL) {
            QHash<QString, QString> searchHints;
            m_dbusIface->call("SetGlobalSearch", m_globalSearchQuery, qVariantFromValue(searchHints));
        }
        Q_EMIT globalSearchQueryChanged();
    }
}

void
PlaceEntry::setGlobalRendererName(QString globalRendererName)
{
    if (globalRendererName != m_globalRendererName) {
        m_globalRendererName = globalRendererName;
        Q_EMIT globalRendererNameChanged();
    }
}

void
PlaceEntry::setGlobalGroupsModelName(QString globalGroupsModelName)
{
    if (globalGroupsModelName != m_globalGroupsModelName) {
        m_globalGroupsModelName = globalGroupsModelName;
        delete m_globalGroupsModel;
        m_globalGroupsModel = NULL;
        Q_EMIT globalGroupsModelNameChanged();
        Q_EMIT globalGroupsModelChanged();
    }
}

void
PlaceEntry::setGlobalGroupsModel(DeeListModel* globalGroupsModel)
{
    if (globalGroupsModel == NULL) {
        return;
    }
    if (m_globalGroupsModel != NULL) {
        delete m_globalGroupsModel;
    }
    m_globalGroupsModel = globalGroupsModel;
    Q_EMIT globalGroupsModelChanged();
}

void
PlaceEntry::setGlobalResultsModelName(QString globalResultsModelName)
{
    if (globalResultsModelName != m_globalResultsModelName) {
        m_globalResultsModelName = globalResultsModelName;
        delete m_globalResultsModel;
        m_globalResultsModel = NULL;
        Q_EMIT globalResultsModelNameChanged();
        Q_EMIT globalResultsModelChanged();
    }
}

void
PlaceEntry::setGlobalResultsModel(DeeListModel* globalResultsModel)
{
    if (globalResultsModel == NULL) {
        return;
    }
    if (m_globalResultsModel != NULL) {
        delete m_globalResultsModel;
    }
    m_globalResultsModel = globalResultsModel;
    Q_EMIT globalResultsModelChanged();
}

void
PlaceEntry::setGlobalRendererHints(QMap<QString, QVariant> globalRendererHints)
{
    m_globalRendererHints = globalRendererHints;
    Q_EMIT globalRendererHintsChanged();
}

void
PlaceEntry::connectToDash()
{
    if (m_dashDbusIface != NULL) {
        return;
    }

    m_dashDbusIface = new QDBusInterface(DASH_DBUS_SERVICE, DASH_DBUS_PATH, DASH_DBUS_INTERFACE,
                                         QDBusConnection::sessionBus(), this);
    connect(m_dashDbusIface, SIGNAL(activeChanged(bool)),
            SLOT(slotDashActiveChanged(bool)));
    connect(m_dashDbusIface, SIGNAL(activePlaceEntryChanged(const QString&)),
            SLOT(slotDashActivePlaceEntryChanged(const QString&)));

    m_dashActive = m_dashDbusIface->property("active").toBool();
    m_dashActivePlaceEntry = m_dashDbusIface->property("activePlaceEntry").toString();

    updateActiveState();
}

void
PlaceEntry::slotDashActiveChanged(bool value)
{
    if (m_dashActive != value) {
        m_dashActive = value;
        updateActiveState();
    }
}

void
PlaceEntry::slotDashActivePlaceEntryChanged(const QString& entry)
{
    if (m_dashActivePlaceEntry != entry) {
        m_dashActivePlaceEntry = entry;
        updateActiveState();
    }
}

void
PlaceEntry::updateActiveState()
{
    bool active = false;
    if (m_dashActive && !m_dbusObjectPath.isEmpty() && (m_dashActivePlaceEntry == m_dbusObjectPath)) {
        active = true;
    }

    if (active != m_active) {
        m_active = active;
        Q_EMIT activeChanged();
    }
}

void
PlaceEntry::activate()
{
    activateEntry(0);
}

void
PlaceEntry::activateEntry(const int section)
{
    if (!m_sensitive) {
        startRemotePlaceOnDemand();
    }

    QDBusInterface iface(DASH_DBUS_SERVICE, DASH_DBUS_PATH, DASH_DBUS_INTERFACE);
    iface.asyncCall(QLatin1String("activatePlaceEntry"), m_fileName, m_groupName, section);
}

void
PlaceEntry::startRemotePlaceOnDemand()
{
    /* On-demand startup of the place. */
    Place* place = qobject_cast<Place*>(parent());
    place->connectToRemotePlace();
}

void
PlaceEntry::createMenuActions()
{
    if (!m_sensitive) {
        /* Monitor updates to the list of sections upon starting the remote
           place. */
        startMonitoringSections();

        startRemotePlaceOnDemand();
    }

    if (m_sections == NULL) {
        return;
    }

    for (int i = 0; i < m_sections->rowCount(); ++i) {
        QAction* section = new QAction(m_menu);
        QString sectionName = m_sections->data(m_sections->index(i)).toString();
        /* Escape ampersands so that they are not considered as keyboard accelerators. */
        sectionName.replace("&", "&&");
        section->setText(sectionName);
        section->setProperty(SECTION_PROPERTY, QVariant(i));
        m_menu->addAction(section);
        QObject::connect(section, SIGNAL(triggered()), this, SLOT(onSectionTriggered()));
    }
}

void
PlaceEntry::startMonitoringSections()
{
    connect(this, SIGNAL(sectionsChanged()), SLOT(slotSectionsChanged()));
}

void
PlaceEntry::stopMonitoringSections()
{
    disconnect(SIGNAL(sectionsChanged()), this, SLOT(slotSectionsChanged()));
}

void
PlaceEntry::slotSectionsChanged()
{
    /* This is a one-shot slot. */
    stopMonitoringSections();

    createMenuActions();
}

void
PlaceEntry::onSectionTriggered()
{
    QAction* action = static_cast<QAction*>(sender());
    int section = action->property(SECTION_PROPERTY).toInt();
    m_menu->hide();
    activateEntry(section);
}

void
PlaceEntry::connectToRemotePlaceEntry()
{
    delete m_dbusIface;
    m_dbusIface = new QDBusInterface(m_dbusName, m_dbusObjectPath,
                                     UNITY_PLACE_ENTRY_INTERFACE);
    QDBusConnection connection = m_dbusIface->connection();
    if (!connection.isConnected()) {
        return;
    }

    // Connect to RendererInfoChanged and PlaceEntryInfoChanged signals
    connection.connect(m_dbusName, m_dbusObjectPath, UNITY_PLACE_ENTRY_INTERFACE,
                       "RendererInfoChanged", this,
                       SLOT(onRendererInfoChanged(const RendererInfoStruct&)));
    connection.connect(m_dbusName, m_dbusObjectPath, UNITY_PLACE_ENTRY_INTERFACE,
                       "PlaceEntryInfoChanged", this,
                       SLOT(updateInfo(const PlaceEntryInfoStruct&)));

    /* Update state of D-Bus daemon according to the values of local properties */
    QHash<QString, QString> searchHints;
    m_dbusIface->call("SetSearch", m_entrySearchQuery, qVariantFromValue(searchHints));
    m_dbusIface->call("SetGlobalSearch", m_globalSearchQuery, qVariantFromValue(searchHints));
    m_dbusIface->call("SetActive", m_active);
    m_dbusIface->call("SetActiveSection", m_activeSection);
}

void
PlaceEntry::updateInfo(const PlaceEntryInfoStruct& info)
{
    if (info.name != "") {
        setName(info.name);
    }
    setIcon(info.icon);
    setPosition(info.position);
    setMimetypes(info.mimetypes);
    setSensitive(info.sensitive);
    setSection(info.sections_model);

    QMap<QString, QVariant> hints;
    QMap<QString, QString>::const_iterator i;
    for (i = info.hints.constBegin(); i != info.hints.constEnd(); ++i) {
        hints[i.key()] = QVariant(i.value());
    }
    setHints(hints);

    setEntryRendererName(info.entry_renderer_info.default_renderer);
    setEntryGroupsModelName(info.entry_renderer_info.groups_model);
    setEntryResultsModelName(info.entry_renderer_info.results_model);

    QMap<QString, QVariant> entryRendererHints;
    for (i = info.entry_renderer_info.renderer_hints.constBegin();
        i != info.entry_renderer_info.renderer_hints.constEnd(); ++i) {
        entryRendererHints[i.key()] = QVariant(i.value());
    }
    setEntryRendererHints(entryRendererHints);

    setGlobalRendererName(info.global_renderer_info.default_renderer);
    setGlobalGroupsModelName(info.global_renderer_info.groups_model);
    setGlobalResultsModelName(info.global_renderer_info.results_model);

    QMap<QString, QVariant> globalRendererHints;
    for (i = info.global_renderer_info.renderer_hints.constBegin();
        i != info.global_renderer_info.renderer_hints.constEnd(); ++i) {
        globalRendererHints[i.key()] = QVariant(i.value());
    }
    setGlobalRendererHints(globalRendererHints);

    Q_EMIT updated();
    Q_EMIT rendererInfoChanged();
}

void
PlaceEntry::setSection(const QString& sectionModelName)
{
    DeeListModel* sections = new DeeListModel;
    sections->setName(sectionModelName);
    setSections(sections);
}

void
PlaceEntry::onRendererInfoChanged(const RendererInfoStruct& r)
{
    setEntryRendererName(r.default_renderer);
    setEntryGroupsModelName(r.groups_model);
    setEntryResultsModelName(r.results_model);

    QMap<QString, QVariant> entryRendererHints;
    QMap<QString, QString>::const_iterator i;
    for (i = r.renderer_hints.constBegin();
         i != r.renderer_hints.constEnd(); ++i) {
        entryRendererHints[i.key()] = QVariant(i.value());
    }
    setEntryRendererHints(entryRendererHints);

    Q_EMIT updated();
    Q_EMIT rendererInfoChanged();
}

