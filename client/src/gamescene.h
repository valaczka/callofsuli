/*
 * ---- Call of Suli ----
 *
 * gamescene.h
 *
 * Created on: 2022. 12. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameScene
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

#ifndef GAMESCENE_H
#define GAMESCENE_H

#include "qloggingcategory.h"
#include <QQuickItem>
#include "gameterrain.h"

class ActionGame;

class GameScene : public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY(ActionGame* game READ game WRITE setGame NOTIFY gameChanged)

public:
	GameScene(QQuickItem *parent = nullptr);
	virtual ~GameScene();

	ActionGame *game() const;
	void setGame(ActionGame *newGame);

	Q_INVOKABLE void loadTerrain(const QString &terrainName);

signals:
	void gameChanged();

private:
	ActionGame *m_game = nullptr;
	GameTerrain m_terrain;
};


Q_DECLARE_LOGGING_CATEGORY(lcScene);

#endif // GAMESCENE_H
