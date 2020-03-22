/*
 * ---- Call of Suli ----
 *
 * abstractactivity.h
 *
 * Created on: 2020. 03. 29.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AbstractActivity
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ABSTRACTACTIVITY_H
#define ABSTRACTACTIVITY_H

#include <QObject>
#include <QQuickItem>
#include <QJsonObject>
#include <QJsonArray>
#include <QtConcurrent/QtConcurrent>
#include "cosclient.h"
#include "cosdb.h"
#include "cosmessage.h"
#include "cosdownloader.h"
#include "variantmapmodel.h"

class Client;


class AbstractActivity : public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY(Client* client READ client WRITE setClient NOTIFY clientChanged)
	Q_PROPERTY(bool isBusy READ isBusy WRITE setIsBusy NOTIFY isBusyChanged)
	Q_PROPERTY(CosDownloader * downloader READ downloader WRITE setDownloader NOTIFY downloaderChanged)
	Q_PROPERTY(int canUndo READ canUndo NOTIFY canUndoChanged)
	Q_PROPERTY(QString canUndoString READ canUndoString WRITE setCanUndoString NOTIFY canUndoStringChanged)
	Q_PROPERTY(CosDb* db MEMBER m_db NOTIFY dbChanged)
	Q_PROPERTY(bool removeDatabase READ removeDatabase WRITE setRemoveDatabase NOTIFY removeDatabaseChanged)


public:
	struct busyData;

	explicit AbstractActivity(const CosMessage::CosClass &defaultClass = CosMessage::ClassInvalid, QQuickItem *parent = nullptr);
	virtual ~AbstractActivity();

	Client* client() const { return m_client; }
	bool isBusy() const { return m_isBusy; }
	CosDb* db() const { return m_db; }
	CosDownloader * downloader() const { return m_downloader; }

	Q_INVOKABLE
	template <typename Class>
	void run(void (Class::*func)(QVariantMap), QVariantMap data) {
		QtConcurrent::run((Class *)(this), func, data);
	}

	Q_INVOKABLE VariantMapModel *newModel(const QStringList &list) {
		return VariantMapModel::newModel(list, this->parentItem());
	}

public slots:
	void send(const CosMessage::CosClass &cosClass, const QString &cosFunc,
			  const QJsonObject &jsonData = QJsonObject(), const QByteArray &binaryData = QByteArray());
	void send(const QString &cosFunc,
			  const QJsonObject &jsonData = QJsonObject(), const QByteArray &binaryData = QByteArray()) {
		send(m_defaultClass, cosFunc, jsonData, binaryData);
	}
	void setClient(Client* client);
	void setIsBusy(bool isBusy);
	void busyStackAdd(const CosMessage::CosClass &cosClass, const QString &cosFunc, const int &msgId, QObject *otherObject = nullptr);
	void busyStackRemove(const CosMessage::CosClass &cosClass, const QString &cosFunc, const int &msgId, QObject *otherObject = nullptr);
	void setDownloader(CosDownloader * downloader);
	void setCanUndoString(QString canUndoString);

	int canUndo() const;

	void addDb(CosDb *db, const bool &removeDatabase = false);
	void setRemoveDatabase(bool removeDatabase);
	void setDefaultClass(const CosMessage::CosClass &defaultClass);

protected slots:
	virtual void onMessageReceived(const CosMessage &message) { autoSignalEmit(message); }
	virtual void onMessageFrameReceived(const CosMessage &) {}
	virtual void clientSetup() {}

private slots:
	void onMessageReceivedPrivate(const CosMessage &message);
	void onSocketDisconnected();
	void autoSignalEmit(const CosMessage &message);

signals:
	void clientChanged(Client* client);
	void isBusyChanged(bool isBusy);
	void socketDisconnected();
	void downloaderChanged(CosDownloader * downloader);
	void canUndoChanged(int canUndo);
	void canUndoStringChanged(QString canUndoString);
	void dbChanged(CosDb *db);
	void removeDatabaseChanged(bool removeDatabase);

protected:
	Client* m_client;
	bool m_isBusy;
	QList<busyData> m_busyStack;
	CosDownloader *m_downloader;
	QStringList m_ignoredSignals;

private:
	CosDb* m_db;
	int m_runId;
	QString m_canUndoString;
	bool m_removeDatabase;
	CosMessage::CosClass m_defaultClass;

public:

	/**
	 * @brief The busyData struct
	 */

	struct busyData {
		CosMessage::CosClass m_cosClass;
		QString m_cosFunc;
		int m_msgId;
		QObject *m_otherObject;

		busyData(const CosMessage::CosClass &cosClass, const QString &cosFunc, const int &msgid, QObject *other)
			: m_cosClass(cosClass)
			, m_cosFunc(cosFunc)
			, m_msgId(msgid)
			, m_otherObject(other)
		{ }

		friend inline bool operator== (const busyData &b1, const busyData &b2) {
			return b1.m_cosClass == b2.m_cosClass
					&& b1.m_cosFunc == b2.m_cosFunc
					&& b1.m_msgId == b2.m_msgId
					&& b1.m_otherObject == b2.m_otherObject;
		}
	};
	QString canUndoString() const
	{
		return m_canUndoString;
	}
	bool removeDatabase() const
	{
		return m_removeDatabase;
	}
};


#endif // ABSTRACTACTIVITY_H
