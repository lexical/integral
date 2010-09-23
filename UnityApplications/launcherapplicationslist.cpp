#include "launcherapplication.h"
#include "launcherapplicationslist.h"

#include "bamf-matcher.h"
#include "bamf-application.h"

#include <QStringList>

#define FAVORITES_KEY QString("/desktop/unity/launcher/favorites/")

LauncherApplicationsList::LauncherApplicationsList(QObject *parent) :
    QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles[0] = "application";
    setRoleNames(roles);

    load();
}

LauncherApplicationsList::~LauncherApplicationsList()
{
    QList<QLauncherApplication*>::iterator iter;
    for(iter=m_applications.begin(); iter!=m_applications.end(); iter++)
    {
        delete *iter;
    }
}

QString
LauncherApplicationsList::desktopFilePathFromFavorite(QString favorite_id)
{
    GConfItemQmlWrapper favorite;
    favorite.setKey(FAVORITES_KEY + favorite_id + "/desktop_file");
    return favorite.getValue().toString();
}

void
LauncherApplicationsList::load()
{
    /* FIXME: applications should be sorted depending on their priority */

    /* Assume m_applications is empty */

    m_favorites_list = new GConfItemQmlWrapper;
    m_favorites_list->setKey(FAVORITES_KEY + "favorites_list");
    QStringList favorites = m_favorites_list->getValue().toStringList();

    for(QStringList::iterator iter=favorites.begin(); iter!=favorites.end(); iter++)
        *iter = desktopFilePathFromFavorite(*iter);

    BamfMatcher& matcher = BamfMatcher::get_default();
    BamfApplicationList* running_applications = matcher.running_applications();
    BamfApplication* bamf_application;
    QLauncherApplication* application;

    /* Insert running applications from Bamf */
    for(int i=0; i<running_applications->size(); i++)
    {
        bamf_application = running_applications->at(i);
        if(!bamf_application->user_visible())
            continue;

        int favorite = favorites.removeAll(bamf_application->desktop_file());
        application = new QLauncherApplication;
        application->setBamfApplication(bamf_application);
        application->setSticky(favorite > 0);
        m_applications.append(application);
    }

    /* Insert remaining favorites that are not running */
    for(QStringList::iterator iter=favorites.begin(); iter!=favorites.end(); iter++)
    {
        application = new QLauncherApplication;
        application->setDesktopFile(*iter);
        application->setSticky(true);
        m_applications.append(application);
    }

    QObject::connect(m_favorites_list, SIGNAL(valueChanged()), SLOT(onFavoritesListChanged()));
    // FIXME: watch all the QLauncherApplications’ stickyChanged signal and
    // update the gconf entries accordingly
    QObject::connect(&matcher, SIGNAL(ViewOpened(BamfView*)), SLOT(onBamfViewOpened(BamfView*)));
}

void
LauncherApplicationsList::onFavoritesListChanged()
{
    QStringList favorites = m_favorites_list->getValue().toStringList();

    /* 1/ Find out if the sticky status changed for the applications currently
       displayed in the launcher. */
    for (int i = 0; i < m_applications.size(); ++i)
    {
        QLauncherApplication* application = m_applications.at(i);
        bool sticky = false;
        for (int j = 0; j < favorites.size(); ++j)
        {
            QString desktop_file = desktopFilePathFromFavorite(favorites.at(j));
            if (desktop_file == application->desktop_file())
            {
                sticky = true;
                break;
            }
        }
        if (sticky != application->sticky())
        {
            application->setSticky(sticky);
            if (!sticky && !application->running())
            {
                int index = m_applications.indexOf(application);
                beginRemoveRows(QModelIndex(), index, index);
                m_applications.removeAt(index);
                endRemoveRows();
            }
        }
    }

    /* 2/ Find out whether new favorites that are not currently displayed in
       the launcher have been added. */
    for (int i = 0; i < favorites.size(); ++i)
    {
        QString desktop_file = desktopFilePathFromFavorite(favorites.at(i));
        bool in = false;
        for (int j = 0; j < m_applications.size(); ++j)
        {
            QLauncherApplication* application = m_applications.at(j);
            if (desktop_file == application->desktop_file())
            {
                in = true;
                break;
            }
        }
        if (!in)
        {
            /* New favorite */
            QLauncherApplication* application = new QLauncherApplication;
            application->setDesktopFile(desktop_file);
            application->setSticky(true);
            /* FIXME: it should probably be inserted at the right position,
                      not just appended. */
            int index = m_applications.size();
            beginInsertRows(QModelIndex(), index, index);
            m_applications.append(application);
            endInsertRows();
        }
    }
}

void LauncherApplicationsList::insertBamfApplication(BamfApplication* bamf_application)
{
    if(!bamf_application->user_visible())
        return;

    QLauncherApplication* application;
    QList<QLauncherApplication*>::iterator iter;
    for(iter=m_applications.begin(); iter!=m_applications.end(); iter++)
    {
        application = *iter;
        if(application->desktop_file() == bamf_application->desktop_file())
        {
            /* There is an already existing QLauncherApplication corresponding
               to bamf_application */
            application->setBamfApplication(bamf_application);
            return;
        }
    }

    /* Create a new QLauncherApplication for bamf_application */
    application = new QLauncherApplication;
    application->setBamfApplication(bamf_application);

    beginInsertRows(QModelIndex(), m_applications.size(), m_applications.size());
    m_applications.append(application);
    endInsertRows();

    QObject::connect(application, SIGNAL(closed()), this, SLOT(onApplicationClosed()));
}

void LauncherApplicationsList::onApplicationClosed()
{
    QLauncherApplication* application = static_cast<QLauncherApplication*>(sender());
    int index = m_applications.indexOf(application);

    beginRemoveRows(QModelIndex(), index, index);
    m_applications.removeAt(index);
    endRemoveRows();

    delete application;
}

void
LauncherApplicationsList::onBamfViewOpened(BamfView* bamf_view)
{
    /* Make sure bamf_view is in fact a BamfApplication */
    BamfApplication* bamf_application;
    bamf_application = dynamic_cast<BamfApplication*>(bamf_view);

    if(bamf_application == NULL)
        return;

    insertBamfApplication(bamf_application);
}

int
LauncherApplicationsList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_applications.size();
}

QVariant
LauncherApplicationsList::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role);

    if (!index.isValid())
        return QVariant();

    return QVariant::fromValue(m_applications.at(index.row()));
}
