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
#include "rpgshield.h"
#include "rpgwerebear.h"



/// Static hash

const QHash<QString, RpgEnemyIface::RpgEnemyType> RpgEnemyIface::m_typeHash = {
	{ QStringLiteral("enemy"), EnemyWerebear }
};




/**
 * @brief TiledRpgGame::TiledRpgGame
 * @param parent
 */

TiledRpgGame::TiledRpgGame(QQuickItem *parent)
	: TiledGame(parent)
{

}


/**
 * @brief TiledRpgGame::~TiledRpgGame
 */

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
				"file": "qrc:/teszt.tmx",
				"ambient": "qrc:/rpg/common/birds-isaiah658.mp3"
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
		LOG_CINFO("game") << "CREATE ENEMY" << e.objectId.id << e.objectId.sceneId;

		Q_ASSERT(!e.path.isEmpty());

		IsometricEnemy *character = createEnemy(e.type, e.subtype, e.scene);

		if (!character)
			continue;

		character->setObjectId(e.objectId);

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
			auto player = RpgPlayer::createPlayer(this, p.scene, "default");

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


	for (auto &e : m_pickableDataList) {
		LOG_CINFO("game") << "CREATE PICKABLE" << e.objectId.id << e.objectId.sceneId;

		RpgPickableObject *pickable = createPickable(e.type, e.scene);

		if (!pickable)
			continue;

		pickable->setObjectId(e.objectId);
		pickable->body()->emplace(e.position);

		e.scene->appendToObjects(pickable);

		e.pickableObject = pickable;

		pickable->setIsActive(true);
	}

	return true;
}



/**
 * @brief TiledRpgGame::playerAttackEnemy
 * @param player
 * @param enemy
 */

bool TiledRpgGame::playerAttackEnemy(TiledObject *player, TiledObject *enemy, const TiledWeapon::WeaponType &weaponType)
{
	Q_ASSERT(player);
	Q_ASSERT(enemy);

	IsometricEnemy *e = qobject_cast<IsometricEnemy*>(enemy);
	RpgPlayer *p = qobject_cast<RpgPlayer*>(player);

	if (!e || !p)
		return false;

	if (!canAttack(p, e, weaponType))
		return false;

	if (e->protectWeapon(weaponType))
		return false;

	e->attackedByPlayer(p, weaponType);

	return true;
}



/**
 * @brief TiledRpgGame::enemyAttackPlayer
 * @param enemy
 * @param player
 */

bool TiledRpgGame::enemyAttackPlayer(TiledObject *enemy, TiledObject *player, const TiledWeapon::WeaponType &weaponType)
{
	Q_ASSERT(player);
	Q_ASSERT(enemy);

	IsometricEnemy *e = qobject_cast<IsometricEnemy*>(enemy);
	RpgPlayer *p = qobject_cast<RpgPlayer*>(player);

	if (!e || !p)
		return false;

	if (!canAttack(e, p, weaponType))
		return false;

	if (p->protectWeapon(weaponType))
		return false;

	p->attackedByEnemy(e, weaponType);

	return true;
}



/**
 * @brief TiledRpgGame::playerPickPickable
 * @param player
 * @param pickable
 * @return
 */

bool TiledRpgGame::playerPickPickable(TiledObject *player, TiledObject *pickable)
{
	Q_ASSERT(player);

	RpgPlayer *p = qobject_cast<RpgPlayer*>(player);
	RpgPickableObject *object = qobject_cast<RpgPickableObject*>(pickable);

	if (!p || !object)
		return false;

	if (!object->isActive())
		return false;

	object->playerPick(p);
	object->setIsActive(false);

	if (object->scene()) {
		object->scene()->removeFromObjects(object);
		object->setScene(nullptr);
	}

	p->removePickable(object);

	p->updateLayers();

	playSfx(QStringLiteral(":/rpg/common/leather_inventory.wav"), player->scene(), player->body()->bodyPosition());


	return true;
}



/**
 * @brief TiledRpgGame::onPlayerDead
 * @param player
 */

void TiledRpgGame::onPlayerDead(TiledObject *player)
{

}






/**
 * @brief TiledRpgGame::onEnemyDead
 * @param enemy
 */

void TiledRpgGame::onEnemyDead(TiledObject *enemy)
{
	if (!enemy)
		return;

	RpgEnemyIface *iface = dynamic_cast<RpgEnemyIface*>(enemy);

	if (!iface)
		return;

	RpgPickableObject *pickable = createPickable(RpgPickableObject::PickableShield, enemy->scene());

	if (!pickable)
		return;

	pickable->body()->emplace(iface->getPickablePosition());

	enemy->scene()->appendToObjects(pickable);

	///e.pickableObject = pickable;

	pickable->setIsActive(true);
}






/**
 * @brief TiledRpgGame::canAttack
 * @param player
 * @param enemy
 * @return
 */

bool TiledRpgGame::canAttack(RpgPlayer *player, IsometricEnemy *enemy, const TiledWeapon::WeaponType &weaponType)
{
	if (!player || !enemy)
		return false;

	return true;
}





/**
 * @brief TiledRpgGame::canAttack
 * @param enemy
 * @param player
 * @return
 */

bool TiledRpgGame::canAttack(IsometricEnemy *enemy, RpgPlayer *player, const TiledWeapon::WeaponType &weaponType)
{
	if (!player || !enemy)
		return false;


	return true;
}


/**
 * @brief TiledRpgGame::canTransport
 * @param player
 * @param transport
 * @return
 */

bool TiledRpgGame::canTransport(RpgPlayer *player, TiledTransport *transport)
{
	if (!player || !transport)
		return false;


	return true;
}



/**
 * @brief TiledRpgGame::createEnemy
 * @param type
 * @param subtype
 * @param game
 * @param scene
 * @return
 */

IsometricEnemy *TiledRpgGame::createEnemy(const RpgEnemyIface::RpgEnemyType &type, const QString &subtype, TiledScene *scene)
{
	IsometricEnemy *enemy = nullptr;

	switch (type) {
		case RpgEnemyIface::EnemyWerebear: {
			RpgWerebear *e = nullptr;
			TiledObjectBase::createFromCircle<RpgWerebear>(&e, QPointF{}, 30, nullptr, this);
			e->setWerebearType(subtype);
			enemy = e;
			break;
		}

		case RpgEnemyIface::EnemyInvalid:
			LOG_CERROR("game") << "Invalid enemy type" << type;
			return nullptr;
	}

	if (enemy) {
		enemy->setParent(this);
		enemy->setGame(this);
		enemy->setScene(scene);
		enemy->initialize();
	}

	return enemy;
}



/**
 * @brief TiledRpgGame::createPickable
 * @param type
 * @param scene
 * @return
 */

RpgPickableObject *TiledRpgGame::createPickable(const RpgPickableObject::PickableType &type, TiledScene *scene)
{
	RpgPickableObject *pickable = nullptr;

	switch (type) {
		case RpgPickableObject::PickableShield: {
			RpgShieldPickable *e = nullptr;
			TiledObjectBase::createFromCircle<RpgShieldPickable>(&e, QPointF{}, 30, nullptr, this);
			pickable = e;
			break;
		}
		default:
			break;
	}

	if (pickable) {
		pickable->setParent(this);
		pickable->setGame(this);
		pickable->setScene(scene);
		pickable->initialize();
	}

	return pickable;
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

	if (const RpgEnemyIface::RpgEnemyType &type = RpgEnemyIface::typeFromString(object->className());
			type != RpgEnemyIface::EnemyInvalid) {

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
								   TiledObjectBase::ObjectId{object->id(), scene->sceneId()},
								   type,
								   "whiteShirt",
								   p,
								   object->property("direction").toInt(),
								   scene,
								   nullptr
							   });
		/*
		m_pickableDataList.append(PickableData{
									  TiledObjectBase::ObjectId{},
									  RpgPickableObject::PickableShield,
									  QPointF{},
									  nullptr,
									  ,
									  nullptr
								  });
*/

		LOG_CINFO("scene") << "ENEMY" << object->id();
	} else if (const RpgPickableObject::PickableType &type = RpgPickableObject::typeFromString(object->className());
			   type != RpgPickableObject::PickableInvalid) {

		if (object->shape() != Tiled::MapObject::Point) {
			LOG_CWARNING("scene") << "Invalid object point" << object->id() << scene->sceneId() << object->name();
			return;
		}
		const QPointF &point = renderer->pixelToScreenCoords(object->position());

		m_pickableDataList.append(PickableData{
									  TiledObjectBase::ObjectId{object->id(), scene->sceneId()},
									  type,
									  point,
									  scene,
									  nullptr,
									  nullptr
								  });


		LOG_CINFO("scene") << "PICKABLE" << object->id();
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

		case Qt::Key_Space:
			if (m_controlledPlayer)
				m_controlledPlayer->attackCurrentWeapon();
			break;

		case Qt::Key_Q:
			if (m_controlledPlayer)
				m_controlledPlayer->nextWeapon();
			break;


		case Qt::Key_H:
			if (m_controlledPlayer)
				m_controlledPlayer->setHp(m_controlledPlayer->hp()+3);
			break;


		case Qt::Key_Return:
		case Qt::Key_Enter:
			if (m_controlledPlayer)
				m_controlledPlayer->pickCurrentObject();
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




