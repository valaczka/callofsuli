/*
 * ---- Call of Suli ----
 *
 * tiledactiongame.h
 *
 * Created on: 2024. 03. 12.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledRpgGame
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

#ifndef TILEDRPGGAME_H
#define TILEDRPGGAME_H

#include "tiledgame.h"
#include "isometricenemy.h"
#include <QQmlEngine>

class TiledRpgGame : public TiledGame
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(QList<IsometricPlayer *> players READ players WRITE setPlayers NOTIFY playersChanged FINAL)
	Q_PROPERTY(IsometricPlayer *controlledPlayer READ controlledPlayer WRITE setControlledPlayer NOTIFY controlledPlayerChanged FINAL)

public:
	explicit TiledRpgGame(QQuickItem *parent = nullptr);
	virtual ~TiledRpgGame();


	Q_INVOKABLE bool load();

	IsometricPlayer *controlledPlayer() const;
	void setControlledPlayer(IsometricPlayer *newControlledPlayer);

	QList<IsometricPlayer *> players() const;
	void setPlayers(const QList<IsometricPlayer *> &newPlayers);

signals:
	void controlledPlayerChanged();

	void playersChanged();

protected:
	virtual void loadObjectLayer(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer) override;
	virtual void joystickStateEvent(const JoystickState &state) override;
	virtual void keyPressEvent(QKeyEvent *event) override final;

private:
	void addEnemyPosition(TiledScene *scene, const IsometricEnemyIface::EnemyType &type, const QPolygonF &position);

	struct EnemyData {
		TiledObjectBase::Object object;
		IsometricEnemyIface::EnemyType type = IsometricEnemyIface::EnemyInvalid;
		QPolygonF path;
		int defaultAngle = 0;
		QPointer<TiledScene> scene;
		QPointer<IsometricEnemy> enemy;
	};

	QVector<EnemyData> m_enemyDataList;

	QList<IsometricPlayer*> m_players;
	QPointer<IsometricPlayer> m_controlledPlayer;
};

#endif // TILEDRPGGAME_H
