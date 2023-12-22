/*
 * ---- Call of Suli ----
 *
 * gameenemysniper.h
 *
 * Created on: 2022. 12. 29.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameEnemySniper
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

#ifndef GAMEENEMYSNIPER_H
#define GAMEENEMYSNIPER_H

#include "gameenemy.h"

class GameEnemySniper : public GameEnemy
{
    Q_OBJECT

    Q_PROPERTY(int msecBeforeTurn READ msecBeforeTurn WRITE setMsecBeforeTurn NOTIFY msecBeforeTurnChanged)

public:
    explicit GameEnemySniper(QQuickItem *parent = nullptr);
    virtual ~GameEnemySniper();

    static GameEnemySniper* create(GameScene *scene, const GameTerrain::EnemyData &enemyData, const QString &type = "");

    Q_INVOKABLE void attackPlayer();

    int msecBeforeTurn() const;
    void setMsecBeforeTurn(int newMsecBeforeTurn);

    int turnElapsedMsec() const;
    void setTurnElapsedMsec(int newTurnElapsedMsec);

    void onTimingTimerTimeout(const int &msec, const qreal &delayFactor) override;

protected:
    virtual void rayCastReport(const QMultiMap<qreal, GameEntity *> &items) override;
    virtual void enemyStateModified() override;
    virtual void onSceneConnected() override;

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

#endif // GAMEENEMYSNIPER_H
