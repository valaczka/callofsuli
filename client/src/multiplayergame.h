#ifndef MULTIPLAYERGAME_H
#define MULTIPLAYERGAME_H

#include "actiongame.h"

class MultiPlayerGame : public ActionGame
{
	Q_OBJECT

	Q_PROPERTY(Mode multiPlayerMode READ multiPlayerMode NOTIFY multiPlayerModeChanged)
	Q_PROPERTY(int engineId READ engineId WRITE setEngineId NOTIFY engineIdChanged)

public:
	explicit MultiPlayerGame(GameMapMissionLevel *missionLevel, Client *client);
	virtual ~MultiPlayerGame();

	enum Mode {
		MultiPlayerClient = 0,
		MultiPlayerHost = 1
	};

	Q_ENUM(Mode);

	Q_INVOKABLE void start();
	Q_INVOKABLE void sendWebSocketMessage(const QJsonValue &data = {});
	Q_INVOKABLE void getServerState();

	const Mode &multiPlayerMode() const;
	void setMultiPlayerMode(const Mode &newMultiPlayerMode);

	virtual void sceneTimerTimeout(const int &msec, const qreal &delayFactor) override;
	virtual void onSceneReady() override;
	virtual void onSceneAnimationFinished() override;

	int engineId() const;
	void setEngineId(int newEngineId);

public slots:
	void gameAbort() override;

signals:
	void multiPlayerModeChanged();
	void engineIdChanged();

protected:
	virtual QQuickItem *loadPage() override;
	virtual void onSceneAboutToStart() override;
	virtual void timerEvent(QTimerEvent *) override;
	//virtual void connectGameQuestion() override;
	//virtual bool gameFinishEvent() override;

private:
	void onTimeSyncTimerTimeout();
	void onActiveChanged();
	void onJsonReceived(const QString &operation, const QJsonValue &data);
	void onBinaryDataReceived(const QByteArray &data);
	void loadGamePage();

	QTimer m_timeSyncTimer;
	Mode m_multiPlayerMode = MultiPlayerClient;
	int m_engineId = -1;

	QHash<qint64, GameObject*> m_test_enemies;	//!!! unique!

};

#endif // MULTIPLAYERGAME_H
