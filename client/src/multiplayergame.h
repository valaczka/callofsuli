#ifndef MULTIPLAYERGAME_H
#define MULTIPLAYERGAME_H

#include "actiongame.h"
#include "objectstate.h"

class MultiPlayerGame : public ActionGame
{
    Q_OBJECT

    Q_PROPERTY(Mode multiPlayerMode READ multiPlayerMode NOTIFY multiPlayerModeChanged)
    Q_PROPERTY(int engineId READ engineId NOTIFY engineIdChanged)
    Q_PROPERTY(MultiPlayerGameState multiPlayerGameState READ multiPlayerGameState NOTIFY multiPlayerGameStateChanged)
    Q_PROPERTY(int playerId READ playerId WRITE setPlayerId NOTIFY playerIdChanged FINAL)
    Q_PROPERTY(qint64 playerEntityId READ playerEntityId WRITE setPlayerEntityId NOTIFY playerEntityIdChanged FINAL)

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

    MultiPlayerGameState multiPlayerGameState() const;
    void setMultiPlayerGameState(MultiPlayerGameState newMultiPlayerGameState);

    int serverInterval() const;
    void setServerInterval(int newServerInterval);

    int playerId() const;
    void setPlayerId(int newPlayerId);

    qint64 playerEntityId() const;
    void setPlayerEntityId(qint64 newPlayerEntityId);

    qint64 getEntityId(GameObject *object) const;
    GameObject *getEntity(const qint64 &entityId) const;

    virtual void enemyAttackPlayer(GameEnemy *enemy, const bool &canProtect, GamePlayer *player) override;
    virtual void enemyKillPlayer(GameEnemy *enemy, GamePlayer *player) override;
    virtual void tryAttack(GamePlayer *player, GameEnemy *enemy) override;

public slots:
    void gameAbort() override;

signals:
    void multiPlayerModeChanged();
    void engineIdChanged();
    void multiPlayerGameStateChanged();
    void playerIdChanged();
    void playerEntityIdChanged();

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
    void loadGamePage();

    void createGameTrigger();
    void updateGameTrigger(const QByteArray &data);
    void playGameTrigger();

    void updateBody(GameObject *object, const bool &owned);

    QTimer m_timeSyncTimer;
    Mode m_multiPlayerMode = MultiPlayerClient;
    int m_engineId = -1;
    int m_serverInterval = 0;
    int m_playerId = -1;
    qint64 m_playerEntityId = -1;
    MultiPlayerGameState m_multiPlayerGameState = StateInvalid;

    struct Entity {
        qint64 id = -1;
        ObjectStateBase::ObjectType type = ObjectStateBase::TypeInvalid;
        QPointer<GameObject> object;
    };

    std::vector<Entity> m_entities;

    bool m_binarySignalConnected = false;
};

#endif // MULTIPLAYERGAME_H
