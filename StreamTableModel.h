/**
 * discover - a real-time monitoring and analysis tool for IEC 61850-9-2LE Sampled Values
 *
 * Copyright (c) 2013 Steven Blair
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef STREAMTABLEMODEL_H
#define STREAMTABLEMODEL_H

#include <QAbstractTableModel>
#include <QMap>
#include <QItemSelection>
#include <QItemSelectionModel>
#include "Stream.h"

#define STREAM_TABLE_NUMBER_OF_COLUMNS                  10

#define STREAM_TABLE_COLUMNS_STATUS                     0
#define STREAM_TABLE_COLUMNS_SVID                       1
#define STREAM_TABLE_COLUMNS_SOURCE_MAC                 2
#define STREAM_TABLE_COLUMNS_SAMPLES_PER_CYCLE          3
#define STREAM_TABLE_COLUMNS_FREQ                       4
#define STREAM_TABLE_COLUMNS_VOLTAGE                    5
#define STREAM_TABLE_COLUMNS_CURRENT                    6
#define STREAM_TABLE_COLUMNS_POWER                      7
#define STREAM_TABLE_COLUMNS_THD                        8
#define STREAM_TABLE_COLUMNS_CURRENT_THD                9

#define STREAM_TABLE_COLUMNS_STATUS_TITLE               "Status"
#define STREAM_TABLE_COLUMNS_SVID_TITLE                 "svID"
#define STREAM_TABLE_COLUMNS_SOURCE_MAC_TITLE           "Source MAC Address"
#define STREAM_TABLE_COLUMNS_SAMPLES_PER_CYCLE_TITLE    "Samples/cycle"
#define STREAM_TABLE_COLUMNS_FREQ_TITLE                 "Frequency"
#define STREAM_TABLE_COLUMNS_VOLTAGE_TITLE              "Voltage (RMS, L-L)"
#define STREAM_TABLE_COLUMNS_CURRENT_TITLE              "Current (RMS)"
#define STREAM_TABLE_COLUMNS_POWER_TITLE                "Power (3-phase)"
#define STREAM_TABLE_COLUMNS_THD_TITLE                  "Voltage THD"
#define STREAM_TABLE_COLUMNS_CURRENT_THD_TITLE          "Current THD"


class StreamTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit StreamTableModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    Stream *getPhasorData(QString svID);
    StreamTableRow *getRowFromIndex(QPersistentModelIndex *index);

signals:
    void resizeColumnsToContents();
    void streamSelected(Stream* stream);
    void streamTableEmpty();                // inform comms thread to begin receiving on new network interface
    void setGraphicsViewRow(StreamTableRow *row);
    
public slots:
    void networkInterfaceStopped();
    void setStreamTableRow(StreamTableRow *row);

private:
    QMap<QString, QPointer<StreamTableRow> > rows;
    bool blockUpdates;

    int getIndexFromKey(QString key);
    
};

#endif // STREAMTABLEMODEL_H
