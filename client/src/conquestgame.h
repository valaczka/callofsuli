/*
 * ---- Call of Suli ----
 *
 * conquestgame.h
 *
 * Created on: 2024. 01. 21.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ConquestGame
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

#ifndef CONQUESTGAME_H
#define CONQUESTGAME_H

#include "abstractgame.h"
#include "conquestconfig.h"
#include "conquestlanddata.h"
#include "qslistmodel.h"
#include "studentmaphandler.h"


/**
 * @brief The ConquestGame class
 */

class ConquestGame : public AbstractGame
{
	Q_OBJECT

	Q_PROPERTY(ConquestConfig config READ config WRITE setConfig NOTIFY configChanged FINAL)
	Q_PROPERTY(HostMode hostMode READ hostMode NOTIFY hostModeChanged FINAL)
	Q_PROPERTY(int engineId READ engineId WRITE setEngineId NOTIFY engineIdChanged FINAL)
	Q_PROPERTY(int playerId READ playerId WRITE setPlayerId NOTIFY playerIdChanged FINAL)
	Q_PROPERTY(ConquestLandDataList *landDataList READ landDataList CONSTANT FINAL)
	Q_PROPERTY(QSize worldSize READ worldSize WRITE setWorldSize NOTIFY worldSizeChanged FINAL)
	Q_PROPERTY(QSListModel *engineModel READ engineModel CONSTANT FINAL)

public:
	explicit ConquestGame(Client *client);
	virtual ~ConquestGame();

	enum HostMode {
		ModeGuest = 0,
		ModeHost
	};

	Q_ENUM(HostMode);

	Q_INVOKABLE void sendWebSocketMessage(const QJsonValue &data = {});
	Q_INVOKABLE void getEngineList();
	Q_INVOKABLE void gameCreate();

	virtual void gameAbort() override;

	StudentMapHandler* handler() const;
	void setHandler(StudentMapHandler *newHandler);

	ConquestConfig config() const;
	void setConfig(const ConquestConfig &newConfig);

	HostMode hostMode() const;
	void setHostMode(const HostMode &newHostMode);

	int engineId() const;
	void setEngineId(int newEngineId);

	int playerId() const;
	void setPlayerId(int newPlayerId);

	ConquestLandDataList* landDataList() const;

	QSize worldSize() const;
	void setWorldSize(const QSize &newWorldSize);

	QSListModel* engineModel() const;

signals:
	void configChanged();
	void hostModeChanged();
	void engineIdChanged();
	void playerIdChanged();
	void worldSizeChanged();

protected:
	virtual QQuickItem* loadPage() override;
	virtual void timerEvent(QTimerEvent *) override;
	virtual void connectGameQuestion() override;
	virtual bool gameStartEvent() override;
	virtual bool gameFinishEvent() override;

private:
	void onTimeSyncTimerTimeout();
	void onWebSocketActiveChanged();
	void onJsonReceived(const QString &operation, const QJsonValue &data);
	void onConfigChanged();

	void cmdList(const QJsonObject &data);
	void cmdState(const QJsonObject &data);
	void cmdCreate(const QJsonObject &data);
	void cmdConnect(const QJsonObject &data);
	void cmdStart(const QJsonObject &data);
	void cmdPrepare(const QJsonObject &data);
	void cmdQuestionRequest(const QJsonObject &data);

	void cmdTest(const QJsonObject &data);	///

	void reloadLandList();
	ConquestWordListHelper getWorldList() const;

	QTimer m_timeSyncTimer;
	TickTimer m_tickTimer;
	QPointer<StudentMapHandler> m_handler;
	ConquestConfig m_config;
	HostMode m_hostMode = ModeGuest;
	int m_engineId = -1;
	int m_playerId = -1;
	std::unique_ptr<ConquestLandDataList> m_landDataList;
	QString m_loadedWorld;
	QSize m_worldSize;
	std::unique_ptr<QSListModel> m_engineModel;
};





#endif // CONQUESTGAME_H
