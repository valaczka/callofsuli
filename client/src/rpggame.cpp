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

#include "actionrpggame.h"
#include "rpgarrow.h"
#include "rpgcontrolcollection.h"
#include "rpgcontrolcontainer.h"
#include "rpgcontrolgate.h"
#include "rpgcontrollight.h"
#include "rpgcontrolrandomizer.h"
#include "rpgcontrolteleport.h"
#include "rpgenemybase.h"
#include "rpgfireball.h"
#include "rpggame.h"
#include "rpgpickable.h"
#include "rpglightning.h"
#include "rpglongsword.h"
#include "rpgwerebear.h"
#include "rpguserwallet.h"
#include "sound.h"
#include "application.h"
#include "rpgquestion.h"
#include "utils_.h"
#include "tiledspritehandler.h"
#include "tileddebugdraw.h"
#include <libtiled/imagelayer.h>
#include <libtiled/objectgroup.h>
#include <QBuffer>

#include <libtcod/path.hpp>
#include <libtcod/fov.hpp>

#ifndef Q_OS_WASM
#include "standaloneclient.h"
#endif



/// Static hash

QHash<QString, RpgGameDefinition> RpgGame::m_terrains;
QHash<QString, RpgPlayerCharacterConfig> RpgGame::m_characters;

const QHash<QString, RpgGameData::EnemyBaseData::EnemyType> RpgEnemyIface::m_typeHash = {
	{ QStringLiteral("werebear"), RpgGameData::EnemyBaseData::EnemyWerebear },
	{ QStringLiteral("soldier"), RpgGameData::EnemyBaseData::EnemySoldier },
	{ QStringLiteral("soldierFix"), RpgGameData::EnemyBaseData::EnemySoldierFix },
	{ QStringLiteral("archer"), RpgGameData::EnemyBaseData::EnemyArcher },
	{ QStringLiteral("archerFix"), RpgGameData::EnemyBaseData::EnemyArcherFix },
	{ QStringLiteral("skeleton"), RpgGameData::EnemyBaseData::EnemySkeleton },
	{ QStringLiteral("smith"), RpgGameData::EnemyBaseData::EnemySmith },
	{ QStringLiteral("smithFix"), RpgGameData::EnemyBaseData::EnemySmithFix },
	{ QStringLiteral("barbarian"), RpgGameData::EnemyBaseData::EnemyBarbarian },
	{ QStringLiteral("barbarianFix"), RpgGameData::EnemyBaseData::EnemyBarbarianFix },
	{ QStringLiteral("butcher"), RpgGameData::EnemyBaseData::EnemyButcher },
	{ QStringLiteral("butcherFix"), RpgGameData::EnemyBaseData::EnemyButcherFix },
};





/**
 * @brief The RpgGamePrivate class
 */

class RpgGamePrivate
{
private:
	RpgGamePrivate(RpgGame *game) :
		d(game)
	{}

	RpgGame *const d;

	QPointer<ActionRpgGame> m_action;

	struct PlayerPosition {
		QPointer<TiledScene> scene;
		QPointF position;

		friend bool operator==(const PlayerPosition &p1, const PlayerPosition &p2) {
			return p1.scene == p2.scene && p1.position == p2.position;
		}
	};

	int m_loadForPlayerCount = 1;

	QVector<PlayerPosition> m_playerPositionList;



	enum EnemyWatchedEvent {
		EnemyWatchedHit,
		EnemyWatchedShot,
		EnemyWatchedControlCollection,
		EnemyWatchedControlGate
	};

	void updatePlayerWatched(RpgPlayer *player, const EnemyWatchedEvent &event);

	static QHash<QString, RpgArmory::LayerData> readLayerData(const QString &file);

	void playerUseCollection(RpgPlayer *player, RpgControlCollection *control, const bool &success);
	void playerUseGate(RpgPlayer *player, RpgControlGate *control, const bool &success);
	void playerUseTeleport(RpgPlayer *player, RpgControlTeleport *control, const bool &success);
	void playerUseExit(RpgPlayer *player);
	void playerUseContainer(RpgPlayer *player, RpgControlContainer *control, const bool &success);
	void playerUsePickable(RpgPlayer *player, RpgPickable *control, const bool &success);

	QList<RpgPickable *> extractInventory(const RpgGameData::Inventory &inventory,
										  const int &scene, const int &owner,
										  const std::function<int ()> &nextIdFn,
										  const std::function<cpVect (const int &)> &nextPosFn);


	/**
	 * @brief The CollectionPrivate class
	 */

	class CollectionPrivate : public RpgGameData::Collection
	{
	public:
		CollectionPrivate()
			: RpgGameData::Collection()
		{}

		void addImage(const int &id, const QUrl &url, const int &size = 0,
					  const QString &displayName = QString(), const QString &helperText = QString()) {
			images.append(id);
			m_imageHash.emplace(id, url, size, displayName, helperText);
		}

		RpgCollectionData get(const int &id) const {
			return m_imageHash.value(id);
		}

	private:
		QHash<int, RpgCollectionData> m_imageHash;
	};

	CollectionPrivate m_collection;

	QHash<RpgConfig::ControlType, QDeadlineTimer> m_controlMessageTimer;
	QList<QPair<RpgActiveIface*, bool> > m_controlMessages;



	// Player attack, control,...etc.

	struct PlayerEvent {
		enum Type {
			EventNone = 0,
			EventAttack,
			EventControlUse,
			EventExit
		};

		Type type = EventNone;
		QPointer<RpgWeapon> weapon;
		std::optional<QPointF> dest;
	};

	QMutex m_playerMutex;
	QHash<RpgPlayer *, std::vector<PlayerEvent> > m_playerEventHash;


	friend class RpgGame;

};



/**
 * @brief RpgGame::RpgGame
 * @param parent
 */

RpgGame::RpgGame(QQuickItem *parent)
	: TiledGame(parent)
	, q(new RpgGamePrivate(this))
{
	loadMetricDefinition();
}


/**
 * @brief RpgGame::~RpgGame
 */

RpgGame::~RpgGame()
{
	m_controls.clear();
	m_enemyDataList.clear();
	m_players.clear();
	delete q;
	q = nullptr;
}






/**
 * @brief RpgGame::load
 * @return
 */


bool RpgGame::load(const RpgGameDefinition &def, const int &playerCount)
{
	LOG_CINFO("game") << "Create game for" << playerCount << "players";

	q->m_loadForPlayerCount = playerCount;

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


	// Link controls

	for (const auto &ptr : m_controls)
		ptr->linkControls(m_controls);



	for (auto &e : m_enemyDataList) {
		Q_ASSERT(!e.motor.path.isEmpty());

		RpgEnemy *enemy = createEnemy(e.type, e.subtype, e.scene, e.objectId.id);

		if (!enemy)
			continue;

		if (!e.displayName.isEmpty()) {
			enemy->setDisplayName(e.displayName);
		}

		enemy->createMarkerItem(QStringLiteral("qrc:/TiledEnemyMarker.qml"));

		if (e.motor.path.size() > 1)
			enemy->loadPathMotor(e.motor.path);
		else
			enemy->loadFixPositionMotor(e.motor,
										enemy->nearestDirectionFromRadian(TiledObject::toRadian(e.defaultAngle)));


		enemy->m_inventory = e.inventory;

		e.enemy = enemy;
	}

	/*for (auto &e : m_pickableDataList) {

		if (e.type == RpgGameData::PickableBaseData::PickableMp)
			e.type = RpgGameData::PickableBaseData::PickableShield;

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
	}*/

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
 * @brief RpgGame::findBody
 * @param objectId
 * @return
 */

TiledObjectBody *RpgGame::findBody(const TiledObjectBody::ObjectId &objectId)
{
	TiledObjectBody *ret = nullptr;

	iterateOverBodies([&objectId, &ret](TiledObjectBody *b){
		if (b && b->objectId() == objectId) {
			ret = b;
			return;
		}
	});

	return ret;
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

		///playerFinishCast(isoPlayer);
		///
		if (isoPlayer == m_controlledPlayer) {
			for (RpgQuest &q : m_gameDefinition.quests) {
				if (q.type == RpgQuest::SuddenDeath)
					q.success = -1;
			}
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
	RpgEnemy *rpgEnemy = qobject_cast<RpgEnemy*>(enemy);

	if (!enemy)
		return;

	if (q->m_action)
		q->m_action->onEnemyDead(rpgEnemy);
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

bool RpgGame::playerTryUseControl(RpgPlayer *player, RpgActiveIface *control)
{
	if (!player || !control || control->isLocked() || !control->isActive()) {
		if (player && !player->m_sfxDecline.soundList().isEmpty())
			player->m_sfxDecline.playOne();
		messageColor(tr("Locked"), QColorConstants::Svg::red);
		return false;
	}


	if (control->activeType() == RpgConfig::ControlCollection)
		q->updatePlayerWatched(player, RpgGamePrivate::EnemyWatchedControlCollection);
	else if (control->activeType() == RpgConfig::ControlGate)
		q->updatePlayerWatched(player, RpgGamePrivate::EnemyWatchedControlGate);


	if (ActionRpgGame *a = actionRpgGame())
		return a->onPlayerUseControl(player, control);
	else
		return false;
}


/**
 * @brief RpgGame::playerUseContainer
 * @param player
 * @param container
 */

void RpgGame::playerUseControl(RpgPlayer *player, RpgActiveIface *control, const bool &success)
{
	if (!control || !player)
		return;

	if (control->questionLock() && !success) {
		RpgGameData::Player p = player->serialize(1);			// Azért kell az 1, mert különben nem fogja update-elni
		p.controlFailed(control->activeType());
		player->updateFromSnapshot(p);
	}

	if (RpgControlCollection *c = dynamic_cast<RpgControlCollection*>(control))
		q->playerUseCollection(player, c, success);
	else if (RpgControlGate *c = dynamic_cast<RpgControlGate*>(control))
		q->playerUseGate(player, c, success);
	else if (RpgControlTeleport *c = dynamic_cast<RpgControlTeleport*>(control))
		q->playerUseTeleport(player, c, success);
	else if (RpgControlContainer *c = dynamic_cast<RpgControlContainer*>(control))
		q->playerUseContainer(player, c, success);
	else if (RpgPickable *c = dynamic_cast<RpgPickable*>(control))
		q->playerUsePickable(player, c, success);
	else if (control->activeType() == RpgConfig::ControlExit)
		q->playerUseExit(player);
	else
		LOG_CERROR("game") << "Missing implementation";


}



/**
 * @brief RpgGame::createEnemy
 * @param type
 * @param subtype
 * @param game
 * @param scene
 * @return
 */

RpgEnemy *RpgGame::createEnemy(const RpgGameData::EnemyBaseData::EnemyType &type, const QString &subtype, TiledScene *scene, const int &id)
{
	RpgEnemy *enemy = nullptr;

	switch (type) {
		case RpgGameData::EnemyBaseData::EnemyWerebear: {
			enemy = createObject<RpgWerebear>(-1, scene, id,
											  this);
			break;
		}

		case RpgGameData::EnemyBaseData::EnemySoldier:
		case RpgGameData::EnemyBaseData::EnemyArcher:
		case RpgGameData::EnemyBaseData::EnemySoldierFix:
		case RpgGameData::EnemyBaseData::EnemyArcherFix:
		case RpgGameData::EnemyBaseData::EnemySmith:
		case RpgGameData::EnemyBaseData::EnemySmithFix:
		case RpgGameData::EnemyBaseData::EnemyButcher:
		case RpgGameData::EnemyBaseData::EnemyButcherFix:
		case RpgGameData::EnemyBaseData::EnemyBarbarian:
		case RpgGameData::EnemyBaseData::EnemyBarbarianFix:
		case RpgGameData::EnemyBaseData::EnemySkeleton:
		{
			if (RpgEnemyBase *e = createObject<RpgEnemyBase>(-1, scene, id,
															 type, this, 15.)) {
				e->setSubType(subtype);
				enemy = e;
			}
			break;
		}

		case RpgGameData::EnemyBaseData::EnemyInvalid:
			LOG_CERROR("game") << "Invalid enemy type" << type;
			return nullptr;
	}

	if (enemy) {
		enemy->baseData().s = scene->sceneId();
		enemy->baseData().o = -1;
		enemy->baseData().id = id;

		enemy->setMetric(getMetric(enemy->m_metric, type, subtype));


		// moved to onAlive

		/*enemy->filterSet(TiledObjectBody::FixtureEnemyBody,

						 TiledObjectBody::FixtureGround |
						 TiledObjectBody::FixturePlayerBody |
						 TiledObjectBody::FixtureTarget);*/

		if (type == RpgGameData::EnemyBaseData::EnemySoldierFix ||
				type == RpgGameData::EnemyBaseData::EnemyArcherFix ||
				type == RpgGameData::EnemyBaseData::EnemySmithFix ||
				type == RpgGameData::EnemyBaseData::EnemyButcherFix ||
				type == RpgGameData::EnemyBaseData::EnemyBarbarianFix
				) {
			enemy->m_metric.pursuitSpeed = 0;
		}

		enemy->initialize();
	}

	return enemy;
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

RpgBullet *RpgGame::createBullet(const RpgGameData::Weapon::WeaponType &type, TiledScene *scene, const int &id, const int &ownerId,
								 const bool &isDynamic)
{
	RpgBullet *bullet = nullptr;


	switch (type) {
		case RpgGameData::Weapon::WeaponShortbow:
			bullet = createObject<RpgArrow>(ownerId, scene, id, this, isDynamic ? CP_BODY_TYPE_DYNAMIC : CP_BODY_TYPE_KINEMATIC );
			break;

		case RpgGameData::Weapon::WeaponLongbow:
			bullet = createObject<RpgFireball>(ownerId, scene, id, this, isDynamic ? CP_BODY_TYPE_DYNAMIC : CP_BODY_TYPE_KINEMATIC );
			break;

		case RpgGameData::Weapon::WeaponLightningWeapon:
			bullet = createObject<RpgLightning>(ownerId, scene, id, this, isDynamic ? CP_BODY_TYPE_DYNAMIC : CP_BODY_TYPE_KINEMATIC );
			break;

		case RpgGameData::Weapon::WeaponHand:
		case RpgGameData::Weapon::WeaponDagger:
		case RpgGameData::Weapon::WeaponLongsword:
		case RpgGameData::Weapon::WeaponBroadsword:
		case RpgGameData::Weapon::WeaponAxe:
		case RpgGameData::Weapon::WeaponHammer:
		case RpgGameData::Weapon::WeaponMace:
		case RpgGameData::Weapon::WeaponGreatHand:
		case RpgGameData::Weapon::WeaponFireFogWeapon:
		case RpgGameData::Weapon::WeaponShield:
		case RpgGameData::Weapon::WeaponInvalid:
			break;

	}

	if (bullet) {
		bullet->baseData().o = ownerId;
		bullet->baseData().s = scene->sceneId();
		bullet->baseData().id = id;

		bullet->filterSet(TiledObjectBody::FixtureBulletBody,
						  TiledObjectBody::FixtureTarget |
						  TiledObjectBody::FixtureGround);
		bullet->setSensor(true);
		bullet->initialize();
		bullet->setStage(RpgGameData::LifeCycle::StageCreate);
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

RpgBullet *RpgGame::createBullet(RpgWeapon *weapon, TiledScene *scene, const int &id, const int &ownerId, const bool &isDynamic)
{
	Q_ASSERT(weapon);
	RpgBullet *b = createBullet(weapon->weaponType(), scene, id, ownerId, isDynamic);

	if (b && weapon->parentObject()) {
		const auto &o = weapon->parentObject()->objectId();
		b->setOwnerId(RpgGameData::BaseData(
						  o.ownerId,
						  o.sceneId,
						  o.id
						  ));
	}

	return b;
}



/**
 * @brief RpgGame::extractEnemyInventory
 * @param enemy
 * @return
 */

QList<RpgPickable *> RpgGame::extractEnemyInventory(RpgEnemy *enemy)
{
	if (!enemy || enemy->m_inventory.l.isEmpty())
		return {};

	if (!m_controlledPlayer) {
		LOG_CERROR("game") << "Missing controlled player";
		return {};
	}

	QList<RpgPickable*> list = q->extractInventory(enemy->m_inventory,
												   enemy->baseData().s, m_controlledPlayer->baseData().o,
												   std::bind(&RpgPlayer::nextObjectId, m_controlledPlayer),
												   std::bind(&RpgEnemy::getPickablePosition, enemy, std::placeholders::_1));

	enemy->m_inventory = RpgGameData::Inventory();

	return list;
}



/**
 * @brief RpgGame::updateRandomizer
 * @param randomizer
 */

void RpgGame::updateRandomizer(const RpgGameData::Randomizer &randomizer)
{
	QSet<int> sceneReload;

	for (const RpgGameData::RandomizerGroup &g : randomizer.groups) {
		RpgGameData::ControlBaseData base(RpgConfig::ControlRandomizer,
										  -1, g.scene, g.gid);

		if (RpgControlRandomizer *r = controlFind<RpgControlRandomizer>(base)) {
			if (r->fromRandomizerGroup(g)) {
				sceneReload.insert(g.scene);
			}

		} else {
			LOG_CERROR("game") << "Missing randomizer group" << g.scene << g.gid;
		}
	}

	for (const int &s : sceneReload)
		reloadTcodMap(findScene(s));
}



/**
 * @brief RpgGame::onShapeAboutToDeletePrivate
 * @param shape
 */

void RpgGame::onShapeAboutToDeletePrivate(cpShape *shape)
{
	for (const auto &ptr : m_controls) {
		if (RpgControlBase *b = ptr.get())
			b->onShapeAboutToDelete(shape);
	}
}



/**
 * @brief RpgGame::loadTileLayer
 * @param scene
 * @param layer
 * @param renderer
 */

void RpgGame::loadTileLayer(TiledScene *scene, Tiled::TileLayer *layer, Tiled::MapRenderer *renderer)
{
	if (layer->className().isEmpty())
		return TiledGame::loadTileLayer(scene, layer, renderer);
	else if (layer->className() == QStringLiteral("player1") && q->m_loadForPlayerCount == 1)
		return TiledGame::loadTileLayer(scene, layer, renderer);
	else if (layer->className() == QStringLiteral("player2") && q->m_loadForPlayerCount == 2)
		return TiledGame::loadTileLayer(scene, layer, renderer);
	else if (layer->className() == QStringLiteral("player3") && q->m_loadForPlayerCount == 3)
		return TiledGame::loadTileLayer(scene, layer, renderer);
	else if (layer->className() == QStringLiteral("player4") && q->m_loadForPlayerCount == 4)
		return TiledGame::loadTileLayer(scene, layer, renderer);
	else if (layer->className() == QStringLiteral("player5") && q->m_loadForPlayerCount == 5)
		return TiledGame::loadTileLayer(scene, layer, renderer);
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
	else if (groupClass == QStringLiteral("enemy2") && q->m_loadForPlayerCount > 1)
		return loadEnemy(scene, object, renderer);
	else if (groupClass == QStringLiteral("enemy3") && q->m_loadForPlayerCount > 2)
		return loadEnemy(scene, object, renderer);
	else if (groupClass == QStringLiteral("enemy4") && q->m_loadForPlayerCount > 3)
		return loadEnemy(scene, object, renderer);
	else if (groupClass == QStringLiteral("enemy5") && q->m_loadForPlayerCount > 4)
		return loadEnemy(scene, object, renderer);

	else if (groupClass == QStringLiteral("ground1") && q->m_loadForPlayerCount == 1)
		loadGround(scene, object, renderer);
	else if (groupClass == QStringLiteral("ground2") && q->m_loadForPlayerCount == 2)
		loadGround(scene, object, renderer);
	else if (groupClass == QStringLiteral("ground3") && q->m_loadForPlayerCount == 3)
		loadGround(scene, object, renderer);
	else if (groupClass == QStringLiteral("ground4") && q->m_loadForPlayerCount == 4)
		loadGround(scene, object, renderer);
	else if (groupClass == QStringLiteral("ground5") && q->m_loadForPlayerCount == 5)
		loadGround(scene, object, renderer);

	/*else if (groupClass == QStringLiteral("pickable"))
		return loadPickable(scene, object, renderer);*/
	else if (object->className() == QStringLiteral("player"))
		addPlayerPosition(scene, renderer->pixelToScreenCoords(object->position()));
	else if (object->className() == QStringLiteral("player2") && q->m_loadForPlayerCount > 1)
		addPlayerPosition(scene, renderer->pixelToScreenCoords(object->position()));
	else if (object->className() == QStringLiteral("player3") && q->m_loadForPlayerCount > 2)
		addPlayerPosition(scene, renderer->pixelToScreenCoords(object->position()));
	else if (object->className() == QStringLiteral("player4") && q->m_loadForPlayerCount > 3)
		addPlayerPosition(scene, renderer->pixelToScreenCoords(object->position()));
	else if (object->className() == QStringLiteral("player5") && q->m_loadForPlayerCount > 4)
		addPlayerPosition(scene, renderer->pixelToScreenCoords(object->position()));
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

	const QString &cname = group->className();

	if (cname == QStringLiteral("container")) {
		controlAdd<RpgControlContainer>(this, scene, group, renderer);
	} else if (cname == QStringLiteral("container2") && q->m_loadForPlayerCount > 1) {
		controlAdd<RpgControlContainer>(this, scene, group, renderer);
	} else if (cname == QStringLiteral("container3") && q->m_loadForPlayerCount > 2) {
		controlAdd<RpgControlContainer>(this, scene, group, renderer);
	} else if (cname == QStringLiteral("container4") && q->m_loadForPlayerCount > 3) {
		controlAdd<RpgControlContainer>(this, scene, group, renderer);
	} else if (cname == QStringLiteral("container5") && q->m_loadForPlayerCount > 4) {
		controlAdd<RpgControlContainer>(this, scene, group, renderer);
	} else if (cname == QStringLiteral("gate")) {
		controlAdd<RpgControlGate>(this, scene, group, renderer);
	} else if (cname == QStringLiteral("teleport")) {
		controlAdd<RpgControlTeleport>(this, scene, group, false, renderer);
	} else if (cname == QStringLiteral("hideout")) {
		controlAdd<RpgControlTeleport>(this, scene, group, true, renderer);
	} else if (cname == QStringLiteral("randomizer")) {
		if (RpgControlRandomizer *r = RpgControlRandomizer::find(m_controls, group, scene->sceneId()))
			r->addGroupLayer(scene, group, renderer);
		else
			controlAdd<RpgControlRandomizer>(this, scene, group, renderer);
	} else if (cname == QStringLiteral("collection")) {
		addCollection(scene, group, renderer);
	}
}



/**
 * @brief RpgGame::loadObjectLayer
 * @param scene
 * @param group
 * @param renderer
 * @return
 */

bool RpgGame::loadObjectLayer(TiledScene *scene, Tiled::ObjectGroup *group, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(scene);
	Q_ASSERT(group);


	if (group->className() == QStringLiteral("collection")) {
		addCollection(scene, group, renderer);

	} else if (group->className() == QStringLiteral("viewport") || group->name() == QStringLiteral("viewport")) {
		QMap<int, QPointF> viewportTopLeft;
		QMap<int, QPointF> viewportBottomRight;

		for (Tiled::MapObject *object : std::as_const(group->objects())) {
			if (object->name() == QStringLiteral("topLeft") || object->name() == QStringLiteral("topLeft1"))
				viewportTopLeft.insert(1, renderer->pixelToScreenCoords(object->position()));
			else if (object->name() == QStringLiteral("topLeft2") && q->m_loadForPlayerCount > 1)
				viewportTopLeft.insert(2, renderer->pixelToScreenCoords(object->position()));
			else if (object->name() == QStringLiteral("topLeft3") && q->m_loadForPlayerCount > 2)
				viewportTopLeft.insert(3, renderer->pixelToScreenCoords(object->position()));
			else if (object->name() == QStringLiteral("topLeft4") && q->m_loadForPlayerCount > 3)
				viewportTopLeft.insert(4, renderer->pixelToScreenCoords(object->position()));
			else if (object->name() == QStringLiteral("topLeft5") && q->m_loadForPlayerCount > 4)
				viewportTopLeft.insert(5, renderer->pixelToScreenCoords(object->position()));

			else if (object->name() == QStringLiteral("bottomRight") || object->name() == QStringLiteral("bottomRight1"))
				viewportBottomRight.insert(1, renderer->pixelToScreenCoords(object->position()));
			else if (object->name() == QStringLiteral("bottomRight2") && q->m_loadForPlayerCount > 1)
				viewportBottomRight.insert(2, renderer->pixelToScreenCoords(object->position()));
			else if (object->name() == QStringLiteral("bottomRight3") && q->m_loadForPlayerCount > 2)
				viewportBottomRight.insert(3, renderer->pixelToScreenCoords(object->position()));
			else if (object->name() == QStringLiteral("bottomRight4") && q->m_loadForPlayerCount > 3)
				viewportBottomRight.insert(4, renderer->pixelToScreenCoords(object->position()));
			else if (object->name() == QStringLiteral("bottomRight5") && q->m_loadForPlayerCount > 4)
				viewportBottomRight.insert(5, renderer->pixelToScreenCoords(object->position()));
		}

		if (!viewportTopLeft.isEmpty() && !viewportBottomRight.isEmpty()) {
			QRectF vp;
			vp.setTopLeft(viewportTopLeft.last());
			vp.setBottomRight(viewportBottomRight.last());

			LOG_CDEBUG("scene") << "Set viewport on scene" << scene->sceneId() << vp;
			scene->setViewport(vp);
		}

	} else {
		return TiledGame::loadObjectLayer(scene, group, renderer);
	}

	return true;
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
			if (m_controlledPlayer)
				m_controlledPlayer->exitHiding();
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
				m_controlledPlayer->useCurrentControl();
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
 * @brief RpgGame::loadLights
 * @param scene
 * @param objects
 * @param renderer
 * @return
 */

bool RpgGame::loadLights(TiledScene *scene, const QList<Tiled::MapObject *> &objects, Tiled::MapRenderer *renderer)
{
	Q_ASSERT (scene);

	LOG_CTRACE("scene") << "Load lights" << scene;

	bool r = true;

	for (Tiled::MapObject *o : objects) {
		if (o->property(QStringLiteral("fix")).toBool()) {
			QQuickItem *item = scene->addLight(o, renderer);
			LOG_CTRACE("scene") << "Fixed light added" << item;
			continue;
		}

		RpgGameData::ControlBaseData d;
		d.t = RpgConfig::ControlLight;
		d.o = -1;
		d.s = scene->sceneId();
		d.id = o->id();


		if (controlFind<RpgControlLight>(d)) {
			LOG_CERROR("scene") << "Control already exists" << d.t << d.o << d.s << d.id;
			r = false;
			continue;
		}

		QQuickItem *item = scene->addLight(o, renderer);
		if (!item) {
			r = false;
			continue;
		}

		RpgControlLight *light = controlAdd<RpgControlLight>(d);

		light->setVisualItem(item);

		LOG_CTRACE("scene") << "Light added" << light;
	}


	return r;
}






/**
 * @brief RpgGame::timeStepPrepareEvent
 */

void RpgGame::timeStepPrepareEvent()
{
	TiledGame::timeStepPrepareEvent();

	if (ActionRpgGame *a = actionRpgGame())
		a->onTimeStepPrepare();

}



/**
 * @brief RpgGame::timeBeforeWorldStepEvent
 * @param lag
 */

void RpgGame::timeBeforeWorldStepEvent(const qint64 &tick)
{
	TiledGame::timeBeforeWorldStepEvent(tick);

	if (ActionRpgGame *a = actionRpgGame())
		a->onTimeBeforeWorldStep(tick);
}



/**
 * @brief RpgGame::timeSteppedEvent
 */

void RpgGame::timeSteppedEvent()
{
	TiledGame::timeSteppedEvent();

	if (ActionRpgGame *a = actionRpgGame())
		a->onTimeStepped();

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
		if (e.scene != scene || e.motor.path.isEmpty())
			continue;

		if (!e.enemy || !e.enemy->isAlive())
			continue;


		if (TiledPathMotor *motor = e.enemy->destinationMotor()) {
			debugDraw->drawPolygon(motor->polygon(),
								   QColor::fromRgb(230, 150, 0),
								   3.);
		} else if (e.enemy->m_returnPathMotor) {
			debugDraw->drawPolygon(e.enemy->m_returnPathMotor->path(),
								   cpBodyGetType(e.enemy->body()) == CP_BODY_TYPE_KINEMATIC ?
									   QColor::fromRgb(0, 200, 0) :
									   QColor::fromRgb(230, 0, 200),
								   3.);
		} else if (e.motor.path.size() > 1) {
			debugDraw->drawPolygon(e.motor.path, QColor::fromRgb(230, 0, 0), 3.);
		} else
			debugDraw->drawSolidCircle(e.motor.path.first(), 3., QColor::fromRgb(230, 0, 0));
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
 * @brief RpgGame::addPlayerPosition
 * @param scene
 * @param position
 */

void RpgGame::addPlayerPosition(TiledScene *scene, const QPointF &position)
{
	if (!scene) {
		LOG_CERROR("game") << "Missing scene";
		return;
	}

	/*auto it = std::find_if(q->m_playerPositionList.constBegin(), q->m_playerPositionList.constEnd(),
						   [scene, &position](const auto &p){
		return p.scene == scene && p.position == position;
	});

	if (it != q->m_playerPositionList.constEnd()) {
		LOG_CWARNING("game") << "Player position already loaded:" << scene->sceneId() << it->position;
		return;
	}*/

	q->m_playerPositionList.append({
									   .scene = scene,
									   .position = position
								   });
}



/**
 * @brief RpgGame::addCollection
 * @param scene
 * @param groupLayer
 * @param renderer
 */

void RpgGame::addCollection(TiledScene *scene, Tiled::GroupLayer *groupLayer, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(scene);
	Q_ASSERT(groupLayer);


	// %1 -> az összegyűjtendő itemek száma

	if (const QString &str = groupLayer->propertyAsString(QStringLiteral("quest")); !str.isEmpty())
		q->m_collection.quest = str;

	for (Tiled::Layer *layer : std::as_const(*groupLayer)) {
		if (Tiled::ImageLayer *tl = layer->asImageLayer()) {
			int size = tl->hasProperty(QStringLiteral("size")) ?
						   tl->property(QStringLiteral("size")).toInt() :
						   0;

			q->m_collection.addImage(layer->id(), tl->imageSource(), size,
									 tl->propertyAsString(QStringLiteral("displayName")),
									 tl->propertyAsString(QStringLiteral("help"))
									 );

		} else if (Tiled::ObjectGroup *objgroup = layer->asObjectGroup()) {
			if (layer->className() == QStringLiteral("collection1") ||
					layer->className() == QStringLiteral("collection") ||
					layer->className().isEmpty())
				addCollection(scene, objgroup, renderer);
			else if (layer->className() == QStringLiteral("collection2") && q->m_loadForPlayerCount > 1)
				addCollection(scene, objgroup, renderer);
			else if (layer->className() == QStringLiteral("collection3") && q->m_loadForPlayerCount > 2)
				addCollection(scene, objgroup, renderer);
			else if (layer->className() == QStringLiteral("collection4") && q->m_loadForPlayerCount > 3)
				addCollection(scene, objgroup, renderer);
			else if (layer->className() == QStringLiteral("collection5") && q->m_loadForPlayerCount > 4)
				addCollection(scene, objgroup, renderer);
		}
	}
}



/**
 * @brief RpgGame::addCollection
 * @param scene
 * @param group
 * @param renderer
 */

void RpgGame::addCollection(TiledScene *scene, Tiled::ObjectGroup *group, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(scene);
	Q_ASSERT(group);

	LOG_CDEBUG("scene") << "Load collection from" << group->name() << group->id();

	RpgGameData::CollectionGroup cgr(scene->sceneId(), group->id());

	const QPointF base = group->position()+group->totalOffset();

	for (Tiled::MapObject *object : std::as_const(group->objects())) {
		if (object->shape() == Tiled::MapObject::Point) {

			// Külön kell számolni az eltolást, nem lesz jó a rendereren keresztül

			QPointF pos = renderer ? renderer->pixelToScreenCoords(object->position()) : object->position();
			pos += base;

			cgr.pos.emplace_back(pos);
		}
	}


	q->m_collection.groups.append(cgr);
}





/**
 * @brief RpgGame::playerPosition
 * @return
 */

QList<RpgGameData::PlayerPosition> RpgGame::playerPositions() const
{
	QList<RpgGameData::PlayerPosition> list;

	list.reserve(q->m_playerPositionList.size());

	for (const RpgGamePrivate::PlayerPosition &p : q->m_playerPositionList) {
		if (p.scene)
			list.emplaceBack(p.scene->sceneId(), p.position.x(), p.position.y());
		else
			LOG_CERROR("game") << "Missing scene" << p.position;
	}

	return list;
}


/**
 * @brief RpgGame::playerPositions
 * @param sceneId
 * @return
 */

QList<QPointF> RpgGame::playerPositions(const int &sceneId) const
{
	QList<QPointF> list;

	list.reserve(q->m_playerPositionList.size());

	for (const RpgGamePrivate::PlayerPosition &p : q->m_playerPositionList) {
		if (!p.scene) {
			LOG_CERROR("game") << "Missing scene" << p.position;
			continue;
		}

		if (p.scene->sceneId() == sceneId)
			list << QPointF(p.position.x(), p.position.y());
	}

	return list;
}


/**
 * @brief RpgGame::collection
 * @return
 */

const RpgGameData::Collection &RpgGame::collection() const
{
	return q->m_collection;
}


/**
 * @brief RpgGame::collection
 * @return
 */

RpgGameData::Collection &RpgGame::collection()
{
	return q->m_collection;
}


/**
 * @brief RpgGame::randomizer
 * @return
 */

RpgGameData::Randomizer RpgGame::randomizer() const
{
	RpgGameData::Randomizer randomizer;

	for (const auto &ptr : m_controls) {
		if (RpgControlRandomizer *r = dynamic_cast<RpgControlRandomizer*>(ptr.get())) {
			randomizer.groups.append(r->toRandomizerGroup());
		}
	}

	return randomizer;
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

EnemyMetric RpgGame::getMetric(EnemyMetric baseMetric, const RpgGameData::EnemyBaseData::EnemyType &type, const QString &subtype)
{
	if (m_metricDefinition.isEmpty())
		return {};

	for (int i=0; i<std::max(1, m_level) && i<m_metricDefinition.size(); ++i) {
		const RpgEnemyMetricDefinition &def = m_metricDefinition.at(i);
		QHash<QString, EnemyMetric> ptr;

		switch (type) {
			case RpgGameData::EnemyBaseData::EnemyWerebear:
				ptr = def.werebear;
				break;

			case RpgGameData::EnemyBaseData::EnemySoldier:
			case RpgGameData::EnemyBaseData::EnemySoldierFix:
				ptr = def.soldier;
				break;

			case RpgGameData::EnemyBaseData::EnemyArcher:
			case RpgGameData::EnemyBaseData::EnemyArcherFix:
				ptr = def.archer;
				break;

			case RpgGameData::EnemyBaseData::EnemySmith:
			case RpgGameData::EnemyBaseData::EnemySmithFix:
				ptr = def.smith;
				break;

			case RpgGameData::EnemyBaseData::EnemyButcher:
			case RpgGameData::EnemyBaseData::EnemyButcherFix:
				ptr = def.butcher;
				break;

			case RpgGameData::EnemyBaseData::EnemyBarbarian:
			case RpgGameData::EnemyBaseData::EnemyBarbarianFix:
				ptr = def.barbarian;
				break;

			case RpgGameData::EnemyBaseData::EnemySkeleton:
				ptr = def.skeleton;
				break;

			case RpgGameData::EnemyBaseData::EnemyInvalid:
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
 * @brief RpgGame::actionRpgGame
 * @return
 */

ActionRpgGame *RpgGame::actionRpgGame() const
{
	Q_ASSERT(q);

	return q->m_action;
}



/**
 * @brief RpgGame::setActionRpgGame
 * @param game
 */

void RpgGame::setActionRpgGame(ActionRpgGame *game)
{
	Q_ASSERT(q);

	q->m_action = game;
}



/**
 * @brief RpgGame::playerAttack
 * @param player
 * @param weapon
 * @param dest
 */

void RpgGame::playerAttack(RpgPlayer *player, RpgWeapon *weapon, const std::optional<QPointF> &dest)
{
	if (!player)
		return;

	QMutexLocker locker(&q->m_playerMutex);

	q->m_playerEventHash[player].emplace_back(RpgGamePrivate::PlayerEvent::EventAttack, weapon, dest);
}



/**
 * @brief RpgGame::playerUseCurrentControl
 * @param player
 */

void RpgGame::playerUseCurrentControl(RpgPlayer *player)
{
	QMutexLocker locker(&q->m_playerMutex);

	q->m_playerEventHash[player].emplace_back(RpgGamePrivate::PlayerEvent::EventControlUse, nullptr, std::nullopt);
}


/**
 * @brief RpgGame::playerExitHiding
 * @param player
 */

void RpgGame::playerExitHiding(RpgPlayer *player)
{
	QMutexLocker locker(&q->m_playerMutex);

	q->m_playerEventHash[player].emplace_back(RpgGamePrivate::PlayerEvent::EventExit, nullptr, std::nullopt);
}


/**
 * @brief RpgGame::playerShot
 * @param player
 * @param weapon
 * @param angle
 * @return
 */

bool RpgGame::playerShot(RpgPlayer *player, RpgWeapon *weapon, const qreal &angle)
{
	Q_ASSERT(q);

	q->updatePlayerWatched(player, RpgGamePrivate::EnemyWatchedShot);

	if (q->m_action)
		return q->m_action->onPlayerShot(player, weapon, angle);
	else
		return false;
}



/**
 * @brief RpgGame::playerHit
 * @param player
 * @param enemy
 * @param weapon
 * @return
 */

bool RpgGame::playerHit(RpgPlayer *player, RpgEnemy *enemy, RpgWeapon *weapon)
{
	Q_ASSERT(q);

	if (enemy)
		q->updatePlayerWatched(player, RpgGamePrivate::EnemyWatchedHit);

	if (q->m_action)
		return q->m_action->onPlayerHit(player, enemy, weapon);
	else
		return false;
}


/**
 * @brief RpgGame::playerAttackEnemy
 * @param player
 * @param enemy
 * @param weaponType
 * @return
 */

bool RpgGame::playerAttackEnemy(RpgPlayer *player, RpgEnemy *enemy, const RpgGameData::Weapon::WeaponType &weaponType, const int &weaponSubtype)
{
	Q_ASSERT(q);

	if (q->m_action)
		return q->m_action->onPlayerAttackEnemy(player, enemy, weaponType, weaponSubtype);
	else
		return false;
}


/**
 * @brief RpgGame::enemyHit
 * @param enemy
 * @param player
 * @param weapon
 * @return
 */

bool RpgGame::enemyHit(RpgEnemy *enemy, RpgPlayer *player, RpgWeapon *weapon)
{
	Q_ASSERT(q);

	if (q->m_action)
		return q->m_action->onEnemyHit(enemy, player, weapon);
	else
		return false;
}



/**
 * @brief RpgGame::enemyShot
 * @param enemy
 * @param weapon
 * @param angle
 * @return
 */

bool RpgGame::enemyShot(RpgEnemy *enemy, RpgWeapon *weapon, const qreal &angle)
{
	Q_ASSERT(q);

	if (q->m_action)
		return q->m_action->onEnemyShot(enemy, weapon, angle);
	else
		return false;
}


/**
 * @brief RpgGame::enemyAttackPlayer
 * @param enemy
 * @param player
 * @param weaponType
 * @return
 */

bool RpgGame::enemyAttackPlayer(RpgEnemy *enemy, RpgPlayer *player,
								const RpgGameData::Weapon::WeaponType &weaponType, const int &weaponSubtype)
{
	Q_ASSERT(q);
	if (q->m_action)
		return q->m_action->onEnemyAttackPlayer(enemy, player, weaponType, weaponSubtype);
	else
		return false;
}


/**
 * @brief RpgGame::bulletImpact
 * @param bullet
 * @param other
 * @return
 */

bool RpgGame::bulletImpact(RpgBullet *bullet, TiledObjectBody *other)
{
	Q_ASSERT(q);
	if (q->m_action)
		return q->m_action->onBulletImpact(bullet, other);
	else
		return false;
}



/**
 * @brief RpgGame::createPlayer
 * @param scene
 * @param config
 * @return
 */

RpgPlayer *RpgGame::createPlayer(TiledScene *scene, const RpgPlayerCharacterConfig &config, const int &ownerId, const bool &isDynamic)
{
	RpgPlayer *player = createObject<RpgPlayer>(ownerId, scene, 1,
												this, 15., isDynamic ? CP_BODY_TYPE_DYNAMIC : CP_BODY_TYPE_KINEMATIC );

	if (player) {
		player->setConfig(config);
		player->initialize();

		if (config.walk > 0)
			player->m_speedLength = config.walk;

		if (config.run > 0)
			player->m_speedRunLength = config.run;

		if (config.inability > 0)
			player->m_inabilityTime = config.inability;
	}

	return player;
}



/**
 * @brief RpgGame::worldStep
 * @param body
 */

void RpgGame::worldStep(TiledObjectBody *body)
{
	if (RpgPlayer *player = dynamic_cast<RpgPlayer*>(body)) {
		QMutexLocker locker(&q->m_playerMutex);

		if (const auto it = q->m_playerEventHash.find(player); it != q->m_playerEventHash.cend()) {
			for (const RpgGamePrivate::PlayerEvent &e : it.value()) {
				switch (e.type) {
					case RpgGamePrivate::PlayerEvent::EventAttack:
						player->attackReal(e.weapon, e.dest);
						break;

					case RpgGamePrivate::PlayerEvent::EventControlUse:
						player->useCurrentControlReal();
						break;

					case RpgGamePrivate::PlayerEvent::EventExit:
						player->exitHidingReal();
						break;

					case RpgGamePrivate::PlayerEvent::EventNone:
						break;
				}
			}

			q->m_playerEventHash.erase(it);
		}
	}

	if (ActionRpgGame *a = actionRpgGame(); a && a->onBodyStep(body))
		return;
	else
		TiledGame::worldStep(body);
}


/**
 * @brief RpgGame::worldStep
 */

void RpgGame::worldStep()
{
	if (ActionRpgGame *a = actionRpgGame())
		a->onWorldStep();
	else
		TiledGame::worldStep();
}



/**
 * @brief RpgGame::timeAfterWorldStepEvent
 * @param tick
 */

void RpgGame::timeAfterWorldStepEvent(const qint64 &tick)
{
	TiledGame::timeAfterWorldStepEvent(tick);

	if (ActionRpgGame *a = actionRpgGame())
		a->onTimeAfterWorldStep(tick);
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

	const RpgGameData::EnemyBaseData::EnemyType &type = RpgEnemyIface::typeFromString(object->className());

	if (type == RpgGameData::EnemyBaseData::EnemyInvalid) {
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


	const RpgGameData::Inventory inventory = getInventoryFromPropertyValue(object->propertyAsString(QStringLiteral("inventory")));
	const int &defaultAngle = object->property(QStringLiteral("direction")).toInt();

	EnemyMotorData motor;

	motor.path = p;

	if (object->hasProperty(QStringLiteral("rotateFrom")) ||
			object->hasProperty(QStringLiteral("range"))) {

		motor.rotation = true;

		if (object->hasProperty(QStringLiteral("range"))) {
			const int range = object->property(QStringLiteral("range")).toInt() * 0.5;
			motor.from = defaultAngle - range;
			motor.to = defaultAngle + range;
			motor.direction = TiledRotationMotor::DirectionCCW;
		} else {
			motor.from = object->property(QStringLiteral("rotateFrom")).toFloat();

			if (object->hasProperty(QStringLiteral("rotateTo")))
				motor.to = object->property(QStringLiteral("rotateTo")).toFloat();
			else
				motor.to = motor.from;

			if (object->hasProperty(QStringLiteral("cw")))
				motor.direction = object->property(QStringLiteral("cw")).toBool() ? TiledRotationMotor::DirectionCW : TiledRotationMotor::DirectionCCW;

			if (object->hasProperty(QStringLiteral("ccw")))
				motor.direction = object->property(QStringLiteral("ccw")).toBool() ? TiledRotationMotor::DirectionCCW : TiledRotationMotor::DirectionCW;
		}

		if (object->hasProperty(QStringLiteral("steps")))
			motor.steps = object->property(QStringLiteral("steps")).toInt();

		if (object->hasProperty(QStringLiteral("msec")))
			motor.wait = object->property(QStringLiteral("msec")).toInt();
	}



	m_enemyDataList.append(EnemyData{
							   TiledObject::ObjectId{.sceneId = scene->sceneId(), .id = object->id()},
							   type,
							   object->name(),
							   motor,
							   defaultAngle,
							   scene,
							   nullptr,
							   object->property(QStringLiteral("dieForever")).toBool(),
							   inventory,
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
 * @brief RpgGame::onGameQuestionSuccess
 * @param answer
 */

void RpgGame::onGameQuestionSuccess(const QVariantMap &answer)
{
	setWinnerStreak(m_winnerStreak+1);

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

	setWinnerStreak(0);

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

		QPointF pos = e.enemy->bodyPositionF();
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
		if (!p || p->scene() != m_currentScene || !p->isAlive() ||
				p->isHiding() || (p->config().cast == RpgPlayerCharacterConfig::CastInvisible && p->castTimerActive()))
			continue;

		QPointF pos = p->bodyPositionF();
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

	for (const auto &ptr : m_controls) {
		if (!ptr)
			continue;

		if (ptr->type() == RpgConfig::ControlCollection) {
			RpgControlCollection *c = dynamic_cast<RpgControlCollection*>(ptr.get());

			if (!c->isActive())
				continue;

			for (const RpgActiveControlObject *obj : c->controlObjectList()) {
				if (!obj)
					continue;

				QPointF pos = obj->bodyPositionF();
				pos.setY(m_currentScene->height()-pos.y());
				list.append(pos);
			}
		}
	}

	m_scatterSeriesPoints->replace(list);
	m_scatterSeriesPoints->setColor(QColorConstants::Svg::royalblue);
	m_scatterSeriesPoints->setMarkerShape(QScatterSeries::MarkerShapeStar);
	m_scatterSeriesPoints->setMarkerSize(15);

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
 * @brief RpgGame::loadTextureSprites
 * @param handler
 * @param path
 * @return
 */

QRect RpgGame::loadTextureSprites(TiledSpriteHandler *handler, const QString &path,
								  QHash<QString, RpgArmory::LayerData> *layerPtr)
{
	static const QVector<TiledObject::Direction> directions = {
		TiledObject::SouthWest,
		TiledObject::South,
		TiledObject::SouthEast,
		TiledObject::East,
		TiledObject::NorthEast,
		TiledObject::North,
		TiledObject::NorthWest,
		TiledObject::West,
	};



	QByteArray input = Utils::fileContentRead(path+QStringLiteral("input.txt"));

	if (input.isEmpty())
		return QRect();

	QTextStream buffer(&input, QIODevice::ReadOnly);
	int n = 0;

	QRect measure;

	/// input.txt format
	///
	/// line 1: <width> \t <height> \t [<bodyOffsetX>] \t [<bodyOffsetY>]
	/// line ...: <sprite> \t <frames> \t <duration> \t [<loops>] \ [<baked>]

	QVector<RpgGame::TextureSpriteMapper> mapper;

	QString line;

	while (buffer.readLineInto(&line)) {
		const QStringList field = line.split('\t');

		if (n == 0) {
			if (field.size() > 3)
				measure.setY(field.at(3).toInt());

			if (field.size() > 2)
				measure.setX(field.at(2).toInt());

			if (field.size() > 1)
				measure.setHeight(field.at(1).toInt());

			if (field.size() > 0)
				measure.setWidth(field.at(0).toInt());
		} else {
			if (field.size() < 3) {
				LOG_CERROR("game") << "Invalid line" << line;
				continue;
			}

			const QString sprite = field.at(0);
			const int frames = field.at(1).toInt();
			const int duration = field.at(2).toInt();

			const int loops = field.size() > 3 ? field.at(3).toInt() : 0;
			const bool baked = field.size() > 4 ? field.at(4).toInt() : false;

			for (const auto &d : directions) {
				TextureSpriteMapper dst;
				dst.name = sprite;
				dst.direction = d;
				dst.width = measure.width();
				dst.height = measure.height();
				dst.duration = duration;
				dst.loops = loops;
				dst.baked = baked;

				for (int i=0; i<frames; ++i)
					mapper.append(dst);
			}
		}

		++n;
	}


	QHash<QString, RpgArmory::LayerData> layerData = RpgGamePrivate::readLayerData(path+QStringLiteral("layers.txt"));

	for (const auto &[layer, data] : layerData.asKeyValueRange()) {
		QString basePath = path;
		if (layer == QStringLiteral("default"))
			basePath += QStringLiteral("texture");
		else
			basePath += layer + QStringLiteral("-texture");

		if (data.baked)
			LOG_CDEBUG("scene") << "Load texture from" << qPrintable(basePath) << "to baked layer" << qPrintable(layer);
		else
			LOG_CDEBUG("scene") << "Load texture from" << qPrintable(basePath) << "to layer" << qPrintable(layer);

		const auto &ptr = Utils::fileToJsonObject(basePath+QStringLiteral(".json"));

		if (!ptr) {
			LOG_CERROR("scene") << "Missing" << qPrintable(basePath) << "JSON";
			continue;
		}

		TextureSpriteDef def;
		def.fromJson(*ptr);

		QVector<RpgGame::TextureSpriteMapper> filteredMapper;

		filteredMapper.reserve(mapper.size());

		const QString bakedName = layer+QStringLiteral("-");

		for (RpgGame::TextureSpriteMapper m : mapper) {
			if (data.baked) {
				if (m.baked && m.name.startsWith(bakedName)) {
					m.name.remove(0, bakedName.size());
					filteredMapper.append(m);
				}
			} else {
				if (!m.baked)
					filteredMapper.append(m);
			}
		}

		const QVector<TiledGame::TextureSpriteDirection> &sprites = spritesFromMapper(filteredMapper, def);

		if (!appendToSpriteHandler(handler, sprites, basePath+QStringLiteral(".png"), layer))
			return QRect();
	}

	if (layerPtr)
		layerPtr->swap(layerData);

	return measure;
}






/**
 * @brief RpgGame::getInventoryFromPropertyValue
 * @param value
 * @return
 */

RpgGameData::Inventory RpgGame::getInventoryFromPropertyValue(const QString &value)
{
	RpgGameData::Inventory inventory;

	const QStringList &pList = value.split(',', Qt::SkipEmptyParts);
	for (const QString &s : pList) {
		const QStringList &field = s.split(':', Qt::SkipEmptyParts);

		const RpgGameData::PickableBaseData::PickableType &type = RpgPickable::typeHash().key(field.at(0).simplified(),
																							  RpgGameData::PickableBaseData::PickableInvalid);

		if (type == RpgGameData::PickableBaseData::PickableInvalid) {
			LOG_CWARNING("scene") << "Invalid pickable type:" << s;
			continue;
		}

		if (field.size() > 1)
			inventory.add(type, 1, field.at(1));
		else
			inventory.add(type);
	}

	return inventory;
}





/**
 * @brief RpgGame::getAttackSprite
 * @param weaponType
 * @return
 */

QString RpgGame::getAttackSprite(const RpgGameData::Weapon::WeaponType &weaponType)
{
	switch (weaponType) {
		case RpgGameData::Weapon::WeaponHand:
		case RpgGameData::Weapon::WeaponGreatHand:
		case RpgGameData::Weapon::WeaponLongsword:
		case RpgGameData::Weapon::WeaponBroadsword:
		case RpgGameData::Weapon::WeaponAxe:
		case RpgGameData::Weapon::WeaponMace:
		case RpgGameData::Weapon::WeaponHammer:
		case RpgGameData::Weapon::WeaponDagger:
			return QStringLiteral("attack");

		case RpgGameData::Weapon::WeaponShortbow:
		case RpgGameData::Weapon::WeaponLongbow:
			return QStringLiteral("bow");

			/*case RpgGameData::Weapon::WeaponMageStaff:
			return QStringLiteral("cast");*/

		case RpgGameData::Weapon::WeaponShield:
		case RpgGameData::Weapon::WeaponLightningWeapon:
		case RpgGameData::Weapon::WeaponFireFogWeapon:
		case RpgGameData::Weapon::WeaponInvalid:
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

		if (const auto &ptr = findShortestPath(m_controlledPlayer, cpv(x,y))) {
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

	// DISABLED

	///QTimer::singleShot(2000, this, [s = QPointer<TiledScene>(scene), this](){ this->resurrectEnemies(s); });
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


	QVector<RpgGameData::PickableBaseData::PickableType> pickableList;

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

							if (RpgEnemyIface::typeFromString(type) != RpgGameData::EnemyBaseData::EnemyInvalid) {
								++enemyCount;

								while (xml.readNextStartElement()) {
									if (xml.name() == QStringLiteral("properties")) {
										while (xml.readNextStartElement()) {
											if (xml.name() == QStringLiteral("property") &&
													(xml.attributes().value(QStringLiteral("name")).toString() == QStringLiteral("pickable") ||
													 xml.attributes().value(QStringLiteral("name")).toString() == QStringLiteral("pickableOnce")
													 )) {

												/*pickableList.append(getPickablesFromPropertyValue(
																		xml.attributes().value(QStringLiteral("value")).toString()
																		));*/
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

				} /*else if (xml.name() == QStringLiteral("objectgroup") &&
						   xml.attributes().value(QStringLiteral("name")).toString() == QStringLiteral("transport")) {

					while (xml.readNextStartElement()) {
						if (xml.name() == QStringLiteral("object") &&
								xml.attributes().value(QStringLiteral("type")).toString() == QStringLiteral("market")) {
							hasMarket = true;
						}

						xml.skipCurrentElement();
					}

				} */ else if (xml.name() == QStringLiteral("objectgroup") &&
							  xml.attributes().value(QStringLiteral("name")).toString() == QStringLiteral("pickable")) {

					/*while (xml.readNextStartElement()) {
						if (xml.name() == QStringLiteral("object")) {
							const QString type = xml.attributes().value(QStringLiteral("type")).toString();
							if (const RpgGameData::PickableBaseData::PickableType &p = RpgPickableObject::typeFromString(type.simplified());
									p != RpgGameData::PickableBaseData::PickableInvalid) {
								pickableList.append(p);
							}
						}

						xml.skipCurrentElement();
					}*/
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
		if (p == RpgGameData::PickableBaseData::PickableMp)
			++mpCount;
		else if (p == RpgGameData::PickableBaseData::PickableCoin)
			++currencyCount;
	}

	QJsonObject info;
	info.insert(QStringLiteral("hasMarket"), hasMarket);
	info.insert(QStringLiteral("enemyCount"), enemyCount);
	//info.insert(QStringLiteral("currencyCount"), currencyCount * RpgCoinPickable::amount(true));
	//info.insert(QStringLiteral("mpCount"), mpCount * RpgMpPickable::amount());
	info.insert(QStringLiteral("duration"), def.duration);

	market.info = info;

	return market;
}





/**
 * @brief RpgGame::useWeapon
 * @param type
 */

void RpgGame::useWeapon(const RpgGameData::Weapon::WeaponType &type)
{
	// DEPRECATED

	if (type == RpgGameData::Weapon::WeaponInvalid) {
		LOG_CERROR("game") << "Invalid weapon" << type;
		return;
	}

	/*if (type == RpgGameData::Weapon::WeaponDagger) {			// Free
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
	}*/
}


/**
 * @brief RpgGame::usedWalletAsArray
 * @return
 */

QJsonArray RpgGame::usedWalletAsArray() const
{
	// DEPRECATED

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

	if (m_controlledPlayer) {
		m_controlledPlayer->onJoystickStateChanged({});
		m_controlledPlayer->removeVirtualCircle();
	}

	m_controlledPlayer = newControlledPlayer;

	if (m_controlledPlayer)
		m_controlledPlayer->setVirtualCircle();

	emit controlledPlayerChanged();
}



const QList<RpgQuest> &RpgGame::quests() const
{
	return m_gameDefinition.quests;
}



/**
 * @brief RpgGame::controlRemove
 * @param control
 */

void RpgGame::controlRemove(RpgControlBase *control)
{
	if (!control)
		return;

	std::erase_if(m_controls, [control](const auto &ptr) {
		return ptr.get() == control;
	});
}


/**
 * @brief RpgGame::controlRemove
 * @param controls
 */

void RpgGame::controlRemove(const QList<RpgControlBase*> &controls)
{
	std::erase_if(m_controls, [&controls](const auto &ptr) {
		return controls.contains(ptr.get());
	});
}


/**
 * @brief RpgGame::controlAppeared
 * @param iface
 */

void RpgGame::controlAppeared(RpgActiveIface *iface)
{
	if (!iface)
		return;

	const RpgConfig::ControlType &type = iface->activeType();

	if (!iface->isActive())
		return;

	const QPair<RpgActiveIface*, bool> data(iface, iface->isLocked());

	if (q->m_controlMessages.contains(data))
		return;


	if (auto it = q->m_controlMessageTimer.find(type); it != q->m_controlMessageTimer.end()
			&& !it->isForever() && !it->hasExpired())
		return;

	if (const QString &txt = iface->helperText(iface->isLocked()); !txt.isEmpty()) {
		if (iface->activeType() == RpgConfig::ControlCollection) {
			if (m_controlledPlayer && m_controlledPlayer->collectionRq() > m_controlledPlayer->collection())
				message(txt, true);
		} else {
			message(txt, true);
		}

		q->m_controlMessageTimer[type].setRemainingTime(5000);
		q->m_controlMessages.append(data);
	}
}



/**
 * @brief RpgGame::getCollectionImageUrl
 * @param id
 * @return
 */

RpgCollectionData RpgGame::getCollectionImageData(const int &id) const
{
	return q->m_collection.get(id);
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




/**
 * @brief RpgGamePrivate::updatePlayerWatched
 * @param player
 * @param event
 */

void RpgGamePrivate::updatePlayerWatched(RpgPlayer *player, const EnemyWatchedEvent &event)
{
	if (!player)
		return;

	if (player->config().features == RpgGameData::Player::FeatureInvalid)
		return;

	for (const RpgGame::EnemyData &ed : d->m_enemyDataList) {
		RpgEnemy *enemy = qobject_cast<RpgEnemy*>(ed.enemy);

		if (!enemy || !enemy->isAlive() || enemy->isSleeping())
			continue;

		// TODO: check enemy features

		if (event == EnemyWatchedControlGate)
			continue;

		if (enemy->isWatchingPlayer(player)) {
			LOG_CTRACE("game") << "Enemy" << ed.objectId.sceneId << ed.objectId.id << "watched player" << player->baseData().o << "by event" << event;

			RpgPlayerCharacterConfig config = player->config();
			config.features.setFlag(RpgGameData::Player::FeatureCamouflage, false);
			config.features.setFlag(RpgGameData::Player::FeatureFreeWalk, false);
			config.features.setFlag(RpgGameData::Player::FeatureFreeWalkNoWeapon, false);
			config.features.setFlag(RpgGameData::Player::FeatureLockEnemy, false);
			player->setConfig(config);

			d->messageColor(QObject::tr("You have been recognised"), QColorConstants::Svg::red);

			return;
		}
	}
}





/**
 * @brief RpgGamePrivate::readLayerData
 * @param file
 * @return
 */

QHash<QString, RpgArmory::LayerData> RpgGamePrivate::readLayerData(const QString &file)
{
	QHash<QString, RpgArmory::LayerData> hash;

	hash.insert(QStringLiteral("default"), RpgArmory::LayerData(RpgGameData::Weapon::WeaponInvalid, 0, RpgArmory::ShieldNeutral));

	/// layer.txt format
	///
	/// <sprite-prefix> \t [<weapon-str>] \t [<shield-layer-str>] \t [<baked>]

	QByteArray layerData = Utils::fileContentRead(file);
	QTextStream layerBuffer(&layerData, QIODevice::ReadOnly);

	QString line;

	while (layerBuffer.readLineInto(&line)) {
		const QStringList field = line.split('\t');

		if (field.isEmpty())
			continue;


		RpgArmory::LayerData data;

		if (field.size() > 4)
			data.baked = field.at(4).toInt();

		if (field.size() > 3)
			data.shield = QVariant::fromValue(field.at(3)).value<RpgArmory::ShieldLayer>();

		if (field.size() > 2)
			data.subType = field.at(2).toInt();

		if (field.size() > 1)
			data.weapon = RpgArmory::weaponHash().key(field.at(1), RpgGameData::Weapon::WeaponInvalid);

		hash.insert(field.at(0), data);
	}

	return hash;
}




/**
 * @brief RpgGamePrivate::playerUseCollection
 */

void RpgGamePrivate::playerUseCollection(RpgPlayer *player, RpgControlCollection *control, const bool &success)
{
	Q_ASSERT(control);
	Q_ASSERT(player);

	if (success) {
		control->setIsActive(false);
		control->visualItem()->setVisible(false);
		player->setCollection(player->collection()+1);

		const int left = player->collectionRq() - player->collection();

		if (left > 1)
			d->message(QObject::tr("Collect %1 more items").arg(left), true);
		else if (left > 0)
			d->message(QObject::tr("Collect 1 more item"), true);
		else if (left == 0) {
			d->messageColor(QObject::tr("All required items collected"), QColorConstants::Svg::limegreen, true);

			bool hasTeleport = false;

			for (const auto &ptr : d->m_controls) {
				RpgControlTeleport *c = dynamic_cast<RpgControlTeleport*>(ptr.get());

				if (!c || c->baseData().hd)
					continue;

				hasTeleport = true;

				c->setIsActive(true);
				c->setCurrentState(RpgControlTeleportState::Active);
			}

			if (hasTeleport)
				d->message(QObject::tr("Escape through the teleporter"), true);
		}

	} else {
		const auto &it = m_collection.find(control->baseData().gid);

		if (it == m_collection.groups.cend() || it->pos.empty()) {
			LOG_CERROR("game") << "Missing places for group" << control->baseData().gid << "in" << control->baseData();
			return;
		}

		QList<int> freeIndices;
		freeIndices.reserve(it->pos.size());

		for (int i=0; i<it->pos.size(); ++i) {
			if (!it->pos.at(i).done)
				freeIndices.append(i);
		}


		for (const auto &ptr : d->m_controls) {
			RpgControlCollection *c = dynamic_cast<RpgControlCollection*>(ptr.get());

			if (!c || c->baseData().gid != control->baseData().gid)
				continue;

			freeIndices.removeAll(c->idx());
		}

		if (freeIndices.isEmpty())
			return;

		const int idx = freeIndices.at(QRandomGenerator::global()->bounded(freeIndices.size()));

		const auto &p = it->pos.at(idx);

		control->setIsActive(false);
		control->moveTo(d->tickTimer()->tickAddMsec(3000), cpv(p.x, p.y));
		control->setIdx(idx);
	}

}



/**
 * @brief RpgGamePrivate::playerUseGate
 * @param player
 * @param control
 * @param success
 */

void RpgGamePrivate::playerUseGate(RpgPlayer *player, RpgControlGate *control, const bool &success)
{
	Q_ASSERT(control);
	Q_ASSERT(player);

	if (!success)
		return;

	if (control->currentState() == RpgGameData::ControlGate::GateDamaged) {
		if (!player->m_sfxDecline.soundList().isEmpty())
			player->m_sfxDecline.playOne();
		return;
	}

	if (control->currentState() == RpgGameData::ControlGate::GateOpen)
		control->setCurrentState(RpgGameData::ControlGate::GateClose);
	else
		control->setCurrentState(RpgGameData::ControlGate::GateOpen);

	if (!player->m_sfxAccept.soundList().isEmpty())
		player->m_sfxAccept.playOne();
}





/**
 * @brief RpgGamePrivate::playerUseTeleport
 */

void RpgGamePrivate::playerUseTeleport(RpgPlayer *player, RpgControlTeleport *control, const bool &success)
{
	Q_ASSERT(control);
	Q_ASSERT(player);

	if (!success)
		return;

	RpgGameData::Player pData = player->serialize(1);
	RpgGameData::ControlTeleportBaseData data = control->baseData();

	if (pData.useTeleport(data, player->baseData())) {
		LOG_CDEBUG("game") << "Teleport used" << data;

		// Final teleport

		if (!data.dst.isValid() && !data.hd) {
			const int left = std::max(0, player->collectionRq() - player->collection());

			if (left == 0) {
				player->setIsGameCompleted(true);
				emit d->gameSuccess();
			} else {

				if (left > 1)
					d->messageColor(QObject::tr("%1 items missing").arg(left), QColorConstants::Svg::red, false);
				else
					d->messageColor(QObject::tr("1 item missing"), QColorConstants::Svg::red, false);

				if (!player->m_sfxDecline.soundList().isEmpty())
					player->m_sfxDecline.playOne();

				return;
			}
		}

		player->updateFromSnapshot(pData);

		if (!player->m_sfxAccept.soundList().isEmpty())
			player->m_sfxAccept.playOne();
	}
}






/**
 * @brief RpgGamePrivate::playerUseExit
 * @param player
 * @param control
 * @param success
 */

void RpgGamePrivate::playerUseExit(RpgPlayer *player)
{
	Q_ASSERT(player);

	if (!player->isHiding())
		return;

	RpgGameData::Player pData = player->serialize(1);

	RpgControlTeleport *c = d->controlFind<RpgControlTeleport>(pData.pck);

	if (!c)
		return;

	RpgGameData::ControlTeleportBaseData data = c->baseData();

	if (data.x > 0 && data.y > 0)
		player->emplace(cpv(data.x, data.y));

	if (data.a >= 0)
		player->setCurrentAngleForced(data.a);

	player->setHidingObject({});
}




/**
 * @brief RpgGamePrivate::extractInventory
 * @param player
 * @param control
 */

QList<RpgPickable*> RpgGamePrivate::extractInventory(const RpgGameData::Inventory &inventory,
													 const int &scene, const int &owner,
													 const std::function<int()> &nextIdFn,
													 const std::function<cpVect(const int &)> &nextPosFn)
{
	Q_ASSERT(nextIdFn);
	Q_ASSERT(nextPosFn);

	QList<RpgPickable*> list;

	int num=0;

	for (const RpgGameData::InventoryItem &item : inventory.l) {
		TiledScene *s = d->findScene(scene);

		if (!s) {
			LOG_CERROR("game") << "Invalid scene" << scene;
			continue;
		}

		RpgGameData::PickableBaseData data(item.t, owner, scene, nextIdFn());

		const cpVect pos = nextPosFn(num++);

		data.p = QList<float>{(float) pos.x, (float) pos.y};

		RpgPickable *p = d->controlAdd<RpgPickable>(d, s, data);
		p->playSfx();

		list.append(p);
	}

	return list;
}




/**
 * @brief RpgGamePrivate::playerUseContainer
 * @param player
 * @param control
 * @param success
 */

void RpgGamePrivate::playerUseContainer(RpgPlayer *player, RpgControlContainer *control, const bool &success)
{
	Q_ASSERT(control);
	Q_ASSERT(player);

	if (!success)
		return;

	control->setCurrentState(RpgGameData::ControlContainer::ContainerOpen);
	control->setIsActive(false);

	static const auto fn = [control](const int &num) -> cpVect {
		return cpv(control->baseData().x + num*15., control->baseData().y + num*15.);
	};

	extractInventory(control->baseData().inv,
					 control->baseData().s, player->baseData().o,
					 std::bind(&RpgPlayer::nextObjectId, player),
					 fn);
}




/**
 * @brief RpgGamePrivate::playerUsePickable
 * @param player
 * @param control
 * @param success
 */

void RpgGamePrivate::playerUsePickable(RpgPlayer *player, RpgPickable *control, const bool &success)
{
	Q_ASSERT(control);
	Q_ASSERT(player);

	if (!success)
		return;

	RpgGameData::Player pData = player->serialize(1);
	RpgGameData::PickableBaseData data = control->baseData();

	const int pv = pData.pick(data.pt);

	if (pv < 0) {
		player->updateFromSnapshot(pData);

		if (data.pt == RpgGameData::PickableBaseData::PickableBullet)
			d->message(QObject::tr("%1 bullets gained").arg(-pv));

	} else if (pv > 0 && data.pt == RpgGameData::PickableBaseData::PickableTime) {
		if (ActionRpgGame *game = d->actionRpgGame()) {
			game->m_deadlineTick += AbstractGame::TickTimer::msecToTick(pv*1000);
			d->message(QObject::tr("%1 seconds gained").arg(pv));
		} else {
			LOG_CERROR("game") << "Invalid ActionRpgGame";
		}
	} else {
		LOG_CWARNING("game") << "Invalid pickable" << data.pt;
		return;
	}

	control->setIsActive(false);
	control->visualItem()->setVisible(false);

	d->playSfx(QStringLiteral(":/rpg/common/leather_inventory.mp3"), player->scene(), player->bodyPositionF());

}
