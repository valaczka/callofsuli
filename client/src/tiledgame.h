/*
 * ---- Call of Suli ----
 *
 * tiledgame.h
 *
 * Created on: 2024. 03. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledGame
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

#ifndef TILEDGAME_H
#define TILEDGAME_H

#include "isometricplayer.h"
#include "tiledscene.h"
#include <QQuickItem>

class TiledGame : public QQuickItem
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(QQuickItem *joystick READ joystick WRITE setJoystick NOTIFY joystickChanged FINAL)
	Q_PROPERTY(bool debugView READ debugView WRITE setDebugView NOTIFY debugViewChanged FINAL)
	Q_PROPERTY(TiledScene *currentScene READ currentScene WRITE setCurrentScene NOTIFY currentSceneChanged FINAL)

public:
	explicit TiledGame(QQuickItem *parent = nullptr);
	virtual ~TiledGame();

	Q_INVOKABLE bool load();

	void loadPlayer(TiledScene *scene, const QPointF &pos);

	QQuickItem *joystick() const;
	void setJoystick(QQuickItem *newJoystick);

	bool debugView() const;
	void setDebugView(bool newDebugView);

	TiledScene *currentScene() const;
	void setCurrentScene(TiledScene *newCurrentScene);

signals:
	void joystickChanged();
	void debugViewChanged();

	void currentSceneChanged();

private:
	bool loadScene(const QString &file);


protected:
	QPointer<QQuickItem> m_joystick;

	bool m_debugView = false;

	struct Scene {
		QQuickItem *container = nullptr;
		TiledScene *scene = nullptr;
	};

	QVector<Scene> m_sceneList;
	TiledScene *m_currentScene = nullptr;
	std::unique_ptr<IsometricPlayer> m_player;
};

#endif // TILEDGAME_H
