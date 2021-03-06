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

#include "CommsThread.h"

CommsThread *commsThreadPtr;

void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data) {
    gse_sv_packet_filter((unsigned char *) pkt_data, header->len);
}

void SV_callback(CTYPE_INT16U smpCnt) {
    commsThreadPtr->proxyPacketReceived();
}

CommsThread::CommsThread(QObject *parent) : QThread(parent)
{
    commsThreadPtr = this;
    interfaceNumber = 0;
    scheduledNewInterface = false;
    modelReady = false;
}

void CommsThread::proxyPacketReceived() {
    if (scheduledNewInterface == false) {
        streamManager.addStreamData(QString((const char*) LE_IED_RECV.S1.MUnn.IEC_61850_9_2LETCTR_1.sv_inputs_MSVCB01.svID),
                                    macFromChar(LE_IED_RECV.S1.MUnn.IEC_61850_9_2LETCTR_1.sv_inputs_MSVCB01.sourceMac),
                                    &LE_IED_RECV.S1.MUnn.IEC_61850_9_2LETCTR_1.sv_inputs_MSVCB01.LE_IED_MUnn_PhsMeas1,
                                    LE_IED_RECV.S1.MUnn.IEC_61850_9_2LETCTR_1.sv_inputs_MSVCB01.smpCnt);
    }
    else {
        qDebug() << "blocking addStreamData()";
    }
}

void CommsThread::setNetworkInterface(int value) {
    if (value != interfaceNumber && scheduledNewInterface == false) {
        //qDebug() << "got signal from UI";
        scheduledNewInterface = true;
        interfaceNumber = value;

        if (fp != NULL) {
            pcap_breakloop(fp);
            pcap_close(fp);
        }

        streamManager.removeAll();

        if (interfaceTimeout == NULL) {
            interfaceTimeout = new QTimer(this);
            interfaceTimeout->setInterval(NETWORK_INTERFACE_OFF_DELAY);
            interfaceTimeout->setSingleShot(true);
            connect(interfaceTimeout, SIGNAL(timeout()), SLOT(timerDone()));
        }

        if (interfaceTimeout->isActive()) {
            interfaceTimeout->setInterval(NETWORK_INTERFACE_OFF_DELAY);
            qDebug() << interfaceTimeout;
        }
        else {
            interfaceTimeout->start();
        }

        // TODO still not correct: "left over" data in table
        //QTimer::singleShot(NETWORK_INTERFACE_OFF_DELAY, this, SLOT(timerDone()));   // allow time for network interface to stop
    }
}

void CommsThread::timerDone()
{
    emit networkInterfaceStopped();
}

void CommsThread::startNetworkInterface()
{
    modelReady = true;
}

QString CommsThread::macFromChar(unsigned char *mac)
{
    return QString("%1-%2-%3-%4-%5-%6")
            .arg(QString::number(mac[0], 16), 2, QChar::fromLatin1('0'))
            .arg(QString::number(mac[1], 16), 2, QChar::fromLatin1('0'))
            .arg(QString::number(mac[2], 16), 2, QChar::fromLatin1('0'))
            .arg(QString::number(mac[3], 16), 2, QChar::fromLatin1('0'))
            .arg(QString::number(mac[4], 16), 2, QChar::fromLatin1('0'))
            .arg(QString::number(mac[5], 16), 2, QChar::fromLatin1('0'));
}

void CommsThread::findNetworkInterfaces() {
    pcap_if_t *list_if;
    int interfaceCount = 0;

#ifdef _WIN32
    if (pcap_findalldevs_ex((char *) PCAP_SRC_IF_STRING, NULL /* auth is not needed */, &list_if, CommsThread::errbuf) == -1) {
        fprintf(stderr, "Error in pcap_findalldevs_ex: %s\n", CommsThread::errbuf);
        exit(1);
    }
#else
    if (pcap_findalldevs(&list_if, CommsThread::errbuf) == -1) {
        fprintf(stderr, "Error in pcap_findalldevs: %s\n", CommsThread::errbuf);
        exit(1);
    }
#endif

    while (list_if != NULL) {
#ifdef _WIN32
        emit addInterface(interfaceCount, QString(list_if->description));
#else
        emit addInterface(interfaceCount, QString(list_if->name));
#endif
        list_if = list_if->next;
        interfaceCount++;
    }
}

pcap_t *CommsThread::initWinpcap(int interfaceNumber) {
    pcap_t *fpl;
    pcap_if_t *alldevs;
    pcap_if_t *used_if;
    pcap_if_t *list_if;
    int interfaceCount = 0;

#ifdef _WIN32
    if (pcap_findalldevs_ex((char *) PCAP_SRC_IF_STRING, NULL /* auth is not needed */, &alldevs, CommsThread::errbuf) == -1) {
        fprintf(stderr, "Error in pcap_findalldevs_ex: %s\n", CommsThread::errbuf);
        exit(1);
    }
#else
    if (pcap_findalldevs(&alldevs, CommsThread::errbuf) == -1) {
        fprintf(stderr, "Error in pcap_findalldevs: %s\n", CommsThread::errbuf);
        exit(1);
    }
#endif

    // list all interfaces
    list_if = alldevs;
    used_if = list_if;  // default to first interface

    while (list_if != NULL) {
        if (interfaceCount == interfaceNumber) {
            used_if = list_if;
            break;
        }

        list_if = list_if->next;
        interfaceCount++;
    }

    //fprintf(stdout, "%s\n", /*interfaceName.toLocal8Bit().data()*/used_if->description);
    //fflush(stdout);

    if ((fpl = pcap_open_live(used_if->name,    // name of the device
                             65536,             // portion of the packet to capture. It doesn't matter in this case
                             1,                 // promiscuous mode (nonzero means promiscuous)
                             1,                 // read timeout
                             errbuf             // error buffer
                             )) == NULL)
    {
        fprintf(stderr, "\nUnable to open the adapter. %s is not supported by WinPcap\n", alldevs->name);
        exit(2);
    }

    pcap_freealldevs(alldevs);

    //pcap_setnonblock(fpl, 1, errbuf);

    return fpl;
}

void CommsThread::run()
{
    initialise_iec61850();
    LE_IED_RECV.S1.MUnn.IEC_61850_9_2LETCTR_1.sv_inputs_MSVCB01.datasetDecodeDone = &SV_callback;

    findNetworkInterfaces();

    fp = initWinpcap(interfaceNumber);
    //pcap_setmintocopy(fp, 1);

    forever {
        if (fp != NULL && scheduledNewInterface == false) {
            pcap_dispatch(fp, 32, packet_handler, NULL);
        }

        QCoreApplication::processEvents();

        if (scheduledNewInterface == true && modelReady == true) {
            qDebug() << "restarting pcap";

            scheduledNewInterface = false;
            modelReady = false;

            //pcap_close(fp);
            fp = initWinpcap(interfaceNumber);
        }
    }
}
