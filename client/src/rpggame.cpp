/*
 * ---- Call of Suli ----
 *
 * rpggame.cpp
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
#include "rpgcontrolgroupcontainer.h"
#include "rpgcontrolgroupdoor.h"
#include "rpgcontrolgroupoverlay.h"
#include "rpgcontrolgroupsave.h"
#include "rpgenemybase.h"
#include "rpgfireball.h"
#include "rpggame.h"
#include "rpghp.h"
#include "rpgkeypickable.h"
#include "rpglightning.h"
#include "rpglongbow.h"
#include "rpglongsword.h"
#include "rpgmp.h"
#include "rpgshield.h"
#include "rpgtimepickable.h"
#include "rpgwerebear.h"
#include "rpgcoin.h"
#include "rpguserwallet.h"
#include "sound.h"
#include "application.h"
#include "rpgquestion.h"
#include "utils_.h"
#include "rpgcontainer.h"
#include "tiledspritehandler.h"
#include "tileddebugdraw.h"
#include <libtiled/imagelayer.h>

#include <libtcod/path.hpp>
#include <libtcod/fov.hpp>

#ifndef Q_OS_WASM
#include "rpgcoin.h"
#include "standaloneclient.h"
#endif


/// Static hash

QHash<QString, RpgGameDefinition> RpgGame::m_terrains;
QHash<QString, RpgPlayerCharacterConfig> RpgGame::m_characters;

const QHash<QString, RpgGameData::Enemy::EnemyType> RpgEnemyIface::m_typeHash = {
	{ QStringLiteral("werebear"), RpgGameData::Enemy::EnemyWerebear },
	{ QStringLiteral("soldier"), RpgGameData::Enemy::EnemySoldier },
	{ QStringLiteral("soldierFix"), RpgGameData::Enemy::EnemySoldierFix },
	{ QStringLiteral("archer"), RpgGameData::Enemy::EnemyArcher },
	{ QStringLiteral("archerFix"), RpgGameData::Enemy::EnemyArcherFix },
	{ QStringLiteral("skeleton"), RpgGameData::Enemy::EnemySkeleton },
	{ QStringLiteral("smith"), RpgGameData::Enemy::EnemySmith },
	{ QStringLiteral("smithFix"), RpgGameData::Enemy::EnemySmithFix },
	{ QStringLiteral("barbarian"), RpgGameData::Enemy::EnemyBarbarian },
	{ QStringLiteral("barbarianFix"), RpgGameData::Enemy::EnemyBarbarianFix },
	{ QStringLiteral("butcher"), RpgGameData::Enemy::EnemyButcher },
	{ QStringLiteral("butcherFix"), RpgGameData::Enemy::EnemyButcherFix },
};





/**
 * @brief RpgGame::RpgGame
 * @param parent
 */

RpgGame::RpgGame(QQuickItem *parent)
	: TiledGame(parent)
{
	loadMetricDefinition();
}


/**
 * @brief RpgGame::~RpgGame
 */

RpgGame::~RpgGame()
{
	m_controlGroups.clear();
	m_enemyDataList.clear();
	m_players.clear();
}






/**
 * @brief RpgGame::load
 * @return
 */


bool RpgGame::load(const RpgGameDefinition &def, const bool &replaceMpToShield)
{
	if (!TiledGame::load(def))
		return false;

	if (!def.minVersion.isEmpty()) {
		const QVersionNumber v = QVersionNumber::fromString(def.minVersion).normalized();

		if (!v.isNull() && v > Utils::versionNumber()) {
			LOG_CWARNING("game") << "Required version:" << v.majorVersion() << v.minorVersion();
			emit gameLoadFailed(tr("Szükséges verzió: %1.%2").arg(v.majorVersion()).arg(v.minorVersion()));
			return false;
		}
	}

	for (auto &e : m_enemyDataList) {
		Q_ASSERT(!e.path.isEmpty());

		// Replace pickables if character has no cast

		if (replaceMpToShield) {
			for (auto &p : e.pickables) {
				if (p == RpgGameData::Pickable::PickableMp)
					p = RpgGameData::Pickable::PickableShield;
			}

			for (auto &p : e.pickablesOnce) {
				if (p == RpgGameData::Pickable::PickableMp)
					p = RpgGameData::Pickable::PickableShield;
			}
		}

		IsometricEnemy *enemy = createEnemy(e.type, e.subtype, e.scene, e.objectId.id);

		if (!enemy)
			continue;

		if (!e.displayName.isEmpty()) {
			enemy->setDisplayName(e.displayName);
		}

		enemy->createMarkerItem(QStringLiteral("qrc:/TiledEnemyMarker.qml"));

		if (e.path.size() > 1)
			enemy->loadPathMotor(e.path);
		else
			enemy->loadFixPositionMotor(e.path.first(),
										enemy->nearestDirectionFromRadian(TiledObject::toRadian(e.defaultAngle)));


		e.enemy = enemy;
	}

	recalculateEnemies();

	for (auto &e : m_pickableDataList) {

		if (e.type == RpgGameData::Pickable::PickableMp)
			e.type = RpgGameData::Pickable::PickableShield;

		RpgPickableObject *pickable = createPickable(e.type, e.name, e.scene);

		if (!pickable)
			continue;

		pickable->setName(e.name);
		pickable->emplace(e.position);

		if (!e.displayName.isEmpty()) {
			pickable->setDisplayName(e.displayName);
			pickable->createMarkerItem();
		}

		e.pickableObject = pickable;
		pickable->setIsActive(true);
	}

	m_gameDefinition = def;



	return true;
}




/**
 * @brief RpgGame::readGameDefinition
 * @param map
 * @return
 */

std::optional<RpgGameDefinition> RpgGame::readGameDefinition(const QString &map)
{
	if (map.isEmpty())
		return std::nullopt;

	const auto &ptr = Utils::fileToJsonObject(QStringLiteral(":/map/%1/game.json").arg(map));

	if (!ptr)
		return std::nullopt;

	RpgGameDefinition def;
	def.fromJson(ptr.value());

	def.basePath = QStringLiteral("qrc:/map/").append(map);

	return def;
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

	if (p->isLocked())
		return false;

	if (iface->protectWeapon(weaponType))
		return false;

	if (m_funcPlayerAttackEnemy)
		return m_funcPlayerAttackEnemy(p, e, weaponType);
	else
		return false;
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

	if (m_funcEnemyAttackPlayer)
		return m_funcEnemyAttackPlayer(e, p, weaponType);
	else
		return false;
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

	if (m_funcPlayerPick && !m_funcPlayerPick(p, object))
		return false;

	if (!object->playerPick(p))
		return false;

	object->setIsActive(false);

	p->inventoryAdd(object);

	p->armory()->updateLayers();
	//p->setShieldCount(p->armory()->getShieldCount());

	playSfx(QStringLiteral(":/rpg/common/leather_inventory.mp3"), player->scene(), player->bodyPosition());


	return true;
}





/**
 * @brief RpgGame::playerUseCast
 * @param player
 * @return
 */

bool RpgGame::playerUseCast(RpgPlayer *player)
{
	Q_ASSERT(player);

	if (player->isLocked())
		return false;

	if (m_funcPlayerUseCast)
		return m_funcPlayerUseCast(player);
	else
		return false;
}


/**
 * @brief RpgGame::playerFinishCast
 * @param player
 * @return
 */

bool RpgGame::playerFinishCast(RpgPlayer *player)
{
	Q_ASSERT(player);

	if (m_funcPlayerFinishCast)
		return m_funcPlayerFinishCast(player);
	else
		return false;
}



/**
 * @brief RpgGame::saveSceneState
 * @param player
 */

void RpgGame::saveSceneState(RpgPlayer *player)
{
	if (!player)
		return;

	messageColor(tr("State saved"), QColor::fromRgbF(0., 0.9, 0.));

	playSfx(QStringLiteral(":/rpg/common/click.mp3"), player->scene(), player->bodyPosition());

	saveSceneState();
}




/**
 * @brief RpgGame::saveSceneState
 */

void RpgGame::saveSceneState()
{
	for (EnemyData &e : m_enemyDataList) {
		if (!e.enemy)
			continue;

		if (e.enemy->hp() <= 0)
			e.dieForever = true;
	}

	recalculateEnemies();
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

		isoPlayer->clearData();

		playerFinishCast(isoPlayer);
	}

	for (RpgQuest &q : m_gameDefinition.quests) {
		if (q.type == RpgQuest::SuddenDeath)
			q.success = -1;
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

	int num = 0;

	if (RpgEnemyIface *iface = dynamic_cast<RpgEnemyIface*>(enemy)) {
		if (IsometricEnemy *isoEnemy = qobject_cast<IsometricEnemy*>(enemy)) {
			if (auto it = enemyFind(isoEnemy); it != m_enemyDataList.end()) {
				for (const auto &p : it->pickables) {
					if (RpgPickableObject *pickable = createPickable(p, enemy->scene())) {
						pickable->emplace(iface->getPickablePosition(num++));
						pickable->setIsActive(true);
					}
				}

				for (const auto &p : it->pickablesOnce) {
					if (RpgPickableObject *pickable = createPickable(p, enemy->scene())) {
						pickable->emplace(iface->getPickablePosition(num++));
						pickable->setIsActive(true);
					}
				}

				it->pickablesOnce.clear();

				it->hasQuestion = false;
			}

			// on all of enemies in the same scene are dead -> set die forever

			QVector<EnemyData*> eList;
			bool isAllDead = true;

			for (auto it = m_enemyDataList.begin(); it != m_enemyDataList.end(); ++it) {
				if (it->enemy && it->enemy->scene() == isoEnemy->scene()) {
					eList.append(&*it);
					if (it->enemy->isAlive())
						isAllDead = false;
				}
			}

			if (isAllDead) {
				for (EnemyData *e : eList)
					e->dieForever = true;
			}
		}
	}


	const int &count = recalculateEnemies();

	if (!count)
		emit gameSuccess();
}


/**
 * @brief RpgGame::onEnemySleepingStart
 * @param enemy
 */

void RpgGame::onEnemySleepingStart(TiledObject *enemy)
{
	Q_UNUSED(enemy);
}



/**
 * @brief RpgGame::onEnemySleepingEnd
 * @param enemy
 */

void RpgGame::onEnemySleepingEnd(TiledObject *enemy)
{
	Q_UNUSED(enemy);
}








/**
 * @brief RpgGame::playerTryUseContainer
 * @param container
 * @return
 */

bool RpgGame::playerTryUseContainer(RpgPlayer *player, RpgContainer *container)
{
	if (!player || !container) {
		if (player && !player->m_sfxDecline.soundList().isEmpty())
			player->m_sfxDecline.playOne();
		messageColor(tr("Locked"), QColor::fromRgbF(0.8, 0., 0.));
		return false;
	}

	if (m_funcPlayerUseContainer)
		return m_funcPlayerUseContainer(player, container);
	else
		return false;
}


/**
 * @brief RpgGame::playerUseContainer
 * @param player
 * @param container
 */

void RpgGame::playerUseContainer(RpgPlayer *player, RpgContainer *container)
{
	if (!container)
		return;

	container->setIsActive(false);

	if (player)
		player->setCurrentContainer(nullptr);

	if (!container->scene())
		return;


	const auto &it = std::find_if(m_controlGroups.cbegin(), m_controlGroups.cend(),
								  [container](const std::unique_ptr<RpgControlGroup> &ptr) {
		RpgControlGroupContainer *c = dynamic_cast<RpgControlGroupContainer*>(ptr.get());
		if (!c)
			return false;

		return c->rpgContainer() == container;
	}
	);

	if (it == m_controlGroups.cend()) {
		LOG_CERROR("game") << "Container not found" << container;
		return;
	}

	if (RpgControlGroupContainer *container = dynamic_cast<RpgControlGroupContainer*>(it->get())) {
		const auto &pList = container->pickableList();

		int i = 0;
		static const int delta = 10;
		const int &count = pList.size();

		QPointF startPos = container->centerPoint();

		if (count > 1) {
			startPos.setX(startPos.x()-(count/2)*delta);
		}

		for (const auto &pType : pList) {
			RpgPickableObject *pickable = createPickable(pType, container->scene());
			if (!pickable)
				continue;

			QPointF pos = startPos;
			pos.setX(pos.x()+(i*delta));

			++i;

			pickable->emplace(pos);
			pickable->setIsActive(true);
		}

		container->setPickableList({});
	}
}



/**
 * @brief RpgGame::createEnemy
 * @param type
 * @param subtype
 * @param game
 * @param scene
 * @return
 */

IsometricEnemy *RpgGame::createEnemy(const RpgGameData::Enemy::EnemyType &type, const QString &subtype, TiledScene *scene, const int &id)
{
	IsometricEnemy *enemy = nullptr;

	b2::Body::Params bParams;
	bParams.type = b2BodyType::b2_kinematicBody;
	bParams.fixedRotation = true;

	b2::Shape::Params params;
	params.density = 1.f;
	params.friction = 1.f;
	params.restitution = 0.f;
	params.enableContactEvents = true;
	params.enableSensorEvents = true;
	params.filter = TiledObjectBody::getFilter(TiledObjectBody::FixtureEnemyBody,
											   TiledObjectBody::FixtureGround |
											   TiledObjectBody::FixturePlayerBody |
											   TiledObjectBody::FixtureTarget);

	static const float size = 15.;

	switch (type) {
		case RpgGameData::Enemy::EnemyWerebear: {
			enemy = createFromCircle<RpgWerebear>(-1, id, scene, {0.f, 0.f}, size, nullptr, bParams, params);
			break;
		}

		case RpgGameData::Enemy::EnemySoldier:
		case RpgGameData::Enemy::EnemyArcher:
		case RpgGameData::Enemy::EnemySoldierFix:
		case RpgGameData::Enemy::EnemyArcherFix:
		case RpgGameData::Enemy::EnemySmith:
		case RpgGameData::Enemy::EnemySmithFix:
		case RpgGameData::Enemy::EnemyButcher:
		case RpgGameData::Enemy::EnemyButcherFix:
		case RpgGameData::Enemy::EnemyBarbarian:
		case RpgGameData::Enemy::EnemyBarbarianFix:
		case RpgGameData::Enemy::EnemySkeleton:
		{
			if (RpgEnemyBase *e = createFromCircle<RpgEnemyBase>(-1, id, scene, {0.f, 0.f}, size, nullptr, bParams, params)) {
				e->m_enemyType = type;
				e->setSubType(subtype);
				enemy = e;
			}
			break;
		}

		case RpgGameData::Enemy::EnemyInvalid:
			LOG_CERROR("game") << "Invalid enemy type" << type;
			return nullptr;
	}

	if (enemy) {
		enemy->setMetric(getMetric(enemy->m_metric, type, subtype));

		if (type == RpgGameData::Enemy::EnemySoldierFix ||
				type == RpgGameData::Enemy::EnemyArcherFix ||
				type == RpgGameData::Enemy::EnemySmithFix ||
				type == RpgGameData::Enemy::EnemyButcherFix ||
				type == RpgGameData::Enemy::EnemyBarbarianFix
				) {
			enemy->m_metric.pursuitSpeed = 0;
		}

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

RpgPickableObject *RpgGame::createPickable(const RpgGameData::Pickable::PickableType &type, const QString &name,
										   TiledScene *scene, const int &ownerId, const int &id)
{
	b2::Body::Params bParams;
	bParams.type = b2BodyType::b2_staticBody;
	bParams.fixedRotation = true;

	b2::Shape::Params params;
	params.density = 0.f;
	params.friction = 0.f;
	params.restitution = 0.f;
	params.isSensor = true;
	params.filter = TiledObjectBody::getFilter(TiledObjectBody::FixturePickable,
											   TiledObjectBody::FixturePlayerBody |
											   TiledObjectBody::FixtureSensor |
											   TiledObjectBody::FixtureVirtualCircle);


	static const float size = 30.;

	RpgPickableObject *pickable = nullptr;

	switch (type) {
		case RpgGameData::Pickable::PickableShield:
			pickable = createFromCircle<RpgShieldPickable>(ownerId, id, scene, {0.f, 0.f}, size, nullptr, bParams, params);
			break;

		case RpgGameData::Pickable::PickableHp:
			pickable = createFromCircle<RpgHpPickable>(ownerId, id, scene, {0.f, 0.f}, size, nullptr, bParams, params);
			break;

		case RpgGameData::Pickable::PickableMp:
			pickable = createFromCircle<RpgMpPickable>(ownerId, id, scene, {0.f, 0.f}, size, nullptr, bParams, params);
			break;

		case RpgGameData::Pickable::PickableCoin:
			pickable = createFromCircle<RpgCoinPickable>(ownerId, id, scene, {0.f, 0.f}, size, nullptr, bParams, params);
			break;

		case RpgGameData::Pickable::PickableShortbow:
			pickable = createFromCircle<RpgShortbowPickable>(ownerId, id, scene, {0.f, 0.f}, size, nullptr, bParams, params);
			break;

		case RpgGameData::Pickable::PickableLongbow:
			pickable = createFromCircle<RpgLongbowPickable>(ownerId, id, scene, {0.f, 0.f}, size, nullptr, bParams, params);
			break;

		case RpgGameData::Pickable::PickableLongsword:
			pickable = createFromCircle<RpgLongswordPickable>(ownerId, id, scene, {0.f, 0.f}, size, nullptr, bParams, params);
			break;

		case RpgGameData::Pickable::PickableTime:
			pickable = createFromCircle<RpgTimePickable>(ownerId, id, scene, {0.f, 0.f}, size, nullptr, bParams, params);
			break;

		case RpgGameData::Pickable::PickableKey:
			pickable = createFromCircle<RpgKeyPickable>(ownerId, id, scene, {0.f, 0.f}, size, nullptr, bParams, params);
			break;

		case RpgGameData::Pickable::PickableDagger:
		case RpgGameData::Pickable::PickableInvalid:
			break;
	}

	if (pickable) {
		pickable->setName(name);
		pickable->initialize();
	}

	return pickable;
}


/**
 * @brief RpgGame::createBullet
 * @param type
 * @param scene
 * @param id
 * @param ownerId
 * @param weapon
 * @return
 */

IsometricBullet *RpgGame::createBullet(const TiledWeapon::WeaponType &type, TiledScene *scene, const int &id, const int &ownerId,
									   TiledWeapon *weapon)
{
	b2::Body::Params bParams;
	bParams.type = b2BodyType::b2_dynamicBody;
	bParams.fixedRotation = true;
	bParams.isBullet = true;

	b2::Shape::Params params;
	params.density = 1.f;
	params.friction = 1.f;
	params.restitution = 0.f;
	params.isSensor = false;
	params.enableSensorEvents = true;
	params.enableContactEvents = true;
	params.filter = TiledObjectBody::getFilter(TiledObjectBody::FixtureBulletBody,
											   TiledObjectBody::FixtureTarget |
											   TiledObjectBody::FixtureGround);



	static const float size = 10.;

	IsometricBullet *bullet = nullptr;

	switch (type) {
		case TiledWeapon::WeaponShortbow:
			bullet = createFromCircle<RpgArrow>(ownerId, id, scene, {0.f, 0.f}, size, nullptr, bParams, params);
			break;

		case TiledWeapon::WeaponLongbow:
			bullet = createFromCircle<RpgFireball>(ownerId, id, scene, {0.f, 0.f}, size, nullptr, bParams, params);
			break;

		case TiledWeapon::WeaponLightningWeapon:
			bullet = createFromCircle<RpgLightning>(ownerId, id, scene, {0.f, 0.f}, size, nullptr, bParams, params);
			break;

		case TiledWeapon::WeaponHand:
		case TiledWeapon::WeaponDagger:
		case TiledWeapon::WeaponLongsword:
		case TiledWeapon::WeaponBroadsword:
		case TiledWeapon::WeaponAxe:
		case TiledWeapon::WeaponHammer:
		case TiledWeapon::WeaponMace:
		case TiledWeapon::WeaponGreatHand:
		case TiledWeapon::WeaponMageStaff:
		case TiledWeapon::WeaponFireFogWeapon:
		case TiledWeapon::WeaponShield:
		case TiledWeapon::WeaponInvalid:
			break;

	}

	if (bullet) {
		bullet->initialize(weapon);
	}

	return bullet;
}


/**
 * @brief RpgGame::createBullet
 * @param weapon
 * @param scene
 * @param id
 * @param ownerId
 * @return
 */

IsometricBullet *RpgGame::createBullet(TiledWeapon *weapon, TiledScene *scene, const int &id, const int &ownerId)
{
	Q_ASSERT(weapon);
	return createBullet(weapon->weaponType(), scene, id, ownerId, weapon);
}



/**
 * @brief RpgGame::shot
 * @param owner
 * @param targets
 * @param angle
 */

bool RpgGame::shot(TiledObject *owner, TiledWeapon *weapon, TiledScene *scene, const IsometricBullet::Targets &targets, const qreal &angle)
{
	LOG_CDEBUG("game") << "Shot" << owner << weapon << targets;

	if (!weapon || !owner)
		return false;

	if (!weapon->shot())
		return false;

	// TODO: playerID
	IsometricBullet *bullet = createBullet(weapon, scene, 0, 0);

	if (!bullet) {
		LOG_CWARNING("game") << "Can't create bullet";
		return false;
	}

	connect(bullet, &IsometricBullet::autoDeleteRequest, this, &RpgGame::removeObject);

	weapon->setBulletCount(weapon->bulletCount()-1);

	bullet->setTargets(targets);
	bullet->setMaxDistance(weapon->bulletDistance());
	bullet->shot(targets, owner->bodyPosition(), angle);

	return true;
}


/**
 * @brief RpgGame::transportPlayer
 * @return
 */

bool RpgGame::transportPlayer()
{
	if (!m_controlledPlayer)
		return false;


	if (TiledTransport *t = m_controlledPlayer->currentTransport()) {
		const bool s = transport(m_controlledPlayer, t, m_controlledPlayer->currentTransportBase());

		if (!s) {
			if (!m_controlledPlayer->m_sfxDecline.soundList().isEmpty())
				m_controlledPlayer->m_sfxDecline.playOne();
			messageColor(tr("Locked"), QColor::fromRgbF(0.8, 0., 0.));

		} else if (t->type() == TiledTransport::TransportMarket) {
			return true;

		} else {
			for (const EnemyData &e : m_enemyDataList) {
				if (e.enemy)
					e.enemy->removeContactedPlayer(m_controlledPlayer);
			}

			m_controlledPlayer->clearDestinationPoint();

			return true;
		}
	}

	return false;
}



/**
 * @brief RpgGame::useContainer
 * @return
 */

bool RpgGame::useContainer()
{
	if (!m_controlledPlayer)
		return false;

	return playerTryUseContainer(m_controlledPlayer, m_controlledPlayer->currentContainer());
}






/**
 * @brief RpgGame::loadObjectLayer
 * @param scene
 * @param object
 * @param renderer
 */

void RpgGame::loadObjectLayer(TiledScene *scene, Tiled::MapObject *object, const QString &groupClass, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(scene);
	Q_ASSERT(object);
	Q_ASSERT(renderer);

	if (groupClass == QStringLiteral("enemy"))
		return loadEnemy(scene, object, renderer);
	else if (groupClass == QStringLiteral("pickable"))
		return loadPickable(scene, object, renderer);
}


/**
 * @brief RpgGame::loadImageLayer
 * @param scene
 * @param image
 * @param renderer
 */

void RpgGame::loadImageLayer(TiledScene *scene, Tiled::ImageLayer *image, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(scene);
	Q_ASSERT(image);
	Q_ASSERT(renderer);

	LOG_CDEBUG("scene") << "Add image layer" << image->imageSource() << image->name();

	scene->addVisualItem(image);
}




/**
 * @brief RpgGame::loadGround
 * @param scene
 * @param object
 * @param renderer
 * @return
*/

TiledObjectBody *RpgGame::loadGround(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer)
{
	TiledObjectBody *p = TiledGame::loadGround(scene, object, renderer);

	if (object->hasProperty(QStringLiteral("sound")) && p) {
		addLocationSound(p, object->propertyAsString(QStringLiteral("sound")));
	}

	return p;
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
	if (m_paused)
		return;

	if (m_controlledPlayer)
		m_controlledPlayer->onJoystickStateChanged(state);
}



/**
 * @brief RpgGame::keyPressEvent
 * @param event
 */

void RpgGame::keyPressEvent(QKeyEvent *event)
{
	if (m_paused)
		return;

	const int &key = event->key();

	switch (key) {
		case Qt::Key_X:
		case Qt::Key_Clear:
		case Qt::Key_5:
			transportPlayer();
			break;

		case Qt::Key_Space:
		case Qt::Key_Insert:
		case Qt::Key_0:
			if (m_controlledPlayer)
				m_controlledPlayer->attackCurrentWeapon();
			break;

		case Qt::Key_Q:
		case Qt::Key_Delete:
		case Qt::Key_Comma:
			if (m_controlledPlayer)
				m_controlledPlayer->armory()->changeToNextWeapon();
			break;

		case Qt::Key_Return:
		case Qt::Key_Enter:
		case Qt::Key_E:
			if (m_controlledPlayer)
				m_controlledPlayer->useCurrentObjects();
			break;


		case Qt::Key_C:
			if (m_controlledPlayer)
				m_controlledPlayer->cast();
			break;

		case Qt::Key_Tab:
			emit minimapToggleRequest();
			break;

		case Qt::Key_F10:
			emit questsRequest();
			break;

#ifndef QT_NO_DEBUG
		case Qt::Key_M:
			emit marketRequest();
			break;

		case Qt::Key_F:
			if (m_controlledPlayer)
				m_controlledPlayer->setIsLocked(!m_controlledPlayer->isLocked());
			break;


		case Qt::Key_N:
			if (event->modifiers().testFlag(Qt::ShiftModifier) && event->modifiers().testFlag(Qt::ControlModifier)) {
				for (auto &e : m_enemyDataList) {
					if (e.enemy) {
						e.enemy->setHp(0);
					}
				}
			}
			break;
#endif

		default:
			TiledGame::keyPressEvent(event);
			break;
	}
}


/**
 * @brief RpgGame::transportBeforeEvent
 * @param object
 * @param transport
 * @return
 */

bool RpgGame::transportBeforeEvent(TiledObject *object, TiledTransport */*transport*/)
{
	RpgPlayer *player = qobject_cast<RpgPlayer*>(object);

	if (!player)
		return false;

	auto *s = player->scene();

	for (const auto &ptr : m_controlGroups) {
		RpgControlGroupOverlay *ol = dynamic_cast<RpgControlGroupOverlay*>(ptr.get());
		if (!ol || (s && ol->scene() != s))
			continue;

		ol->removePlayerFixture(player);
	}

	return true;
}



/**
 * @brief RpgGame::transportAfterEvent
 * @param object
 * @param newScene
 * @param newObject
 * @return
 */

bool RpgGame::transportAfterEvent(TiledObject *object, TiledScene */*newScene*/, TiledObject *newObject)
{
	RpgPlayer *player = qobject_cast<RpgPlayer*>(object);

	if (player) {
		player->setCurrentSceneStartPosition(newObject->bodyPosition());
		player->m_currentTransportBase = newObject;
	}

	return true;
}



/**
 * @brief RpgGame::transportDoor
 * @param object
 * @param transport
 * @return
 */

bool RpgGame::transportDoor(TiledObject *object, TiledTransport *transport)
{
	Q_ASSERT(object);
	Q_ASSERT(transport);

	if (!transport->isOpen())
		return false;

	const auto &it = std::find_if(m_controlGroups.cbegin(), m_controlGroups.cend(),
								  [transport](const std::unique_ptr<RpgControlGroup> &ptr) {
		RpgControlGroupDoor *door = dynamic_cast<RpgControlGroupDoor*>(ptr.get());
		if (!door)
			return false;

		return door->transport() == transport;
	}
	);

	if (it == m_controlGroups.cend()) {
		LOG_CERROR("game") << "Door not found" << transport->name();
		return false;
	}

	RpgControlGroupDoor *door = dynamic_cast<RpgControlGroupDoor*>(it->get());

	Q_ASSERT(door);

	if (door->openState() == RpgControlGroupDoor::StateOpened)
		door->doorClose();
	else if (door->openState() == RpgControlGroupDoor::StateClosed)
		door->doorOpen();
	else
		return false;

	return true;
}



/**
 * @brief RpgGame::timeSteppedEvent
 */

void RpgGame::timeSteppedEvent()
{
	if (m_funcTimeStep)
		m_funcTimeStep();

	TiledGame::timeSteppedEvent();

	updateScatterEnemies();
	updateScatterPlayers();
	updateScatterPoints();

	for (const auto &ptr : m_sfxLocations) {
		if (ptr->baseObject()->scene() != ptr->connectedScene())
			ptr->setConnectedScene(ptr->baseObject()->scene());
		ptr->checkPosition();
	}
}



/**
 * @brief RpgGame::sceneDebugDrawEvent
 * @param debugDraw
 * @param scene
 */

void RpgGame::sceneDebugDrawEvent(TiledDebugDraw *debugDraw, TiledScene *scene)
{
	TiledGame::sceneDebugDrawEvent(debugDraw, scene);

	if (!debugDraw || !scene)
		return;

	for (const auto &e : m_enemyDataList) {
		if (e.scene != scene || e.path.isEmpty())
			continue;

		if (!e.enemy || !e.enemy->isAlive())
			continue;

		if (e.enemy->m_returnPathMotor) {
			debugDraw->drawPolygon(e.enemy->m_returnPathMotor->path(),
								   e.enemy->body().GetType() == b2_kinematicBody ? QColor::fromRgb(0, 200, 0) : QColor::fromRgb(230, 0, 200),
								   3.);
		} else if (e.path.size() > 1) {
			debugDraw->drawPolygon(e.path, QColor::fromRgb(230, 0, 0), 3.);
		} else
			debugDraw->drawSolidCircle(e.path.first(), 3., QColor::fromRgb(230, 0, 0));
	}


	for (const RpgPlayer *p : m_players) {
		if (TiledPathMotor *motor = p->destinationMotor()) {
			debugDraw->drawPolygon(motor->polygon(),
								   p == m_controlledPlayer ? QColor::fromRgb(0, 230, 0) : QColor::fromRgb(230, 150, 0),
								   4.);
		}
	}
}




/**
 * @brief RpgGame::loadMetricDefinition
 */

void RpgGame::loadMetricDefinition()
{
	LOG_CTRACE("game") << "Load metric definition";

	static const QStringList levelDefList = {
		QStringLiteral(":/rpg/common/metric1.json"),
		QStringLiteral(":/rpg/common/metric2.json"),
		QStringLiteral(":/rpg/common/metric3.json"),
	};

	m_metricDefinition.clear();


	static const RpgEnemyMetricDefinition defaultMetric = defaultEnemyMetric();
	static const std::vector<const QHash<QString, EnemyMetric>*> defM = {
		&defaultMetric.soldier,
		&defaultMetric.archer,
		&defaultMetric.skeleton,
		&defaultMetric.werebear
	};
	static const QStringList keyList = {
		QStringLiteral("soldier"),
		QStringLiteral("archer"),
		QStringLiteral("skeleton"),
		QStringLiteral("werebear"),
	};


	RpgEnemyMetricDefinition metric;
	metric.playerCast = defaultMetric.playerCast;

	for (const QString &l : levelDefList) {
		if (const auto &ptr = Utils::fileToJsonObject(l); ptr.has_value()) {
			std::vector<QHash<QString, EnemyMetric>*> dst = {
				&metric.soldier,
				&metric.archer,
				&metric.skeleton,
				&metric.werebear
			};


			for (int i=0; i<keyList.size(); ++i) {
				const QJsonObject sPtr = ptr->value(keyList.at(i)).toObject();
				auto dPtr = dst.at(i);

				for (auto it=sPtr.constBegin(); it != sPtr.constEnd(); ++it) {
					auto dstIt = dPtr->find(it.key());

					EnemyMetric baseMetric = defM.at(i)->value(QStringLiteral("default"));
					baseMetric.fromJson(it.value());

					if (dstIt == dPtr->end()) {
						dPtr->insert(it.key(), baseMetric);
					} else {
						*dstIt = baseMetric;
					}
				}

			}

			RpgEnemyMetricDefinition tmp;
			tmp.fromJson(ptr.value());

			for (auto it = tmp.playerCast.cbegin(); it != tmp.playerCast.cend(); ++it) {
				if (it.value() > 0)
					metric.playerCast.insert(it.key(), it.value());
			}
		}

		m_metricDefinition.append(metric);
	}
}



/**
 * @brief RpgGame::getMetric
 * @param type
 * @param subtype
 * @return
 */

EnemyMetric RpgGame::getMetric(EnemyMetric baseMetric, const RpgGameData::Enemy::EnemyType &type, const QString &subtype)
{
	if (m_metricDefinition.isEmpty())
		return {};

	for (int i=0; i<std::max(1, m_level) && i<m_metricDefinition.size(); ++i) {
		const RpgEnemyMetricDefinition &def = m_metricDefinition.at(i);
		QHash<QString, EnemyMetric> ptr;

		switch (type) {
			case RpgGameData::Enemy::EnemyWerebear:
				ptr = def.werebear;
				break;

			case RpgGameData::Enemy::EnemySoldier:
			case RpgGameData::Enemy::EnemySoldierFix:
				ptr = def.soldier;
				break;

			case RpgGameData::Enemy::EnemyArcher:
			case RpgGameData::Enemy::EnemyArcherFix:
				ptr = def.archer;
				break;

			case RpgGameData::Enemy::EnemySmith:
			case RpgGameData::Enemy::EnemySmithFix:
				ptr = def.smith;
				break;

			case RpgGameData::Enemy::EnemyButcher:
			case RpgGameData::Enemy::EnemyButcherFix:
				ptr = def.butcher;
				break;

			case RpgGameData::Enemy::EnemyBarbarian:
			case RpgGameData::Enemy::EnemyBarbarianFix:
				ptr = def.barbarian;
				break;

			case RpgGameData::Enemy::EnemySkeleton:
				ptr = def.skeleton;
				break;

			case RpgGameData::Enemy::EnemyInvalid:
				LOG_CERROR("game") << "Invalid enemy type" << type;
				break;
		}

		if (ptr.contains(subtype))
			baseMetric = ptr.value(subtype);
		else if (ptr.contains(QStringLiteral("default")))
			baseMetric = ptr.value(QStringLiteral("default"));
	}

	return baseMetric;
}



/**
 * @brief RpgGame::createPlayer
 * @param scene
 * @param config
 * @return
 */

RpgPlayer *RpgGame::createPlayer(TiledScene *scene, const RpgPlayerCharacterConfig &config, const int &ownerId)
{
	b2::Body::Params bParams;
	bParams.type = b2BodyType::b2_dynamicBody;
	bParams.fixedRotation = true;

	b2::Shape::Params params;
	params.density = 1.f;
	params.friction = 1.f;
	params.restitution = 0.f;
	params.enableContactEvents = true;
	params.enableSensorEvents = true;
	params.filter = TiledObjectBody::getFilter(TiledObjectBody::FixturePlayerBody,
											   TiledObjectBody::FixtureCategories(TiledObjectBody::FixtureAll)
											   .setFlag(TiledObjectBody::FixturePlayerBody, false)
											   .setFlag(TiledObjectBody::FixtureVirtualCircle, false)
											   .setFlag(TiledObjectBody::FixtureSensor, false)
											   );


	RpgPlayer *player = createFromCircle<RpgPlayer>(ownerId, 1, scene, {0.f, 0.f}, 15., nullptr, bParams, params);

	if (player) {
		player->setConfig(config);
		player->initialize();
	}

	return player;
}



/**
 * @brief RpgGame::worldStep
 * @param body
 */

void RpgGame::worldStep(const Body &body)
{
	if (m_funcBodyStep)
		m_funcBodyStep(body);
	else
		TiledGame::worldStep(body);
}




/**
 * @brief RpgGame::getMetric
 * @param cast
 * @param level
 * @return
 */

int RpgGame::getMetric(const RpgPlayerCharacterConfig::CastType &cast) const
{
	int value = defaultEnemyMetric().playerCast.value(cast);

	if (m_metricDefinition.isEmpty())
		return value;

	for (int i=0; i<std::max(1, m_level) && i<m_metricDefinition.size(); ++i) {
		const int v = m_metricDefinition.at(i).playerCast.value(cast);

		if (v > 0)
			value = v;
	}

	return value;
}



/**
 * @brief RpgGame::loadEnemy
 * @param scene
 * @param object
 * @param renderer
 */

void RpgGame::loadEnemy(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(object);
	Q_ASSERT(scene);
	Q_ASSERT(renderer);

	const RpgGameData::Enemy::EnemyType &type = RpgEnemyIface::typeFromString(object->className());

	if (type == RpgGameData::Enemy::EnemyInvalid) {
		LOG_CERROR("game") << "Invalid enemy" << object->id() << object->className() << object->name();
		return;
	}

	QPolygonF p;

	if (object->shape() == Tiled::MapObject::Polygon ||
			object->shape() == Tiled::MapObject::Polyline)
		p = TiledObject::toPolygon(object, renderer);
	else if (object->shape() == Tiled::MapObject::Point)
		p << renderer->pixelToScreenCoords(object->position());


	if (p.isEmpty()) {
		LOG_CERROR("scene") << "Invalid enemy polygon" << object->id() << object->className() << object->name();
		return;
	}


	QVector<RpgGameData::Pickable::PickableType> pickableList;

	if (object->hasProperty(QStringLiteral("pickable"))) {
		pickableList = getPickablesFromPropertyValue(object->property(QStringLiteral("pickable")).toString());
	}

	QVector<RpgGameData::Pickable::PickableType> pickableOnceList;

	if (object->hasProperty(QStringLiteral("pickableOnce"))) {
		pickableOnceList = getPickablesFromPropertyValue(object->property(QStringLiteral("pickableOnce")).toString());
	}


	m_enemyDataList.append(EnemyData{
							   TiledObject::ObjectId{.sceneId = scene->sceneId(), .id = object->id()},
							   type,
							   object->name(),
							   p,
							   object->property(QStringLiteral("direction")).toInt(),
							   scene,
							   nullptr,
							   false,
							   object->property(QStringLiteral("dieForever")).toBool(),
							   pickableList,
							   pickableOnceList,
							   object->propertyAsString(QStringLiteral("displayName"))
						   });
}



/**
 * @brief RpgGame::loadPickable
 * @param scene
 * @param object
 * @param renderer
 */

void RpgGame::loadPickable(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(object);
	Q_ASSERT(scene);
	Q_ASSERT(renderer);

	const RpgGameData::Pickable::PickableType &type = RpgPickableObject::typeFromString(object->className());

	if (type == RpgGameData::Pickable::PickableInvalid) {
		LOG_CERROR("game") << "Invalid pickable" << object->id() << object->className() << object->name();
		return;
	}

	if (object->shape() != Tiled::MapObject::Point) {
		LOG_CWARNING("scene") << "Invalid pickable point" << object->id() << object->className() << object->name();
		return;
	}
	const QPointF &point = renderer->pixelToScreenCoords(object->position());

	m_pickableDataList.append(PickableData{
								  TiledObject::ObjectId{.sceneId = scene->sceneId(), .id = object->id() },
								  type,
								  object->name(),
								  point,
								  scene,
								  nullptr,
								  object->propertyAsString(QStringLiteral("displayName"))
							  });
}



/**
 * @brief RpgGame::addLocationSound
 * @param object
 * @param sound
 * @param baseVolume
 * @param channel
 */

void RpgGame::addLocationSound(TiledObjectBody *object, const QString &sound, const qreal &baseVolume, const Sound::ChannelType &channel)
{
	LOG_CTRACE("game") << "Add location sound" << sound << baseVolume << object << channel;
	m_sfxLocations.emplace_back(new TiledGameSfxLocation(sound, baseVolume, object, channel));

}



/**
 * @brief RpgGame::loadDefaultQuests
 */

void RpgGame::loadDefaultQuests(const int &questions)
{
	LOG_CTRACE("game") << "Load default quests";

	// Winner streak quests

	if (questions >= 5) {
		m_gameDefinition.quests.append({ RpgQuest::WinnerDefault, 3, 75 });
		m_gameDefinition.quests.append({ RpgQuest::WinnerDefault, 5, 100 });

		if (questions >= 10) {
			m_gameDefinition.quests.append({ RpgQuest::WinnerDefault, 7, 175 });

			for (int i=2;; ++i) {
				int q = 5*i;

				if (q >= questions) {
					m_gameDefinition.quests.append({ RpgQuest::WinnerDefault, questions, (i+5) * 100 });
					break;
				}

				m_gameDefinition.quests.append({ RpgQuest::WinnerDefault, i*5, i * 100 });
			}
		}
	}

	// Enemy quests

	if (m_enemyCount >= 5) {
		m_gameDefinition.quests.append({ RpgQuest::EnemyDefault, 5, 2 });

		for (int i=10; i<=m_enemyCount; i+=5) {
			m_gameDefinition.quests.append({ RpgQuest::EnemyDefault, i, i-5 });
		}
	}


	// Quests override on empty questions

	if (m_rpgQuestion->emptyQuestions()) {
		for (RpgQuest &q : m_gameDefinition.quests) {
			if (q.type == RpgQuest::SuddenDeath)
				q.currency = std::min(95, std::max(10, (int) (q.currency * 0.1)));
		}
	}

	emit questsChanged();
}




/**
 * @brief RpgGame::onGameQuestionSuccess
 * @param answer
 */

void RpgGame::onGameQuestionSuccess(const QVariantMap &answer)
{
	if (m_rpgQuestion)
		m_rpgQuestion->questionSuccess(answer);

	setWinnerStreak(m_winnerStreak+1);
	checkQuests();

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

	setWinnerStreak(0);
	checkQuests();

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
	int d = 0;
	int all = 0;

	for (const EnemyData &e : m_enemyDataList) {
		if (!e.enemy)
			continue;

		++all;

		if (e.enemy->isAlive())
			++c;
		else if (e.dieForever)
			++d;
	}

	setEnemyCount(c);

	if (c==0)
		d=all;

	setDeadEnemyCount(d);
	checkQuests();

	return c;
}


/**
 * @brief RpgGame::onMarketLoaded
 */

void RpgGame::onMarketLoaded()
{
	if (m_controlledPlayer)
		m_controlledPlayer->setIsLocked(true);
}


/**
 * @brief RpgGame::onMarketUnloaded
 */

void RpgGame::onMarketUnloaded()
{
	this->forceActiveFocus(Qt::OtherFocusReason);

	if (m_controlledPlayer)
		m_controlledPlayer->setIsLocked(false);
}



/**
 * @brief RpgGame::checkQuests
 */

void RpgGame::checkQuests()
{
	checkEnemyQuests(m_deadEnemyCount);
	checkWinnerQuests(m_winnerStreak);
}



/**
 * @brief RpgGame::checkEnemyQuests
 * @param count
 */

void RpgGame::checkEnemyQuests(const int &count)
{
	auto found = m_gameDefinition.quests.end();

	for (auto it = m_gameDefinition.quests.begin(); it != m_gameDefinition.quests.end(); ++it) {
		if (it->type != RpgQuest::EnemyDefault)
			continue;

		if (it->amount > count)
			continue;

		if (it->success > 0)
			continue;

		if (found == m_gameDefinition.quests.end())
			found = it;
		else if (it->amount > found->amount)
			found = it;

		questSuccess(&*it);
	}

	if (found == m_gameDefinition.quests.end())
		return;

	static const QColor color = QColor::fromString(QStringLiteral("#9C27B0"));

	messageColor(tr("%1 killed enemies").arg(found->amount), color);

}


/**
 * @brief RpgGame::checkWinnerQuests
 * @param count
 */

void RpgGame::checkWinnerQuests(const int &count)
{
	if (count < m_lastWinnerStreak)
		m_lastWinnerStreak = 0;

	auto found = m_gameDefinition.quests.end();

	for (auto it = m_gameDefinition.quests.begin(); it != m_gameDefinition.quests.end(); ++it) {
		if (it->type != RpgQuest::WinnerDefault)
			continue;

		if (it->amount > count)
			continue;

		if (found == m_gameDefinition.quests.end())
			found = it;
		else if (it->amount > found->amount)
			found = it;
	}

	if (found == m_gameDefinition.quests.end())
		return;

	if (found->amount == m_lastWinnerStreak)
		return;

	questSuccess(&*found);

	static const QColor color = QColor::fromString(QStringLiteral("#9C27B0"));

	messageColor(tr("Winner streak: %1").arg(found->amount), color);

	m_lastWinnerStreak = found->amount;
}




/**
 * @brief RpgGame::checkFinalQuests
 */

void RpgGame::checkFinalQuests()
{
	for (RpgQuest &q : m_gameDefinition.quests) {
		if (q.type == RpgQuest::SuddenDeath && q.success == 0)
			questSuccess(&q);
	}
}



/**
 * @brief RpgGame::questSuccess
 * @param quest
 */

void RpgGame::questSuccess(RpgQuest *quest)
{
	Q_ASSERT(quest);

	quest->success++;
	setCurrency(m_currency+quest->currency);
}



/**
 * @brief RpgGame::updateScatterEnemies
 */

void RpgGame::updateScatterEnemies()
{
	if (!m_scatterSeriesEnemies)
		return;

	QList<QPointF> list;

	for (const EnemyData &e : m_enemyDataList) {
		if (e.scene != m_currentScene || !e.enemy || !e.enemy->isAlive())
			continue;

		QPointF pos = e.enemy->bodyPosition();
		pos.setY(m_currentScene->height()-pos.y());

		list.append(pos);
	}

	m_scatterSeriesEnemies->replace(list);


}


/**
 * @brief RpgGame::updateScatterPlayers
 */

void RpgGame::updateScatterPlayers()
{
	if (!m_scatterSeriesPlayers)
		return;

	QList<QPointF> list;
	int playerIndex = -1;


	for (const RpgPlayer *p : m_players) {
		if (!p || p->scene() != m_currentScene || !p->isAlive())
			continue;

		QPointF pos = p->bodyPosition();
		pos.setY(m_currentScene->height()-pos.y());

		list.append(pos);

		if (p == m_controlledPlayer)
			playerIndex = list.size()-1;

	}

	m_scatterSeriesPlayers->replace(list);

	if (playerIndex != -1) {
		static const QColor color = QColor::fromString(QStringLiteral("#43A047"));

		m_scatterSeriesPlayers->setPointConfiguration(playerIndex, QXYSeries::PointConfiguration::Color, color);
	}

}



/**
 * @brief RpgGame::updateScatterPoints
 */

void RpgGame::updateScatterPoints()
{
	if (!m_scatterSeriesPoints)
		return;

	QList<QPointF> list;

	for (const auto &ptr : m_controlGroups) {
		if (!ptr)
			continue;

		RpgControlGroup *g = ptr.get();

		if (g->type() == RpgControlGroup::ControlGroupSave) {
			RpgControlGroupSave *s = dynamic_cast<RpgControlGroupSave*>(g);

			if (!s->isActive())
				continue;

			QPointF pos = s->position();
			pos.setY(m_currentScene->height()-pos.y());

			list.append(pos);
		}
	}

	m_scatterSeriesPoints->setMarkerShape(QScatterSeries::MarkerShapeTriangle);
	m_scatterSeriesPoints->replace(list);
}




/**
 * @brief RpgGame::getPickablesFromPropertyValue
 * @param value
 * @return
 */

QVector<RpgGameData::Pickable::PickableType> RpgGame::getPickablesFromPropertyValue(const QString &value)
{
	QVector<RpgGameData::Pickable::PickableType> pickableList;

	const QStringList &pList = value.split(',', Qt::SkipEmptyParts);
	for (const QString &s : pList) {
		const RpgGameData::Pickable::PickableType &type = RpgPickableObject::typeFromString(s.simplified());

		if (type == RpgGameData::Pickable::PickableInvalid) {
			LOG_CWARNING("scene") << "Invalid pickable type:" << s;
			continue;
		}

		pickableList.append(type);
	}

	return pickableList;
}



/**
 * @brief RpgGame::funcTimeStep
 * @return
 */

FuncTimeStep RpgGame::funcTimeStep() const
{
	return m_funcTimeStep;
}

void RpgGame::setFuncTimeStep(const FuncTimeStep &newFuncTimeStep)
{
	m_funcTimeStep = newFuncTimeStep;
}




/**
 * @brief RpgGame::funcBodyStep
 * @return
 */

FuncBodyStep RpgGame::funcBodyStep() const
{
	return m_funcBodyStep;
}

void RpgGame::setFuncBodyStep(const FuncBodyStep &newFuncBodyStep)
{
	m_funcBodyStep = newFuncBodyStep;
}


/**
 * @brief RpgGame::funcEnemyAttackPlayer
 * @return
 */

FuncEnemyAttackPlayer RpgGame::funcEnemyAttackPlayer() const
{
	return m_funcEnemyAttackPlayer;
}

void RpgGame::setFuncEnemyAttackPlayer(const FuncEnemyAttackPlayer &newFuncEnemyAttackPlayer)
{
	m_funcEnemyAttackPlayer = newFuncEnemyAttackPlayer;
}




/**
 * @brief RpgGame::enemyFind
 * @param enemy
 * @return
 */

QVector<RpgGame::EnemyData>::iterator RpgGame::enemyFind(IsometricEnemy *enemy)
{
	return std::find_if(m_enemyDataList.begin(), m_enemyDataList.end(),
						[enemy](const EnemyData &e)	{
		return e.enemy == enemy; }
	);
}




/**
 * @brief RpgGame::enemyFind
 * @param enemy
 * @return
 */

QVector<RpgGame::EnemyData>::const_iterator RpgGame::enemyFind(IsometricEnemy *enemy) const
{
	return std::find_if(m_enemyDataList.constBegin(), m_enemyDataList.constEnd(),
						[enemy](const EnemyData &e)	{
		return e.enemy == enemy; }
	);
}

FuncPlayerUseCast RpgGame::funcPlayerCastTimeout() const
{
	return m_funcPlayerCastTimeout;
}

void RpgGame::setFuncPlayerCastTimeout(const FuncPlayerUseCast &newFuncPlayerCastTimeout)
{
	m_funcPlayerCastTimeout = newFuncPlayerCastTimeout;
}

void RpgGame::onPlayerCastTimeout(RpgPlayer *player) const
{
	if (m_funcPlayerCastTimeout)
		m_funcPlayerCastTimeout(player);
}


/**
 * @brief RpgGame::funcPlayerFinishCast
 * @return
 */

FuncPlayerUseCast RpgGame::funcPlayerFinishCast() const
{
	return m_funcPlayerFinishCast;
}

void RpgGame::setFuncPlayerFinishCast(const FuncPlayerUseCast &newFuncPlayerFinishCast)
{
	m_funcPlayerFinishCast = newFuncPlayerFinishCast;
}



/**
 * @brief RpgGame::funcPlayerUseCast
 * @return
 */

FuncPlayerUseCast RpgGame::funcPlayerUseCast() const
{
	return m_funcPlayerUseCast;
}

void RpgGame::setFuncPlayerUseCast(const FuncPlayerUseCast &newFuncPlayerUseMana)
{
	m_funcPlayerUseCast = newFuncPlayerUseMana;
}


/**
 * @brief RpgGame::winnerStreak
 * @return
 */

int RpgGame::winnerStreak() const
{
	return m_winnerStreak;
}

void RpgGame::setWinnerStreak(int newWinnerStreak)
{
	if (m_winnerStreak == newWinnerStreak)
		return;
	m_winnerStreak = newWinnerStreak;
	emit winnerStreakChanged();
}


/**
 * @brief RpgGame::currency
 * @return
 */

int RpgGame::currency() const
{
	return m_currency;
}

void RpgGame::setCurrency(int newCurrency)
{
	if (m_currency == newCurrency)
		return;
	m_currency = newCurrency;
	emit currencyChanged();
}



int RpgGame::deadEnemyCount() const
{
	return m_deadEnemyCount;
}

void RpgGame::setDeadEnemyCount(int newDeadEnemyCount)
{
	if (m_deadEnemyCount == newDeadEnemyCount)
		return;
	m_deadEnemyCount = newDeadEnemyCount;
	emit deadEnemyCountChanged();
}


/**
 * @brief RpgGame::terrains
 * @return
 */

const QHash<QString, RpgGameDefinition> &RpgGame::terrains()
{
	return m_terrains;
}



/**
 * @brief RpgGame::reloadTerrains
 */

void RpgGame::reloadTerrains()
{
	LOG_CDEBUG("game") << "Reload available RPG terrains...";

	m_terrains.clear();

	QDirIterator it(QStringLiteral(":/map"), {QStringLiteral("game.json")}, QDir::Files, QDirIterator::Subdirectories);

	while (it.hasNext()) {
		const QString &f = it.next();

		QString id = f.section('/',-2,-2);

		if (const auto &ptr = readGameDefinition(id)) {
			m_terrains.insert(id, ptr.value());
		} else {
			LOG_CWARNING("game") << "Invalid RPG terrain:" << qPrintable(f);
		}
	}

	LOG_CDEBUG("game") << "...loaded " << m_terrains.size() << " terrains";
}


/**
 * @brief RpgGame::characters
 * @return
 */

const QHash<QString, RpgPlayerCharacterConfig> &RpgGame::characters()
{
	return m_characters;
}




/**
 * @brief RpgGame::reloadCharacters
 */

void RpgGame::reloadCharacters()
{
	LOG_CDEBUG("game") << "Reload available RPG characters...";

	m_characters.clear();

	QDirIterator it(QStringLiteral(":/character"), {QStringLiteral("character.json")}, QDir::Files, QDirIterator::Subdirectories);

	static const auto writeOnConfig = [](RpgPlayerCharacterConfig *cfg, const QJsonObject &obj, const QString &path) {
		Q_ASSERT(cfg);
		cfg->fromJson(obj);

		if (cfg->name.isEmpty())
			cfg->name = path;

		cfg->prefixPath = QStringLiteral(":/character/").append(path).append('/');

		if (!cfg->image.isEmpty()) {
			cfg->image.prepend(QStringLiteral("qrc")+cfg->prefixPath);
		}

		cfg->updateSfxPath(cfg->prefixPath);
	};

	while (it.hasNext()) {
		const QString &f = it.next();

		QString id = f.section('/',-2,-2);

		const auto &ptr = Utils::fileToJsonObject(f);

		if (!ptr) {
			LOG_CERROR("game") << "Invalid config json:" << f;
			continue;
		}

		RpgPlayerCharacterConfig config;

		writeOnConfig(&config, ptr.value(), id);


		if (!config.base.isEmpty()) {
			QString basePath = QStringLiteral(":/character/").append(config.base).append(QStringLiteral("/character.json"));

			const auto &basePtr = Utils::fileToJsonObject(basePath);

			if (!basePtr) {
				LOG_CERROR("game") << "Invalid config base:" << config.base << "in:" << f;
			} else {
				QString base = config.base;
				config = RpgPlayerCharacterConfig{};
				writeOnConfig(&config, basePtr.value(), base);
				writeOnConfig(&config, ptr.value(), id);
			}
		}

		m_characters.insert(id, config);
	}

	LOG_CDEBUG("game") << "...loaded " << m_characters.size() << " characters";
}


/**
 * @brief RpgGame::reloadWorld
 */

void RpgGame::reloadWorld()
{
	Server *s = Application::instance()->client()->server();

	if (!s) {
		LOG_CTRACE("game") << "Missing server";
		return;
	}

	if (!s->user() || !s->user()->wallet()) {
		LOG_CTRACE("game") << "Missing user or wallet";
		return;
	}

	s->user()->wallet()->loadWorld();
}



/**
 * @brief RpgGame::funcPlayerUseContainer
 * @return
 */

FuncPlayerUseContainer RpgGame::funcPlayerUseContainer() const
{
	return m_funcPlayerUseContainer;
}

void RpgGame::setFuncPlayerUseContainer(const FuncPlayerUseContainer &newFuncPlayerUseContainer)
{
	m_funcPlayerUseContainer = newFuncPlayerUseContainer;
}




/**
 * @brief RpgGame::scatterSeriesEnemies
 * @return
 */

QScatterSeries *RpgGame::scatterSeriesEnemies() const
{
	return m_scatterSeriesEnemies.data();
}

void RpgGame::setScatterSeriesEnemies(QScatterSeries *newScatterSeriesEnemies)
{
	if (m_scatterSeriesEnemies == newScatterSeriesEnemies)
		return;
	m_scatterSeriesEnemies = newScatterSeriesEnemies;
	emit scatterSeriesEnemiesChanged();
}



/**
 * @brief RpgGame::scatterSeriesPlayers
 * @return
 */

QScatterSeries *RpgGame::scatterSeriesPlayers() const
{
	return m_scatterSeriesPlayers.data();
}

void RpgGame::setScatterSeriesPlayers(QScatterSeries *newScatterSeriesPlayers)
{
	if (m_scatterSeriesPlayers == newScatterSeriesPlayers)
		return;
	m_scatterSeriesPlayers = newScatterSeriesPlayers;
	emit scatterSeriesPlayersChanged();
}



/**
 * @brief RpgGame::funcPlayerAttackEnemy
 * @return
 */

FuncPlayerAttackEnemy RpgGame::funcPlayerAttackEnemy() const
{
	return m_funcPlayerAttackEnemy;
}

void RpgGame::setFuncPlayerAttackEnemy(const FuncPlayerAttackEnemy &newFuncPlayerAttackEnemy)
{
	m_funcPlayerAttackEnemy = newFuncPlayerAttackEnemy;
}



/**
 * @brief RpgGame::funcPlayerPick
 * @return
 */

FuncPlayerPick RpgGame::funcPlayerPick() const
{
	return m_funcPlayerPick;
}

void RpgGame::setFuncPlayerPick(const FuncPlayerPick &newFuncPlayerPick)
{
	m_funcPlayerPick = newFuncPlayerPick;
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

const QList<RpgPlayer *> &RpgGame::players() const
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
 * @brief RpgGame::baseEntitySprite
 * @return
 */

const QVector<RpgGame::TextureSpriteMapper> &RpgGame::baseEntitySprite()
{
	static std::unique_ptr<QVector<TextureSpriteMapper>> mapper;

	if (mapper)
		return *(mapper.get());

	mapper.reset(new QVector<TextureSpriteMapper>);

	struct BaseMapper {
		QString name;
		int count = 0;
		int duration = 0;
		int loops = 0;
	};

	static const QVector<TiledObject::Direction> directions = {
		TiledObject::SouthWest, TiledObject::West, TiledObject::NorthWest, TiledObject::North, TiledObject::NorthEast,
		TiledObject::East, TiledObject::SouthEast, TiledObject::South
	};


	static const QVector<BaseMapper> baseMapper = {
		{ QStringLiteral("idle"), 4, 250, 0 },
		{ QStringLiteral("attack"), 10, 40, 1 },
		{ QStringLiteral("bow"), 9, 40, 1 },
		{ QStringLiteral("cast"), 9, 60, 1 },
		{ QStringLiteral("walk"), 11, 60, 0 },
		{ QStringLiteral("run"), 10, 60, 0 },
		{ QStringLiteral("death"), 8, 60, 1 },
	};


	for (const auto &d : directions) {
		for (const auto &m : baseMapper) {
			TextureSpriteMapper dst;
			dst.name = m.name;
			dst.direction = d;
			dst.width = 148;
			dst.height = 130;
			dst.duration = m.duration;
			dst.loops = m.loops;

			for (int i=0; i<m.count; ++i)
				mapper->append(dst);
		}
	}

	return *(mapper.get());
}





/**
 * @brief RpgGame::loadBaseTextureSprites
 * @param handler
 * @param path
 * @param layer
 * @return
 */

bool RpgGame::loadBaseTextureSprites(TiledSpriteHandler *handler, const QString &path, const QString &layer)
{
	return loadTextureSpritesWithHurt(handler, baseEntitySprite(), path, layer);
}





/**
 * @brief RpgGame::loadTextureSpritesWithHurt
 * @param handler
 * @param mapper
 * @param path
 * @param layer
 * @return
 */

bool RpgGame::loadTextureSpritesWithHurt(TiledSpriteHandler *handler, const QVector<TextureSpriteMapper> &mapper, const QString &path, const QString &layer)
{
	Q_ASSERT(handler);

	LOG_CTRACE("game") << "Load extended sprite texture" << path << layer;

	const auto &ptr = Utils::fileToJsonObject(
						  path.endsWith('/') ?
							  path+QStringLiteral("/texture.json") :
							  path+QStringLiteral(".json"));

	if (!ptr)
		return false;

	TextureSpriteDef def;
	def.fromJson(*ptr);

	auto sprites = spritesFromMapper(mapper, def);

	// Add hurt virtual sprites

	const QVector<TiledObject::Direction> directions = directionsFromMapper(mapper, QStringLiteral("death"));

	for (const auto &d : directions) {
		TextureSpriteDirection data;
		data.sprite = spriteFromMapper(mapper, def, QStringLiteral("death"), d, 4);
		data.sprite.name = QStringLiteral("hurt");
		data.direction = d;
		sprites.append(data);
	}

	return appendToSpriteHandler(handler, sprites,
								 path.endsWith('/') ?
									 path+QStringLiteral("/texture.png") :
									 path+QStringLiteral(".png"),
								 layer);
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
		case TiledWeapon::WeaponBroadsword:
		case TiledWeapon::WeaponAxe:
		case TiledWeapon::WeaponMace:
		case TiledWeapon::WeaponHammer:
		case TiledWeapon::WeaponDagger:
			return QStringLiteral("attack");

		case TiledWeapon::WeaponShortbow:
		case TiledWeapon::WeaponLongbow:
			return QStringLiteral("bow");

		case TiledWeapon::WeaponMageStaff:
			return QStringLiteral("cast");

		case TiledWeapon::WeaponShield:
		case TiledWeapon::WeaponLightningWeapon:
		case TiledWeapon::WeaponFireFogWeapon:
		case TiledWeapon::WeaponInvalid:
			break;
	}

	return {};
}


/**
 * @brief RpgGame::defaultEnemyMetric
 * @return
 */

RpgEnemyMetricDefinition RpgGame::defaultEnemyMetric()
{
	static RpgEnemyMetricDefinition *def = nullptr;

	if (!def) {
		def = new RpgEnemyMetricDefinition;

		EnemyMetric soldier;
		soldier.speed = 85.;
		soldier.returnSpeed = 125;
		soldier.pursuitSpeed = 170.;
		soldier.sensorRange = M_PI*0.5;

		EnemyMetric archer = soldier;
		archer.firstAttackTime = 500;
		archer.autoAttackTime = 1250;
		archer.sensorRange = M_PI*0.6;

		EnemyMetric werebear;
		werebear.speed = 4.;
		werebear.returnSpeed = 4.;
		werebear.pursuitSpeed = 7.;
		werebear.sleepingTime = 0;
		werebear.firstAttackTime = 750;
		werebear.autoAttackTime = 1000;
		werebear.sensorRange = M_PI*0.35;
		werebear.targetCircleRadius = 65.;

		def->soldier.insert(QStringLiteral("default"), soldier);
		def->archer.insert(QStringLiteral("default"), archer);
		def->werebear.insert(QStringLiteral("default"), werebear);
		def->skeleton.insert(QStringLiteral("default"), soldier);



		// Player cast with timer

		def->playerCast.insert(RpgPlayerCharacterConfig::CastInvalid, 0);
		def->playerCast.insert(RpgPlayerCharacterConfig::CastInvisible, 4);
		def->playerCast.insert(RpgPlayerCharacterConfig::CastFireFog, 23);
		def->playerCast.insert(RpgPlayerCharacterConfig::CastProtect, 9);

		// Player cast with shot

		def->playerCast.insert(RpgPlayerCharacterConfig::CastFireball, 145);
		def->playerCast.insert(RpgPlayerCharacterConfig::CastFireballTriple, 175/3);
		def->playerCast.insert(RpgPlayerCharacterConfig::CastLightning, 315);
		def->playerCast.insert(RpgPlayerCharacterConfig::CastArrowQuintuple, 95/5);
	}


	return *def;
}



/**
 * @brief RpgGame::onMouseClick
 * @param x
 * @param y
 */

void RpgGame::onMouseClick(const qreal &x, const qreal &y, const int &buttons, const int &modifiers)
{
	if (m_paused)
		return;

	if (!m_controlledPlayer)
		return;

	if (Qt::MouseButtons::fromInt(buttons).testFlag(Qt::RightButton)) {
		m_controlledPlayer->clearDestinationPoint();
		return;
	}

#ifndef QT_NO_DEBUG
	if (modifiers & Qt::AltModifier) {
		m_controlledPlayer->clearDestinationPoint();
		m_controlledPlayer->TiledObject::emplace(x, y);
		return;
	}
#endif

	if (!m_controlledPlayer->isAlive())
		return;

	if (mouseAttack()) {
		m_controlledPlayer->attackToPoint(x, y);
		return;
	}

	if (!mouseNavigation())
		return;

	if (modifiers & Qt::ControlModifier) {
		m_controlledPlayer->attackToPoint(x, y);
	} else {
		if (modifiers & Qt::ShiftModifier)
			m_controlledPlayer->m_pickAtDestination = true;
		else
			m_controlledPlayer->m_pickAtDestination = false;

		if (const auto &ptr = m_currentScene->findShortestPath(m_controlledPlayer, QPointF(x,y))) {
			m_controlledPlayer->setDestinationPoint(ptr.value());

			if (!m_controlledPlayer->m_sfxAccept.soundList().isEmpty())
				m_controlledPlayer->m_sfxAccept.playOne();

		} else {
			if (!m_controlledPlayer->m_sfxDecline.soundList().isEmpty())
				m_controlledPlayer->m_sfxDecline.playOne();

			m_controlledPlayer->clearDestinationPoint();
		}
	}
}


/**
 * @brief RpgGame::setQuestions
 * @param scene
 * @param factor
 * @return
 */

int RpgGame::setQuestions(TiledScene *scene, qreal factor)
{
	if (!scene)
		return -1;

	if (m_rpgQuestion && m_rpgQuestion->emptyQuestions()) {
		for (EnemyData &e : m_enemyDataList)
			e.hasQuestion = false;

		return 0;
	}

	int q = 0;
	int count = 0;
	int created = 0;

	QVector<EnemyData*> eList;

	for (EnemyData &e : m_enemyDataList) {
		if (!e.scene || e.scene != scene)
			continue;

		++count;

		if (e.hasQuestion)
			++q;
		else
			eList.append(&e);
	}

	if (!count)
		return -1;

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(eList.begin(), eList.end(), g);

	for (EnemyData *e : eList) {
		if ((qreal) (q+1) / (qreal) count > factor)
			break;

		e->hasQuestion = true;
		++q;
		++created;
	}

	return created;
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

	player->emplace(player->currentSceneStartPosition());
	player->setHp(player->maxHp());
	//player->setMp(std::max(player->config().mpStart, player->mp()));

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
		if (e.scene == scene && e.enemy && !e.dieForever)
			e.enemy->setHp(e.enemy->maxHp());
	}

	recalculateEnemies();
}



/**
 * @brief RpgGame::saveTerrainInfo
 */

void RpgGame::saveTerrainInfo()
{
	QDir dir = QDir::temp();

	static const QString subdir("callofsuli-terrain-info");

	if (dir.exists(subdir)) {
		LOG_CDEBUG("game") << "Directory already exists, try remove:" << dir.absoluteFilePath(subdir);

		if (!dir.cd(subdir)) {
			LOG_CERROR("game") << "Directory change error:" << dir.absoluteFilePath(subdir);
			return;
		}

		if (!dir.removeRecursively()) {
			LOG_CERROR("game") << "Directory remove error:" << dir.absolutePath();
			return;
		}

		dir.cdUp();
	}

	if (!dir.mkdir(subdir)) {
		LOG_CERROR("game") << "Directory create error:" << dir.absoluteFilePath(subdir);
		return;
	}

	if (!dir.cd(subdir)) {
		LOG_CERROR("game") << "Directory change error:" << dir.absoluteFilePath(subdir);
		return;
	}

	LOG_CINFO("game") << "Save terrain info to:" << dir.absolutePath();

	for (auto it = m_terrains.constBegin(); it != m_terrains.constEnd(); ++it) {
		QDir d = dir;
		d.mkdir(it.key());
		d.cd(it.key());
		static const QString filename("market.json");

		const auto &ptr = saveTerrainInfo(it.value());

		if (!ptr) {
			LOG_CERROR("game") << "Terrain save error:" << it.key();
			continue;
		}

		if (const QString f = d.absoluteFilePath(filename); Utils::jsonObjectToFile(ptr.value().toJson(), f)) {
			LOG_CDEBUG("game") << "Terrain saved:" << f;
		} else {
			LOG_CERROR("game") << "Terrain save error:" << f;
		}
	}
}



/**
 * @brief RpgGame::saveTerrainInfo
 * @param def
 * @return
 */

std::optional<RpgMarket> RpgGame::saveTerrainInfo(const RpgGameDefinition &def)
{
	RpgMarket market;

	const QString marketFile = Tiled::urlToLocalFileOrQrc(QUrl(def.basePath+QStringLiteral("/market.json")));

	if (QFile::exists(marketFile)) {
		LOG_CTRACE("game") << "Read market data from:" << marketFile;

		const auto &ptr = Utils::fileToJsonObject(marketFile);

		if (ptr) {
			market.fromJson(ptr.value());
		} else {
			LOG_CWARNING("game") << "File read error:" << marketFile;
		}
	}

	market.type = RpgMarket::Map;
	market.name.clear();


	QVector<RpgGameData::Pickable::PickableType> pickableList;

	int enemyCount = 0;
	int currencyCount = 0;
	int mpCount = 0;
	bool hasMarket = false;


	for (const TiledSceneDefinition &s : def.scenes) {
		const QString filename = Tiled::urlToLocalFileOrQrc(QUrl(def.basePath+'/'+s.file));

		LOG_CTRACE("game") << "Read scene from file:" << filename;

		QFile f(filename);

		if (!f.exists()) {
			LOG_CERROR("game") << "Read error:" << filename;
			return std::nullopt;
		}

		if (!f.open(QFile::ReadOnly | QFile::Text)) {
			LOG_CERROR("game") << "Read error:" << filename;
			return std::nullopt;
		}

		QXmlStreamReader xml;

		xml.setDevice(&f);

		if (xml.readNextStartElement() && xml.name() == QStringLiteral("map")) {
			while (xml.readNextStartElement()) {
				if (xml.name() == QStringLiteral("objectgroup") &&
						xml.attributes().value(QStringLiteral("name")).toString() == QStringLiteral("enemy")) {

					while (xml.readNextStartElement()) {
						if (xml.name() == QStringLiteral("object")) {
							const QString type = xml.attributes().value(QStringLiteral("type")).toString();

							if (RpgEnemyIface::typeFromString(type) != RpgGameData::Enemy::EnemyInvalid) {
								++enemyCount;

								while (xml.readNextStartElement()) {
									if (xml.name() == QStringLiteral("properties")) {
										while (xml.readNextStartElement()) {
											if (xml.name() == QStringLiteral("property") &&
													(xml.attributes().value(QStringLiteral("name")).toString() == QStringLiteral("pickable") ||
													 xml.attributes().value(QStringLiteral("name")).toString() == QStringLiteral("pickableOnce")
													 )) {

												pickableList.append(getPickablesFromPropertyValue(
																		xml.attributes().value(QStringLiteral("value")).toString()
																		));
											}

											xml.skipCurrentElement();
										}
									} else {
										xml.skipCurrentElement();
									}

								}
							} else {
								xml.skipCurrentElement();
							}
						} else {
							xml.skipCurrentElement();
						}

					}

				} else if (xml.name() == QStringLiteral("objectgroup") &&
						   xml.attributes().value(QStringLiteral("name")).toString() == QStringLiteral("transport")) {

					while (xml.readNextStartElement()) {
						if (xml.name() == QStringLiteral("object") &&
								xml.attributes().value(QStringLiteral("type")).toString() == QStringLiteral("market")) {
							hasMarket = true;
						}

						xml.skipCurrentElement();
					}

				} else if (xml.name() == QStringLiteral("objectgroup") &&
						   xml.attributes().value(QStringLiteral("name")).toString() == QStringLiteral("pickable")) {

					while (xml.readNextStartElement()) {
						if (xml.name() == QStringLiteral("object")) {
							const QString type = xml.attributes().value(QStringLiteral("type")).toString();
							if (const RpgGameData::Pickable::PickableType &p = RpgPickableObject::typeFromString(type.simplified());
									p != RpgGameData::Pickable::PickableInvalid) {
								pickableList.append(p);
							}
						}

						xml.skipCurrentElement();
					}
				} else {
					xml.skipCurrentElement();
				}

			}

		} else {
			LOG_CERROR("game") << "Invalid file:" << filename;
			return std::nullopt;
		}
	}


	for (const auto &p : pickableList) {
		if (p == RpgGameData::Pickable::PickableMp)
			++mpCount;
		else if (p == RpgGameData::Pickable::PickableCoin)
			++currencyCount;
	}

	QJsonObject info;
	info.insert(QStringLiteral("hasMarket"), hasMarket);
	info.insert(QStringLiteral("enemyCount"), enemyCount);
	info.insert(QStringLiteral("currencyCount"), currencyCount * RpgCoinPickable::amount(true));
	info.insert(QStringLiteral("mpCount"), mpCount * RpgMpPickable::amount());
	info.insert(QStringLiteral("duration"), def.duration);

	market.info = info;

	return market;
}





/**
 * @brief RpgGame::useWeapon
 * @param type
 */

void RpgGame::useWeapon(const TiledWeapon::WeaponType &type)
{
	if (type == TiledWeapon::WeaponInvalid) {
		LOG_CERROR("game") << "Invalid weapon" << type;
		return;
	}

	if (type == TiledWeapon::WeaponDagger) {			// Free
		return;
	}

	const QString name = RpgArmory::weaponHash().value(type);

	if (name.isEmpty()) {
		LOG_CERROR("game") << "Invalid weapon" << type;
		return;
	}

	auto it = std::find_if(m_usedWallet.begin(), m_usedWallet.end(), [&name](const RpgWallet &w) {
		return w.type == RpgMarket::Weapon && w.name == name;
	});

	if (it == m_usedWallet.end()) {
		RpgWallet w;
		w.type = RpgMarket::Weapon;
		w.name = name;
		w.amount = 1;
		m_usedWallet.append(w);
	} else {
		it->amount++;
	}
}


/**
 * @brief RpgGame::usedWalletAsArray
 * @return
 */

QJsonArray RpgGame::usedWalletAsArray() const
{
	QJsonArray list;

	for (const RpgWallet &w : m_usedWallet) {
		list.append(w.toJson());
	}

	return list;
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



const QList<RpgQuest> &RpgGame::quests() const
{
	return m_gameDefinition.quests;
}



QScatterSeries *RpgGame::scatterSeriesPoints() const
{
	return m_scatterSeriesPoints;
}

void RpgGame::setScatterSeriesPoints(QScatterSeries *newScatterSeriesPoints)
{
	if (m_scatterSeriesPoints == newScatterSeriesPoints)
		return;
	m_scatterSeriesPoints = newScatterSeriesPoints;
	emit scatterSeriesPointsChanged();
}

