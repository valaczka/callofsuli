/*
 * ---- Call of Suli ----
 *
 * gameenemy.h
 *
 * Created on: 2022. 12. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameEnemy
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

#ifndef GAMEENEMY_H
#define GAMEENEMY_H

#include "gameentity.h"
#include "gameterrain.h"

#ifndef Q_OS_WASM
#include <QSoundEffect>
#endif

class GameEnemy : public GameEntity
{
	Q_OBJECT

	Q_PROPERTY(bool moving READ moving WRITE setMoving NOTIFY movingChanged)

#ifndef Q_OS_WASM
	Q_PROPERTY(QSoundEffect soundEffect READ soundEffect CONSTANT)
#endif

public:
	explicit GameEnemy(QQuickItem *parent = nullptr);
	virtual ~GameEnemy();

	const GameTerrain::EnemyData &terrainEnemyData() const;
	void setTerrainEnemyData(const GameTerrain::EnemyData &newTerrainEnemyData);

	bool moving() const;
	void setMoving(bool newMoving);

	Q_INVOKABLE void startMovingAfter(const int &msec);

#ifndef Q_OS_WASM
	QSoundEffect *soundEffect() const { return m_soundEffect; }
#endif

signals:
	void attack();
	void movingChanged();

private slots:
	void onSceneConnected();

protected:
	GameTerrain::EnemyData m_terrainEnemyData;
	bool m_moving = false;
	int m_startMovingAfter = 0;
#ifndef Q_OS_WASM
	QSoundEffect *m_soundEffect = nullptr;
#endif

};

#endif // GAMEENEMY_H
