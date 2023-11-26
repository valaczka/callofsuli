#ifndef MULTIPLAYERGAME_H
#define MULTIPLAYERGAME_H

#include "actiongame.h"
#include "librg.h"

class MultiPlayerGame : public ActionGame
{
	Q_OBJECT

	Q_PROPERTY(Mode multiPlayerMode READ multiPlayerMode WRITE setMultiPlayerMode NOTIFY multiPlayerModeChanged)

public:
	explicit MultiPlayerGame(GameMapMissionLevel *missionLevel, Client *client);
	virtual ~MultiPlayerGame();

	enum Mode {
		MultiPlayerClient = 0,
		MultiPlayerHost = 1
	};

	Q_ENUM(Mode);

	Q_INVOKABLE void start();

	const Mode &multiPlayerMode() const;
	void setMultiPlayerMode(const Mode &newMultiPlayerMode);

	virtual void recreateEnemies() override;

public slots:
	void gameAbort() override;

signals:
	void multiPlayerModeChanged();

protected:
	virtual QQuickItem *loadPage() override;
	//virtual void connectGameQuestion() override;
	//virtual bool gameFinishEvent() override;

private:
	void onTimeSyncTimerTimeout();
	void onJsonReceived(const QString &operation, const QJsonValue &data);
	void loadGamePage();

	QTimer m_timeSyncTimer;
	Mode m_multiPlayerMode = MultiPlayerClient;

	librg_world* m_world = nullptr;

};

#endif // MULTIPLAYERGAME_H
