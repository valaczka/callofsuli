/*
 * ---- Call of Suli ----
 *
 * tiledactiongame.cpp
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

#include "tiledrpggame.h"

TiledRpgGame::TiledRpgGame(QQuickItem *parent)
	: TiledGame(parent)
{

}



bool TiledRpgGame::load()
{
	QString test = R"({
		"firstScene": 1,
		"scenes": [
			{
				"id": 1,
				"file": "qrc:/teszt.tmx"
			},

			{
				"id": 2,
				"file": "qrc:/teszt2.tmx"
			}
		]
		})";


	TiledGameDefinition def;
	def.fromJson(QJsonDocument::fromJson(test.toUtf8()).object());

	if (!TiledGame::load(def))
		return false;


	for (const auto &e : m_enemyDataList) {
		LOG_CINFO("game") << "CREATE ENEMY" << e.object.id << e.object.sceneId;
		IsometricEnemy *character = IsometricEnemy::createEnemy(e.type, e.scene);

		if (!character)
			continue;

		character->setObjectId(e.object);

		character->loadPathMotor(e.path);
		e.scene->appendToObjects(character);

		break;
	}


	if (!m_playerPositionList.isEmpty()) {
		const auto &p = m_playerPositionList.first();

		auto player = IsometricPlayer::createPlayer(p.scene);

		Q_ASSERT(player);

		player->emplace(p.position);
		player->setCurrentDirection(TiledObject::South);

		p.scene->appendToObjects(player);
		setFollowedItem(player);
		setControlledPlayer(player);


	}


	/*


		IsometricEnemy *character = IsometricEnemy::createEnemy(type, scene);

		if (!character)
			return;

		addLoadedObject(object->id(), scene->sceneId());
		character->setObjectId({object->id(), scene->sceneId()});

		character->loadPathMotor(p);
		scene->appendToObjects(character);

*/


	return true;
}



/**
 * @brief TiledRpgGame::loadObjectLayer
 * @param scene
 * @param object
 * @param renderer
 */

void TiledRpgGame::loadObjectLayer(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(object);
	Q_ASSERT(renderer);

	if (const IsometricEnemyIface::EnemyType &type = IsometricEnemyIface::typeFromString(object->className());
			type != IsometricEnemyIface::EnemyInvalid) {
		QPolygonF p = TiledObjectBase::toPolygon(object, renderer);

		m_enemyDataList.append(EnemyData{
								   TiledObjectBase::Object{object->id(), scene->sceneId()},
								   type,
								   p,
								   scene,
								   nullptr
							   });

		LOG_CINFO("scene") << "ENEMY" << object->id();
	} else {
		LOG_CWARNING("game") << "Invalid object type" << object->className() << object->name() << object->id();
	}
}


/**
 * @brief TiledRpgGame::joystickStateEvent
 * @param newJoystickState
 */

void TiledRpgGame::joystickStateEvent(const JoystickState &state)
{
	if (m_controlledPlayer)
		m_controlledPlayer->onJoystickStateChanged(state);
}


/**
 * @brief TiledRpgGame::keyPressEvent
 * @param event
 */

void TiledRpgGame::keyPressEvent(QKeyEvent *event)
{
	const int &key = event->key();

	switch (key) {
		case Qt::Key_S:
			//switchScene();
			break;

		case Qt::Key_Space:
			if (m_controlledPlayer)
				m_controlledPlayer->hit();
			break;

		default:
			TiledGame::keyPressEvent(event);
			break;
	}
}


/**
 * @brief TiledRpgGame::controlledPlayer
 * @return
 */

IsometricPlayer *TiledRpgGame::controlledPlayer() const
{
	return m_controlledPlayer;
}

void TiledRpgGame::setControlledPlayer(IsometricPlayer *newControlledPlayer)
{
	if (m_controlledPlayer == newControlledPlayer)
		return;
	m_controlledPlayer = newControlledPlayer;
	emit controlledPlayerChanged();
}








/**
 * @brief TiledGame::switchScene

void TiledGame::switchScene()
{
	LOG_CDEBUG("scene") << "SWITCH";

	TiledTransport *transport = m_player->currentTransport();

	if (!transport) {
		LOG_CWARNING("scene") << "No transport";
		return;
	}

	TiledScene *oldScene = m_player->scene();
	TiledScene *newScene = transport->otherScene(oldScene);
	TiledObjectBase *newObject = transport->otherObject(oldScene);

	if (!newScene || !newObject) {
		LOG_CERROR("scene") << "Wrong transport object";
		return;
	}

	oldScene->removeFromObjects(m_player.get());

	m_player->setScene(newScene);
	m_player->emplace(newObject->body()->bodyPosition());

	newScene->appendToObjects(m_player.get());

	setCurrentScene(newScene);
	newScene->forceActiveFocus();
}


void TiledGame::loadPlayer(TiledScene *scene, const QPointF &pos)
{
	m_player.reset(IsometricPlayer::createPlayer(scene));

	Q_ASSERT(m_player);

	m_player->emplace(pos);
	m_player->setCurrentDirection(TiledObject::South);

	scene->appendToObjects(m_player.get());
	setFollowedItem(m_player.get());
	setControlledPlayer(m_player.get());
}
*/
