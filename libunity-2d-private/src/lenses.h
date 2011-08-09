/*
 * Copyright (C) 2011 Canonical, Ltd.
 *
 * Authors:
 *  Florian Boucault <florian.boucault@canonical.com>
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

#ifndef LENSES_H
#define LENSES_H

// Qt
#include <QAbstractListModel>

// libunity-core
#include <UnityCore/Lens.h>

namespace unity
{
namespace dash
{
class Lenses;
}
}

class Lenses : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit Lenses(QObject *parent = 0);
    ~Lenses();

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    Q_INVOKABLE int rowCount(const QModelIndex& parent = QModelIndex()) const;

    Q_INVOKABLE QVariant get(int row) const;
    Q_INVOKABLE QVariant get(const QString& lens_id) const;

private Q_SLOTS:
    void onLensAdded(unity::dash::Lens::Ptr& lens);

private:
    unity::dash::Lenses* m_unityLenses;
};

#endif // LENSES_H
