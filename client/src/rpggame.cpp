/*
 * ---- Call of Suli ----
 *
 * tiledactiongame.cpp
 *
 * Created on: 2024. 03. 12.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgGame
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

#include "rpgarrow.h"
#include "rpgenemybase.h"
#include "rpgfireball.h"
#include "rpggame.h"
#include "rpghp.h"
#include "rpglongbow.h"
#include "rpglongsword.h"
#include "rpgshield.h"
#include "rpgwerebear.h"
#include "sound.h"
#include "application.h"
#include "rpgquestion.h"
#include <QRandomGenerator>

#ifndef Q_OS_WASM
#include "standaloneclient.h"
#endif




/// Static hash

const QHash<QString, RpgEnemyIface::RpgEnemyType> RpgEnemyIface::m_typeHash = {
	{ QStringLiteral("enemy"), EnemyTest }
};



/// Base entity (special thanks to SKYZ0R)

const QByteArray RpgGame::m_baseEntitySprite0 = R"(
{
"sprites": [
	{
		"name": "idle",
		"directions": [ 225, 270, 315, 360, 45, 90, 135, 180 ],
		"x": 0,
		"y": 0,
		"count": 4,
		"width": 148,
		"height": 130,
		"duration": 250
	},

	{
		"name": "attack",
		"directions": [ 225, 270, 315, 360, 45, 90, 135, 180 ],
		"x": 592,
		"y": 0,
		"count": 10,
		"width": 148,
		"height": 130,
		"duration": 40,
		"loops": 1
	},

	{
		"name": "hurt",
		"directions": [ 225, 270, 315, 360, 45, 90, 135, 180 ],
		"x": 2072,
		"y": 0,
		"count": 4,
		"width": 148,
		"height": 130,
		"duration": 60,
		"loops": 1
	},

	{
		"name": "death",
		"directions": [ 225, 270, 315, 360, 45, 90, 135, 180 ],
		"x": 2072,
		"y": 0,
		"count": 8,
		"width": 148,
		"height": 130,
		"duration": 60,
		"loops": 1
	}

]
}
)";







const QByteArray RpgGame::m_baseEntitySprite1 = R"(
{
"sprites": [
	{
		"name": "walk",
		"directions": [ 225, 270, 315, 360, 45, 90, 135, 180 ],
		"x": 0,
		"y": 0,
		"count": 11,
		"width": 148,
		"height": 130,
		"duration": 60
	},

	{
		"name": "run",
		"directions": [ 225, 270, 315, 360, 45, 90, 135, 180 ],
		"x": 1628,
		"y": 0,
		"count": 10,
		"width": 148,
		"height": 130,
		"duration": 60
	}

]
}
)";





const QByteArray RpgGame::m_baseEntitySprite2 = R"(
{
"sprites": [
	{
		"name": "bow",
		"directions": [ 225, 270, 315, 360, 45, 90, 135, 180 ],
		"x": 0,
		"y": 0,
		"count": 9,
		"width": 148,
		"height": 130,
		"duration": 40,
		"loops": 1
	},

	{
		"name": "cast",
		"directions": [ 225, 270, 315, 360, 45, 90, 135, 180 ],
		"x": 1332,
		"y": 0,
		"count": 9,
		"width": 148,
		"height": 130,
		"duration": 40,
		"loops": 1
	}
]
}
)";






/**
 * @brief RpgGame::RpgGame
 * @param parent
 */

RpgGame::RpgGame(QQuickItem *parent)
	: TiledGame(parent)
{

}


/**
 * @brief RpgGame::~RpgGame
 */

RpgGame::~RpgGame()
{
	for (const auto &e : std::as_const(m_enemyDataList)) {
		if (e.enemy)
			e.enemy->setGame(nullptr);
	}

	for (const auto &p : std::as_const(m_players)) {
		if (p)
			p->setGame(nullptr);
	}

	m_enemyDataList.clear();
	m_players.clear();
}





/**
 * @brief RpgGame::load
 * @return
 */


bool RpgGame::load()
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

		IsometricEnemy *character;

		if (QRandomGenerator::global()->bounded(2) % 2 == 0) {
			character = createEnemy(e.type, e.subtype, e.scene);
			if (RpgEnemyIface *iface = dynamic_cast<RpgEnemyIface*>(character)) {
				auto w = iface->armory()->weaponAdd(new RpgLongsword);
				w->setBulletCount(-1);
				//w->setCanThrow(true);
				iface->armory()->setCurrentWeapon(w);
				e.hasQuestion = true;
			}
		} else {
			character = createEnemy(RpgEnemyIface::EnemyWerebear, e.subtype, e.scene);
		}

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

	recalculateEnemies();


	QList<RpgPlayer*> list;

	if (!m_playerPositionList.isEmpty()) {
		const auto &p = m_playerPositionList.first();

		for (int i=0; i<1; ++i) {
			auto player = RpgPlayer::createPlayer(this, p.scene, i==0 ? "default" : "default2");

			player->emplace(p.position+QPointF(i*15, i*15));
			if (i == 0)
				player->setCurrentAngle(TiledObject::directionToRadian(TiledObject::West));
			else
				player->setCurrentAngle(TiledObject::directionToRadian(TiledObject::SouthEast));

			p.scene->appendToObjects(player);

			if (i==0) {
				setFollowedItem(player);
				setControlledPlayer(player);
			}

			player->setCurrentSceneStartPosition(p.position);

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


	emit gameLoaded();

	return true;
}



/**
 * @brief RpgGame::playerAttackEnemy
 * @param player
 * @param enemy
 */

bool RpgGame::playerAttackEnemy(TiledObject *player, TiledObject *enemy, const TiledWeapon::WeaponType &weaponType)
{
	Q_ASSERT(player);
	Q_ASSERT(enemy);

	IsometricEnemy *e = qobject_cast<IsometricEnemy*>(enemy);
	RpgEnemyIface *iface = dynamic_cast<RpgEnemyIface*>(enemy);
	RpgPlayer *p = qobject_cast<RpgPlayer*>(player);

	if (!e || !p || !iface)
		return false;

	if (!canAttack(p, e, weaponType))
		return false;

	if (p->isLocked())
		return false;

	if (iface->protectWeapon(weaponType))
		return false;

	const auto &ptr = getEnemyData(e);
	if (ptr && ptr->hasQuestion) {
		LOG_CWARNING("game") << "Enemy has question" << ptr->enemy;

		const int &hp = e->getNewHpAfterAttack(e->hp(), weaponType, p);
		if (hp <= 0 && m_rpgQuestion->nextQuestion(p, e, weaponType)) {
			Application::instance()->client()->sound()->playSound(QStringLiteral("qrc:/sound/sfx/question.mp3"), Sound::SfxChannel);
			return true;
		}
	}

	e->attackedByPlayer(p, weaponType);

	return true;
}



/**
 * @brief RpgGame::enemyAttackPlayer
 * @param enemy
 * @param player
 */

bool RpgGame::enemyAttackPlayer(TiledObject *enemy, TiledObject *player, const TiledWeapon::WeaponType &weaponType)
{
	Q_ASSERT(player);
	Q_ASSERT(enemy);

	IsometricEnemy *e = qobject_cast<IsometricEnemy*>(enemy);
	RpgPlayer *p = qobject_cast<RpgPlayer*>(player);

	if (!e || !p)
		return false;

	if (!canAttack(e, p, weaponType))
		return false;

	const bool &prot = p->protectWeapon(weaponType);

	p->attackedByEnemy(e, weaponType, prot);

	return true;
}



/**
 * @brief RpgGame::playerPickPickable
 * @param player
 * @param pickable
 * @return
 */

bool RpgGame::playerPickPickable(TiledObject *player, TiledObject *pickable)
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

	p->armory()->updateLayers();

	playSfx(QStringLiteral(":/rpg/common/leather_inventory.mp3"), player->scene(), player->body()->bodyPosition());


	return true;
}



/**
 * @brief RpgGame::onPlayerDead
 * @param player
 */

void RpgGame::onPlayerDead(TiledObject *player)
{
	RpgPlayer *isoPlayer = qobject_cast<RpgPlayer*>(player);

	if (isoPlayer) {
		for (const EnemyData &e : m_enemyDataList) {
			if (e.enemy)
				e.enemy->removeContactedPlayer(isoPlayer);
		}
	}

	emit playerDead(isoPlayer);
}






/**
 * @brief RpgGame::onEnemyDead
 * @param enemy
 */

void RpgGame::onEnemyDead(TiledObject *enemy)
{
	if (!enemy)
		return;

	RpgEnemyIface *iface = dynamic_cast<RpgEnemyIface*>(enemy);

	if (!iface)
		return;

	for (TiledWeapon *w : iface->throwableWeapons()) {
		if (RpgPickableWeaponIface *wIface = dynamic_cast<RpgPickableWeaponIface*>(w)) {
			const RpgPickableObject::PickableType &wType = wIface->toPickable();
			const RpgPickableObject::PickableType &bType = wIface->toBulletPickable();

			if (w->canThrowBullet() && bType != RpgPickableObject::PickableInvalid) {
				if (RpgPickableObject *pickable = createPickable(bType, enemy->scene())) {
					pickable->body()->emplace(iface->getPickablePosition());
					enemy->scene()->appendToObjects(pickable);
					pickable->setIsActive(true);
				}
			}

			if (w->canThrow() && wType != RpgPickableObject::PickableInvalid) {
				if (RpgPickableObject *pickable = createPickable(wType, enemy->scene())) {
					pickable->body()->emplace(iface->getPickablePosition());
					enemy->scene()->appendToObjects(pickable);
					pickable->setIsActive(true);
				}
				iface->throwWeapon(w);
			}
		}
	}

	const int &count = recalculateEnemies();

	if (!count)
		emit gameSuccess();
}






/**
 * @brief RpgGame::canAttack
 * @param player
 * @param enemy
 * @return
 */

bool RpgGame::canAttack(RpgPlayer *player, IsometricEnemy *enemy, const TiledWeapon::WeaponType &weaponType)
{
	if (!player || !enemy)
		return false;

	return true;
}





/**
 * @brief RpgGame::canAttack
 * @param enemy
 * @param player
 * @return
 */

bool RpgGame::canAttack(IsometricEnemy *enemy, RpgPlayer *player, const TiledWeapon::WeaponType &weaponType)
{
	if (!player || !enemy)
		return false;


	return true;
}


/**
 * @brief RpgGame::canTransport
 * @param player
 * @param transport
 * @return
 */

bool RpgGame::canTransport(RpgPlayer *player, TiledTransport *transport)
{
	if (!player || !transport)
		return false;


	return true;
}



/**
 * @brief RpgGame::createEnemy
 * @param type
 * @param subtype
 * @param game
 * @param scene
 * @return
 */

IsometricEnemy *RpgGame::createEnemy(const RpgEnemyIface::RpgEnemyType &type, const QString &subtype, TiledScene *scene)
{
	IsometricEnemy *enemy = nullptr;

	switch (type) {
		case RpgEnemyIface::EnemyWerebear: {
			RpgWerebear *e = nullptr;
			TiledObjectBase::createFromCircle<RpgWerebear>(&e, QPointF{}, 50, nullptr, this);
			e->setWerebearType(subtype);
			enemy = e;
			break;
		}

		case RpgEnemyIface::EnemyTest: {
			RpgEnemyBase *e = nullptr;
			TiledObjectBase::createFromCircle<RpgEnemyBase>(&e, QPointF{}, 50, nullptr, this);
			//e->setWerebearType(subtype);
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
 * @brief RpgGame::createPickable
 * @param type
 * @param scene
 * @return
 */

RpgPickableObject *RpgGame::createPickable(const RpgPickableObject::PickableType &type, TiledScene *scene)
{
	RpgPickableObject *pickable = nullptr;

	switch (type) {
		case RpgPickableObject::PickableShield:
			pickable = RpgPickableObject::createPickable<RpgShieldPickable>(this);
			break;

		case RpgPickableObject::PickableHp:
			pickable = RpgPickableObject::createPickable<RpgHpPickable>(this);
			break;

		case RpgPickableObject::PickableArrow:
			pickable = RpgPickableObject::createPickable<RpgArrowPickable>(this);
			break;

		case RpgPickableObject::PickableFireball:
			pickable = RpgPickableObject::createPickable<RpgFireballPickable>(this);
			break;

		case RpgPickableObject::PickableShortbow:
			pickable = RpgPickableObject::createPickable<RpgShortbowPickable>(this);
			break;

		case RpgPickableObject::PickableLongbow:
			pickable = RpgPickableObject::createPickable<RpgLongbowPickable>(this);
			break;

		case RpgPickableObject::PickableLongsword:
			pickable = RpgPickableObject::createPickable<RpgLongswordPickable>(this);
			break;

		case RpgPickableObject::PickableInvalid:
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
 * @brief RpgGame::loadObjectLayer
 * @param scene
 * @param object
 * @param renderer
 */

void RpgGame::loadObjectLayer(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(scene);
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
 * @brief RpgGame::loadGroupLayer
 * @param scene
 * @param group
 * @param renderer
 */

void RpgGame::loadGroupLayer(TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(scene);
	Q_ASSERT(group);
	Q_ASSERT(renderer);

	std::unique_ptr<RpgControlGroup> g(RpgControlGroup::fromGroupLayer(this, scene, group, renderer));

	if (g)
		m_controlGroups.push_back(std::move(g));
}




/**
 * @brief RpgGame::joystickStateEvent
 * @param newJoystickState
 */

void RpgGame::joystickStateEvent(const JoystickState &state)
{
	if (m_controlledPlayer)
		m_controlledPlayer->onJoystickStateChanged(state);
}


/**
 * @brief RpgGame::keyPressEvent
 * @param event
 */

void RpgGame::keyPressEvent(QKeyEvent *event)
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
				m_controlledPlayer->armory()->nextWeapon();
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


		case Qt::Key_F2: {
			int idx = m_players.indexOf(m_controlledPlayer);
			++idx;
			if (idx >= m_players.size())
				idx = 0;
			setFollowedItem(m_players.at(idx));
			setControlledPlayer(m_players.at(idx));
		}
			break;


		default:
			TiledGame::keyPressEvent(event);
			break;
	}
}


/**
 * @brief RpgGame::transportAfterEvent
 * @param object
 * @param newScene
 * @param newObject
 * @return
 */

bool RpgGame::transportAfterEvent(TiledObject *object, TiledScene */*newScene*/, TiledObjectBase *newObject)
{
	RpgPlayer *player = qobject_cast<RpgPlayer*>(object);

	if (player) {
		player->setCurrentSceneStartPosition(newObject->body()->bodyPosition());
	}

	return true;
}




/**
 * @brief RpgGame::onGameQuestionSuccess
 * @param answer
 */

void RpgGame::onGameQuestionSuccess(const QVariantMap &answer)
{
	if (m_rpgQuestion)
		m_rpgQuestion->questionSuccess(answer);

	Application::instance()->client()->sound()->playSound(QStringLiteral("qrc:/sound/sfx/correct.mp3"), Sound::SfxChannel);
	Application::instance()->client()->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/winner.mp3"), Sound::VoiceoverChannel);
}



/**
 * @brief RpgGame::onGameQuestionFailed
 * @param answer
 */

void RpgGame::onGameQuestionFailed(const QVariantMap &answer)
{
#ifndef Q_OS_WASM
	StandaloneClient *client = qobject_cast<StandaloneClient*>(Application::instance()->client());
	if (client)
		client->performVibrate();
#endif

	if (m_rpgQuestion)
		m_rpgQuestion->questionFailed(answer);

	Application::instance()->client()->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/loser.mp3"), Sound::VoiceoverChannel);
}


/**
 * @brief RpgGame::onGameQuestionStarted
 */

void RpgGame::onGameQuestionStarted()
{
	Application::instance()->client()->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/fight.mp3"), Sound::VoiceoverChannel);
}



/**
 * @brief RpgGame::onGameQuestionFinished
 */

void RpgGame::onGameQuestionFinished()
{
	if (m_rpgQuestion)
		m_rpgQuestion->questionFinished();

	this->forceActiveFocus(Qt::OtherFocusReason);
}





/**
 * @brief RpgGame::recalculateEnemies
 * @return
 */

int RpgGame::recalculateEnemies()
{
	int c = 0;
	for (const EnemyData &e : m_enemyDataList) {
		if (e.enemy && e.enemy->isAlive())
			++c;
	}
	setEnemyCount(c);
	return c;
}





/**
 * @brief RpgGame::getEnemyData
 * @param enemy
 * @return
 */

std::optional<RpgGame::EnemyData> RpgGame::getEnemyData(IsometricEnemy *enemy) const
{
	if (auto it = std::find_if(m_enemyDataList.constBegin(), m_enemyDataList.constEnd(),
							   [enemy](const EnemyData &e)	{
							   return e.enemy == enemy; }
							   ); it != m_enemyDataList.constEnd()) {
		return *it;
	}

	return std::nullopt;
}



/**
 * @brief RpgGame::enemyCount
 * @return
 */

int RpgGame::enemyCount() const
{
	return m_enemyCount;
}

void RpgGame::setEnemyCount(int newEnemyCount)
{
	if (m_enemyCount == newEnemyCount)
		return;
	m_enemyCount = newEnemyCount;
	emit enemyCountChanged();
}

RpgQuestion *RpgGame::rpgQuestion() const
{
	return m_rpgQuestion;
}

void RpgGame::setRpgQuestion(RpgQuestion *newRpgQuestion)
{
	m_rpgQuestion = newRpgQuestion;
}



/**
 * @brief RpgGame::gameQuestion
 * @return
 */

GameQuestion *RpgGame::gameQuestion() const
{
	return m_gameQuestion;
}

void RpgGame::setGameQuestion(GameQuestion *newGameQuestion)
{
	if (m_gameQuestion == newGameQuestion)
		return;
	m_gameQuestion = newGameQuestion;
	emit gameQuestionChanged();

	if (m_gameQuestion) {
		connect(m_gameQuestion, &GameQuestion::success, this, &RpgGame::onGameQuestionSuccess);
		connect(m_gameQuestion, &GameQuestion::failed, this, &RpgGame::onGameQuestionFailed);
		connect(m_gameQuestion, &GameQuestion::finished, this, &RpgGame::onGameQuestionFinished);
		connect(m_gameQuestion, &GameQuestion::started, this, &RpgGame::onGameQuestionStarted);
	}
}






/**
 * @brief RpgGame::players
 * @return
 */

QList<RpgPlayer *> RpgGame::players() const
{
	return m_players;
}

void RpgGame::setPlayers(const QList<RpgPlayer *> &newPlayers)
{
	if (m_players == newPlayers)
		return;
	m_players = newPlayers;
	emit playersChanged();
}




/**
 * @brief RpgGame::getAttackSprite
 * @param weaponType
 * @return
 */

QString RpgGame::getAttackSprite(const TiledWeapon::WeaponType &weaponType)
{
	switch (weaponType) {
		case TiledWeapon::WeaponHand:
		case TiledWeapon::WeaponGreatHand:
		case TiledWeapon::WeaponLongsword:
			return QStringLiteral("attack");

		case TiledWeapon::WeaponShortbow:
		case TiledWeapon::WeaponLongbow:
			return QStringLiteral("bow");

		case TiledWeapon::WeaponShield:
		case TiledWeapon::WeaponInvalid:
			break;
	}

	return {};
}



/**
 * @brief RpgGame::resurrectEnemiesAndPlayer
 * @param player
 */

void RpgGame::resurrectEnemiesAndPlayer(RpgPlayer *player)
{
	if (!player)
		return;

	TiledScene *scene = player->scene();

	if (!scene) {
		LOG_CERROR("game") << "Missing player's scene" << player;
		return;
	}

	player->body()->emplace(player->currentSceneStartPosition());
	player->setHp(player->maxHp());

	QTimer::singleShot(2000, this, [s = QPointer<TiledScene>(scene), this](){ this->resurrectEnemies(s); });
}



/**
 * @brief RpgGame::resurrectEnemies
 * @param scene
 */

void RpgGame::resurrectEnemies(const QPointer<TiledScene> &scene)
{
	if (!scene) {
		LOG_CERROR("game") << "Missing scene" << scene;
		return;
	}

	for (EnemyData &e : m_enemyDataList) {
		if (e.scene == scene && e.enemy)
			e.enemy->setHp(e.enemy->maxHp());
	}

	recalculateEnemies();
}



/**
 * @brief RpgGame::controlledPlayer
 * @return
 */

RpgPlayer *RpgGame::controlledPlayer() const
{
	return m_controlledPlayer;
}

void RpgGame::setControlledPlayer(RpgPlayer *newControlledPlayer)
{
	if (m_controlledPlayer == newControlledPlayer)
		return;

	if (m_controlledPlayer)
		m_controlledPlayer->onJoystickStateChanged({});

	m_controlledPlayer = newControlledPlayer;
	emit controlledPlayerChanged();
}




