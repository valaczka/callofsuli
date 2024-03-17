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
#include "tiledspritehandler.h"

TiledRpgGame::TiledRpgGame(QQuickItem *parent)
	: TiledGame(parent)
{

}

TiledRpgGame::~TiledRpgGame()
{
	for (const auto &e : m_enemyDataList) {
		if (e.enemy)
			e.enemy->setGame(nullptr);
	}

	for (const auto &p : m_players) {
		if (p)
			p->setGame(nullptr);
	}

	m_enemyDataList.clear();
	m_players.clear();
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




	for (auto &e : m_enemyDataList) {
		LOG_CINFO("game") << "CREATE ENEMY" << e.object.id << e.object.sceneId;

		Q_ASSERT(!e.path.isEmpty());

		IsometricEnemy *character = IsometricEnemy::createEnemy(e.type, e.subtype, this, e.scene);

		if (!character)
			continue;

		character->setObjectId(e.object);

		if (e.path.size() > 1)
			character->loadPathMotor(e.path);
		else
			character->loadFixPositionMotor(e.path.first(), character->nearestDirectionFromRadian(e.defaultAngle));

		e.scene->appendToObjects(character);

		e.enemy = character;
	}


	QList<RpgPlayer*> list;

	if (!m_playerPositionList.isEmpty()) {
		const auto &p = m_playerPositionList.first();

		for (int i=0; i<3; ++i) {
			auto player = RpgPlayer::createPlayer(this, p.scene, "rpgDefault");

			player->emplace(p.position+QPointF(i*15, i*15));
			if (i == 0)
				player->setCurrentAngle(TiledObject::directionToRadian(TiledObject::West));
			else
				player->setCurrentAngle(TiledObject::directionToRadian(TiledObject::NorthEast));

			p.scene->appendToObjects(player);

			if (i==0) {
				setFollowedItem(player);
				setControlledPlayer(player);
			}

			list.append(player);
		}
	}

	setPlayers(list);


	return true;
}



/**
 * @brief TiledRpgGame::playerAttackEnemy
 * @param player
 * @param enemy
 */

void TiledRpgGame::playerAttackEnemy(TiledObject *player, TiledObject *enemy)
{
	Q_ASSERT(player);
	Q_ASSERT(enemy);

	IsometricEnemy *e = qobject_cast<IsometricEnemy*>(enemy);
	IsometricPlayer *p = qobject_cast<IsometricPlayer*>(player);

	if (!e || !p)
		return;

	e->attackedByPlayer(p);
}



/**
 * @brief TiledRpgGame::enemyAttackPlayer
 * @param enemy
 * @param player
 */

void TiledRpgGame::enemyAttackPlayer(TiledObject *enemy, TiledObject *player)
{
	Q_ASSERT(player);
	Q_ASSERT(enemy);

	IsometricEnemy *e = qobject_cast<IsometricEnemy*>(enemy);
	RpgPlayer *p = qobject_cast<RpgPlayer*>(player);

	if (!e || !p)
		return;

	p->attackedByEnemy(e);
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

		QPolygonF p;

		if (object->shape() == Tiled::MapObject::Polygon ||
				object->shape() == Tiled::MapObject::Polyline)
			p = TiledObjectBase::toPolygon(object, renderer);
		else if (object->shape() == Tiled::MapObject::Point)
			p << renderer->pixelToScreenCoords(object->position());


		if (p.isEmpty()) {
			LOG_CWARNING("scene") << "Invalid enemy polygon" << object->id() << scene->sceneId() << object->name();
			return;
		}

		m_enemyDataList.append(EnemyData{
								   TiledObjectBase::Object{object->id(), scene->sceneId()},
								   type,
								   "whiteShirt",
								   p,
								   object->property("direction").toInt(),
								   scene,
								   nullptr
							   });


		LOG_CINFO("scene") << "ENEMY" << object->id();
	} else {
		LOG_CWARNING("game") << "Invalid object type" << object->className() << object->name() << object->id() << scene->sceneId();
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
			if (m_controlledPlayer && m_controlledPlayer->currentTransport())
				transport(m_controlledPlayer, m_controlledPlayer->currentTransport());
			break;

		case Qt::Key_Q:
			if (m_controlledPlayer)
				m_controlledPlayer->shot();
			break;

		case Qt::Key_Space:
			if (m_controlledPlayer)
				m_controlledPlayer->hit();
			break;


		case Qt::Key_X:
			for (auto &e : m_enemyDataList) {
				if (e.enemy) {
					e.enemy->setHp(0);
				}
			}
			break;

		case Qt::Key_Y:
			for (int i=1; i<m_players.size(); ++i) {
				m_players.at(i)->setHp(0);
			}
			break;


		default:
			TiledGame::keyPressEvent(event);
			break;
	}
}


/**
 * @brief TiledRpgGame::players
 * @return
 */

QList<RpgPlayer *> TiledRpgGame::players() const
{
	return m_players;
}

void TiledRpgGame::setPlayers(const QList<RpgPlayer *> &newPlayers)
{
	if (m_players == newPlayers)
		return;
	m_players = newPlayers;
	emit playersChanged();
}


/**
 * @brief TiledRpgGame::controlledPlayer
 * @return
 */

RpgPlayer *TiledRpgGame::controlledPlayer() const
{
	return m_controlledPlayer;
}

void TiledRpgGame::setControlledPlayer(RpgPlayer *newControlledPlayer)
{
	if (m_controlledPlayer == newControlledPlayer)
		return;
	m_controlledPlayer = newControlledPlayer;
	emit controlledPlayerChanged();
}




