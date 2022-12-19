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
	Q_PROPERTY(bool atBound READ atBound WRITE setAtBound NOTIFY atBoundChanged)

public:
	explicit GameEnemySoldier(QQuickItem *parent = nullptr);
	virtual ~GameEnemySoldier();

	int msecBeforeTurn() const;
	void setMsecBeforeTurn(int newMsecBeforeTurn);

	bool atBound() const;
	void setAtBound(bool newAtBound);

	int turnElapsedMsec() const;
	void setTurnElapsedMsec(int newTurnElapsedMsec);

	static GameEnemySoldier* create(GameScene *scene, const GameTerrain::EnemyData &enemyData, const QString &type = "");

protected:
	virtual void rayCastReport(const QMultiMap<qreal, GameEntity *> &items) override;

private slots:
	void onSceneConnected();


signals:
	void msecBeforeTurnChanged();
	void atBoundChanged();
	void turnElapsedMsecChanged();

private slots:
	void onAttack();
	void onKilled();
	void onMovingTimerTimeout();
	void onMovingChanged();
	void onAtBoundChanged();

private:
	int m_msecBeforeTurn = 5000;
	bool m_atBound = false;
	int m_turnElapsedMsec = -1;
};

#endif // GAMEENEMYSOLDIER_H
