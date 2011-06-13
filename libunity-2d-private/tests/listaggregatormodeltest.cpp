/*
 * Copyright (C) 2011 Canonical, Ltd.
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

// local
#include <listaggregatormodel.h>

// Qt
#include <QTest>
#include <QStringListModel>
#include <QSignalSpy>
#include <QModelIndex>

class ListAggregatorModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testRoleNames()
    {
        ListAggregatorModel model;
        const QHash<int, QByteArray> roleNames = model.roleNames();
        QCOMPARE(roleNames.size(), 1);
        QVERIFY(roleNames.contains(0));
        QCOMPARE(roleNames[0], QByteArray("item"));
    }

    void testAppendModelWrongType()
    {
        ListAggregatorModel model;
        QVERIFY(model.m_models.isEmpty());
        QTest::ignoreMessage(QtWarningMsg, "void ListAggregatorModel::appendModel(const QVariant&): Unable to append a model that is not of type QAbstractListModel. Invalid model. ");
        model.appendModel(QVariant());
        QVERIFY(model.m_models.isEmpty());
        QTest::ignoreMessage(QtWarningMsg, "void ListAggregatorModel::appendModel(const QVariant&): Unable to append a model that is not of type QAbstractListModel. QVariant(bool, true) is of type bool ");
        model.appendModel(QVariant(true));
        QVERIFY(model.m_models.isEmpty());
        QTest::ignoreMessage(QtWarningMsg, "void ListAggregatorModel::appendModel(const QVariant&): Unable to append a model that is not of type QAbstractListModel. \"\" is of type QObject ");
        model.appendModel(QVariant::fromValue(new QObject));
        QVERIFY(model.m_models.isEmpty());
    }

    void testAggregateListModel()
    {
        ListAggregatorModel model;
        QVERIFY(model.m_models.isEmpty());
        QCOMPARE(model.rowCount(), 0);

        qRegisterMetaType<QModelIndex>("QModelIndex");
        QSignalSpy spyOnRowsAboutToBeInserted(&model, SIGNAL(rowsAboutToBeInserted(const QModelIndex&, int, int)));
        QSignalSpy spyOnRowsInserted(&model, SIGNAL(rowsInserted(const QModelIndex&, int, int)));
        QList<QVariant> signal;

        QStringListModel list1(QStringList() << "aa" << "ab" << "ac");
        model.aggregateListModel(&list1);
        QCOMPARE(model.m_models.size(), 1);
        QCOMPARE(qobject_cast<QStringListModel*>(model.m_models[0]), &list1);
        QCOMPARE(model.rowCount(), 3);
        QCOMPARE(spyOnRowsAboutToBeInserted.count(), 1);
        signal = spyOnRowsAboutToBeInserted.takeFirst();
        QCOMPARE(signal[1].toInt(), 0);
        QCOMPARE(signal[2].toInt(), 2);
        QCOMPARE(spyOnRowsInserted.count(), 1);
        signal = spyOnRowsInserted.takeFirst();
        QCOMPARE(signal[1].toInt(), 0);
        QCOMPARE(signal[2].toInt(), 2);

        QStringListModel list2(QStringList() << "ba" << "bb" << "bc" << "bd");
        model.aggregateListModel(&list2);
        QCOMPARE(model.m_models.size(), 2);
        QCOMPARE(qobject_cast<QStringListModel*>(model.m_models[1]), &list2);
        QCOMPARE(model.rowCount(), 7);
        QCOMPARE(spyOnRowsAboutToBeInserted.count(), 1);
        signal = spyOnRowsAboutToBeInserted.takeFirst();
        QCOMPARE(signal[1].toInt(), 3);
        QCOMPARE(signal[2].toInt(), 6);
        QCOMPARE(spyOnRowsInserted.count(), 1);
        signal = spyOnRowsInserted.takeFirst();
        QCOMPARE(signal[1].toInt(), 3);
        QCOMPARE(signal[2].toInt(), 6);
    }

    void testRemoveListModel()
    {
        ListAggregatorModel model;

        qRegisterMetaType<QModelIndex>("QModelIndex");
        QSignalSpy spyOnRowsAboutToBeRemoved(&model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)));
        QSignalSpy spyOnRowsRemoved(&model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)));
        QList<QVariant> signal;

        QStringListModel list1(QStringList() << "aa" << "ab" << "ac");
        model.aggregateListModel(&list1);
        QStringListModel list2(QStringList() << "ba" << "bb" << "bc" << "bd");
        model.aggregateListModel(&list2);
        QStringListModel list3(QStringList() << "ca" << "cb");
        model.aggregateListModel(&list3);

        model.removeListModel(&list2);
        QCOMPARE(model.m_models.size(), 2);
        QCOMPARE(qobject_cast<QStringListModel*>(model.m_models[0]), &list1);
        QCOMPARE(qobject_cast<QStringListModel*>(model.m_models[1]), &list3);
        QCOMPARE(model.rowCount(), 5);
        QCOMPARE(spyOnRowsAboutToBeRemoved.count(), 1);
        signal = spyOnRowsAboutToBeRemoved.takeFirst();
        QCOMPARE(signal[1].toInt(), 3);
        QCOMPARE(signal[2].toInt(), 6);
        QCOMPARE(spyOnRowsRemoved.count(), 1);
        signal = spyOnRowsRemoved.takeFirst();
        QCOMPARE(signal[1].toInt(), 3);
        QCOMPARE(signal[2].toInt(), 6);
    }

    void testData()
    {
        ListAggregatorModel model;
        model.aggregateListModel(new QStringListModel(QStringList() << "aa" << "ab" << "ac", &model));
        model.aggregateListModel(new QStringListModel(QStringList() << "ba" << "bb" << "bc" << "bd", &model));
        model.aggregateListModel(new QStringListModel(QStringList() << "ca" << "cb", &model));
        QStringList data = QStringList() << "aa" << "ab" << "ac" << "ba" << "bb" << "bc" << "bd" << "ca" << "cb";
        for (int i = 0; i < model.rowCount(); ++i) {
            QCOMPARE(model.data(model.index(i)).toString(), data[i]);
        }
    }

    void testGet()
    {
        ListAggregatorModel model;
        model.aggregateListModel(new QStringListModel(QStringList() << "aa" << "ab" << "ac", &model));
        model.aggregateListModel(new QStringListModel(QStringList() << "ba" << "bb" << "bc" << "bd", &model));
        model.aggregateListModel(new QStringListModel(QStringList() << "ca" << "cb", &model));
        QStringList data = QStringList() << "aa" << "ab" << "ac" << "ba" << "bb" << "bc" << "bd" << "ca" << "cb";
        for (int i = 0; i < model.rowCount(); ++i) {
            QCOMPARE(model.get(i).toString(), data[i]);
        }
    }

    void testComputeOffset()
    {
        ListAggregatorModel model;
        QStringListModel list1(QStringList() << "aa" << "ab" << "ac");
        model.aggregateListModel(&list1);
        QStringListModel list2(QStringList() << "ba" << "bb" << "bc" << "bd");
        model.aggregateListModel(&list2);
        QStringListModel list3(QStringList() << "ca" << "cb");
        model.aggregateListModel(&list3);

        QCOMPARE(model.computeOffset(&list1), 0);
        QCOMPARE(model.computeOffset(&list2), 3);
        QCOMPARE(model.computeOffset(&list3), 7);
    }

    void testModelAtIndex()
    {
        ListAggregatorModel model;
        QStringListModel list1(QStringList() << "aa" << "ab" << "ac");
        model.aggregateListModel(&list1);
        QStringListModel list2(QStringList() << "ba" << "bb" << "bc" << "bd");
        model.aggregateListModel(&list2);
        QStringListModel list3(QStringList() << "ca" << "cb");
        model.aggregateListModel(&list3);

        QCOMPARE(model.modelAtIndex(0), &list1);
        QCOMPARE(model.modelAtIndex(1), &list1);
        QCOMPARE(model.modelAtIndex(2), &list1);
        QCOMPARE(model.modelAtIndex(3), &list2);
        QCOMPARE(model.modelAtIndex(4), &list2);
        QCOMPARE(model.modelAtIndex(5), &list2);
        QCOMPARE(model.modelAtIndex(6), &list2);
        QCOMPARE(model.modelAtIndex(7), &list3);
        QCOMPARE(model.modelAtIndex(8), &list3);
    }

    void testRemoveRows()
    {
        ListAggregatorModel model;

        qRegisterMetaType<QModelIndex>("QModelIndex");
        QSignalSpy spyOnRowsAboutToBeRemoved(&model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)));
        QSignalSpy spyOnRowsRemoved(&model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)));
        QList<QVariant> signal;

        model.aggregateListModel(new QStringListModel(QStringList() << "aa" << "ab" << "ac", &model));
        model.aggregateListModel(new QStringListModel(QStringList() << "ba" << "bb" << "bc" << "bd", &model));
        model.aggregateListModel(new QStringListModel(QStringList() << "ca" << "cb", &model));
        QStringList data;

        // Invalid boundaries, doesn’t remove anything.
        QVERIFY(!model.removeRows(-1, 2));
        QCOMPARE(model.rowCount(), 9);
        QCOMPARE(spyOnRowsAboutToBeRemoved.count(), 0);
        QCOMPARE(spyOnRowsRemoved.count(), 0);
        QVERIFY(!model.removeRows(10, 2));
        QCOMPARE(model.rowCount(), 9);
        QCOMPARE(spyOnRowsAboutToBeRemoved.count(), 0);
        QCOMPARE(spyOnRowsRemoved.count(), 0);
        QVERIFY(!model.removeRows(2, 0));
        QCOMPARE(model.rowCount(), 9);
        QCOMPARE(spyOnRowsAboutToBeRemoved.count(), 0);
        QCOMPARE(spyOnRowsRemoved.count(), 0);

        // Remove two rows from the beginning, only modifies the first model.
        QVERIFY(model.removeRows(0, 2));
        QCOMPARE(model.rowCount(), 7);
        QCOMPARE(spyOnRowsAboutToBeRemoved.count(), 1);
        signal = spyOnRowsAboutToBeRemoved.takeFirst();
        QCOMPARE(signal[1].toInt(), 0);
        QCOMPARE(signal[2].toInt(), 1);
        QCOMPARE(spyOnRowsRemoved.count(), 1);
        signal = spyOnRowsRemoved.takeFirst();
        QCOMPARE(signal[1].toInt(), 0);
        QCOMPARE(signal[2].toInt(), 1);
        data = QStringList() << "ac" << "ba" << "bb" << "bc" << "bd" << "ca" << "cb";
        for (int i = 0; i < model.rowCount(); ++i) {
            QCOMPARE(model.get(i).toString(), data[i]);
        }

        // Request to remove three rows starting from the last index,
        // only one row is actually removed.
        QVERIFY(model.removeRows(6, 3));
        QCOMPARE(model.rowCount(), 6);
        QCOMPARE(spyOnRowsAboutToBeRemoved.count(), 1);
        signal = spyOnRowsAboutToBeRemoved.takeFirst();
        QCOMPARE(signal[1].toInt(), 6);
        QCOMPARE(signal[2].toInt(), 6);
        QCOMPARE(spyOnRowsRemoved.count(), 1);
        signal = spyOnRowsRemoved.takeFirst();
        QCOMPARE(signal[1].toInt(), 6);
        QCOMPARE(signal[2].toInt(), 6);
        data = QStringList() << "ac" << "ba" << "bb" << "bc" << "bd" << "ca";
        for (int i = 0; i < model.rowCount(); ++i) {
            QCOMPARE(model.get(i).toString(), data[i]);
        }

        // Remove rows that span two models.
        QVERIFY(model.removeRows(3, 3));
        QCOMPARE(model.rowCount(), 3);
        QCOMPARE(spyOnRowsAboutToBeRemoved.count(), 2);
        signal = spyOnRowsAboutToBeRemoved.takeFirst();
        QCOMPARE(signal[1].toInt(), 3);
        QCOMPARE(signal[2].toInt(), 4);
        signal = spyOnRowsAboutToBeRemoved.takeFirst();
        QCOMPARE(signal[1].toInt(), 3);
        QCOMPARE(signal[2].toInt(), 3);
        QCOMPARE(spyOnRowsRemoved.count(), 2);
        signal = spyOnRowsRemoved.takeFirst();
        QCOMPARE(signal[1].toInt(), 3);
        QCOMPARE(signal[2].toInt(), 4);
        signal = spyOnRowsRemoved.takeFirst();
        QCOMPARE(signal[1].toInt(), 3);
        QCOMPARE(signal[2].toInt(), 3);
        data = QStringList() << "ac" << "ba" << "bb";
        for (int i = 0; i < model.rowCount(); ++i) {
            QCOMPARE(model.get(i).toString(), data[i]);
        }
    }
};

QTEST_MAIN(ListAggregatorModelTest)

#include "listaggregatormodeltest.moc"

