/*
 * ---- Call of Suli ----
 *
 * gameenemysoldier.h
 *
 * Created on: 2022. 12. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameEnemySoldier
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

#ifndef GAMEENEMYSOLDIER_H
#define GAMEENEMYSOLDIER_H

#include "gameenemy.h"

class GameEnemySoldier : public GameEnemy
{
    Q_OBJECT

    Q_PROPERTY(int msecBeforeTurn READ msecBeforeTurn WRITE setMsecBeforeTurn NOTIFY msecBeforeTurnChanged)

public:
    explicit GameEnemySoldier(QQuickItem *parent = nullptr);
    virtual ~GameEnemySoldier();

    int msecBeforeTurn() const;
    void setMsecBeforeTurn(int newMsecBeforeTurn);

    int turnElapsedMsec() const;
    void setTurnElapsedMsec(int newTurnElapsedMsec);

    static GameEnemySoldier* create(GameScene *scene, const GameTerrain::EnemyData &enemyData, const QString &type = "");
    static ObjectStateBase createState(const GameTerrain::EnemyData &enemyData);

    Q_INVOKABLE void attackPlayer();

    void onTimingTimerTimeout(const int &msec, const qreal &delayFactor) override;
    virtual void cacheCurrentState() override;

    virtual void setStateFromSnapshot(const ObjectStateBase &ptr, const qint64 &currentTick, const bool &force) override;
    virtual ObjectStateBase getCurrentState() const override;
    virtual void setCurrentState(const ObjectStateBase &state, const bool &force) override;

protected:
    virtual void rayCastReport(const QMultiMap<qreal, GameEntity *> &items) override;
    virtual void enemyStateModified() override;
    virtual ObjectStateBase interpolate(const qreal &t, const ObjectStateBase &from, const ObjectStateBase &to) override;

private slots:
    void onSceneConnected();

signals:
    void msecBeforeTurnChanged();
    void turnElapsedMsecChanged();

private slots:
    void onAttack();

private:
    int m_msecBeforeTurn = 5000;
    int m_turnElapsedMsec = -1;
    int m_attackElapsedMsec = -1;
};

#endif // GAMEENEMYSOLDIER_H
