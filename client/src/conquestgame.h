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

#include "abstractlevelgame.h"
#include "conquestconfig.h"
#include "conquestlanddata.h"
#include "mapplay.h"
#include "qslistmodel.h"
#include "studentmaphandler.h"


/**
 * @brief The ConquestGame class
 */

class ConquestGame : public AbstractLevelGame
{
	Q_OBJECT

	Q_PROPERTY(ConquestConfig config READ config WRITE setConfig NOTIFY configChanged FINAL)
	Q_PROPERTY(HostMode hostMode READ hostMode NOTIFY hostModeChanged FINAL)
	Q_PROPERTY(int engineId READ engineId WRITE setEngineId NOTIFY engineIdChanged FINAL)
	Q_PROPERTY(int playerId READ playerId WRITE setPlayerId NOTIFY playerIdChanged FINAL)
	Q_PROPERTY(ConquestLandDataList *landDataList READ landDataList CONSTANT FINAL)
	Q_PROPERTY(QSize worldSize READ worldSize WRITE setWorldSize NOTIFY worldSizeChanged FINAL)
	Q_PROPERTY(QString worldBgImage READ worldBgImage WRITE setWorldBgImage NOTIFY worldBgImageChanged FINAL)
	Q_PROPERTY(QString worldOverImage READ worldOverImage WRITE setWorldOverImage NOTIFY worldOverImageChanged FINAL)
	Q_PROPERTY(QSListModel *engineModel READ engineModel CONSTANT FINAL)
	Q_PROPERTY(QSListModel *playersModel READ playersModel NOTIFY playersModelChanged FINAL)
	Q_PROPERTY(ConquestTurn currentTurn READ currentTurn WRITE setCurrentTurn NOTIFY currentTurnChanged FINAL)
	Q_PROPERTY(ConquestTurn::Stage currentStage READ currentStage WRITE setCurrentStage NOTIFY currentStageChanged FINAL)
	Q_PROPERTY(QQuickItem* messageList READ messageList WRITE setMessageList NOTIFY messageListChanged)
	Q_PROPERTY(QColor defaultMessageColor READ defaultMessageColor WRITE setDefaultMessageColor NOTIFY defaultMessageColorChanged FINAL)
	Q_PROPERTY(int tickTimerInterval READ tickTimerInterval CONSTANT FINAL)
	Q_PROPERTY(ConquestPlayer fighter1 READ fighter1 WRITE setFighter1 NOTIFY fighter1Changed FINAL)
	Q_PROPERTY(ConquestPlayer fighter2 READ fighter2 WRITE setFighter2 NOTIFY fighter2Changed FINAL)
	Q_PROPERTY(int fighter2Fortress READ fighter2Fortress WRITE setFighter2Fortress NOTIFY fighter2FortressChanged FINAL)
	Q_PROPERTY(bool isAttacked READ isAttacked WRITE setIsAttacked NOTIFY isAttackedChanged FINAL)
	Q_PROPERTY(int maxPlayersCount READ maxPlayersCount CONSTANT FINAL)
	Q_PROPERTY(int hp READ hp WRITE setHp NOTIFY hpChanged FINAL)
	Q_PROPERTY(QStringList worldListSelect READ worldListSelect WRITE setWorldListSelect NOTIFY worldListSelectChanged FINAL)

public:
	explicit ConquestGame(GameMapMissionLevel *missionLevel, Client *client);
	virtual ~ConquestGame();

	enum HostMode {
		ModeGuest = 0,
		ModeHost
	};

	Q_ENUM(HostMode);

	Q_INVOKABLE void sendWebSocketMessage(const QJsonValue &data = {});
	Q_INVOKABLE void getEngineList();
	Q_INVOKABLE void gameCreate(const QString &character);
	Q_INVOKABLE QColor getPlayerColor(const int &id) const;
	Q_INVOKABLE void messageColor(const QString &text, const QColor &color);
	Q_INVOKABLE void message(const QString &text) { messageColor(text, m_defaultMessageColor); }
	Q_INVOKABLE qint64 currentTick() const { return m_tickTimer.currentTick(); }
	Q_INVOKABLE QString playerCharacter(const int &id) const;

	Q_INVOKABLE void playMenuBgMusic();
	Q_INVOKABLE void stopMenuBgMusic();

	int tickTimerInterval() const { return m_tickTimer.interval(); }

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

	QSListModel *playersModel() const;

	ConquestTurn currentTurn() const;
	void setCurrentTurn(const ConquestTurn &newCurrentTurn);

	ConquestTurn::Stage currentStage() const;
	void setCurrentStage(const ConquestTurn::Stage &newCurrentStage);

	QQuickItem *messageList() const;
	void setMessageList(QQuickItem *newMessageList);

	QColor defaultMessageColor() const;
	void setDefaultMessageColor(const QColor &newDefaultMessageColor);

	bool isAttacked() const;
	void setIsAttacked(bool newIsAttacked);

	ConquestPlayer fighter1() const;
	void setFighter1(const ConquestPlayer &newFighter1);

	ConquestPlayer fighter2() const;
	void setFighter2(const ConquestPlayer &newFighter2);

	Q_INVOKABLE QString missionName() const;
	Q_INVOKABLE int missionLevel() const;

	static int maxPlayersCount();

	int hp() const;
	void setHp(int newHp);

	QString worldBgImage() const;
	void setWorldBgImage(const QString &newWorldBgImage);

	QString worldOverImage() const;
	void setWorldOverImage(const QString &newWorldOverImage);

	QStringList worldListSelect() const;
	void setWorldListSelect(const QStringList &newWorldListSelect);

	int fighter2Fortress() const;
	void setFighter2Fortress(int newFighter2Fortress);

signals:
	void mapDownRequest();
	void mapUpRequest();
	void answerFailed();

	void configChanged();
	void hostModeChanged();
	void engineIdChanged();
	void playerIdChanged();
	void worldSizeChanged();
	void currentTurnChanged();
	void currentStageChanged();
	void messageListChanged();
	void defaultMessageColorChanged();
	void isAttackedChanged();
	void fighter1Changed();
	void fighter2Changed();
	void hpChanged();
	void worldBgImageChanged();
	void worldOverImageChanged();
	void worldListSelectChanged();
	void fighter2FortressChanged();
	void playersModelChanged();

protected:
	virtual QQuickItem* loadPage() override;
	virtual void timerEvent(QTimerEvent *) override;
	virtual void connectGameQuestion() override;
	virtual bool gameStartEvent() override;
	virtual bool gameFinishEvent() override;
	virtual void jsonOrigDataCheck(const QJsonObject &obj) { Q_UNUSED(obj); }

private:
	void onTimeSyncTimerTimeout();
	void onWebSocketActiveChanged();
	void onJsonReceived(const QString &operation, const QJsonValue &data);
	void onBinaryMessageReceived(const QByteArray &message);
	void onConfigChanged();
	void updatePlayer();

	void cmdList(const QJsonObject &data);
	void cmdState(const QJsonObject &data);
	void cmdCreate(const QJsonObject &data);
	void cmdConnect(const QJsonObject &data);
	void cmdStart(const QJsonObject &data);
	void cmdPrepare(const QJsonObject &data);
	void cmdLeave(const QJsonObject &data);
	void cmdQuestionRequest(const QJsonObject &data);

	void reloadLandList();
	void getWorldList(ConquestWordListHelper *helper) const;
	void getCharacterList(ConquestWordListHelper *helper) const;

	void loadQuestion();
	void revealQuestion();
	void onGameQuestionSuccess(const QVariantMap &answer);
	void onGameQuestionFailed(const QVariantMap &answer);
	void onGameQuestionFinished();

	bool m_binarySignalConnected = false;
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
	std::unique_ptr<QSListModel> m_playersModel;
	ConquestTurn m_currentTurn;
	ConquestTurn::Stage m_currentStage = ConquestTurn::StageInvalid;
	QQuickItem *m_messageList = nullptr;
	QColor m_defaultMessageColor = Qt::white;
	bool m_isAttacked = false;
	QJsonObject m_loadedQuestion;
	ConquestConfig::GameState m_oldGameState = ConquestConfig::StateInvalid;
	ConquestPlayer m_fighter1;
	ConquestPlayer m_fighter2;
	int m_fighter2Fortress = -1;
	int m_hp = 0;
	QString m_worldBgImage;
	QString m_worldOverImage;
	QStringList m_worldListSelect;
};





#endif // CONQUESTGAME_H
