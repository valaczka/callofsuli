/*
 * ---- Call of Suli ----
 *
 * rpgarmory.cpp
 *
 * Created on: 2024. 03. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgArmory
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

#include "rpgarmory.h"
#include "rpgaxe.h"
#include "rpgbroadsword.h"
#include "rpgdagger.h"
#include "rpghammer.h"
#include "rpglongbow.h"
#include "rpglongsword.h"
#include "rpgmace.h"
#include "rpgshield.h"
#include "tiledspritehandler.h"
#include "rpgmagestaff.h"
#include "rpggame.h"


/// Static hash

const QHash<RpgGameData::Weapon::WeaponType, QString> RpgArmory::m_layerInfoHash = {
	{ RpgGameData::Weapon::WeaponLongsword, QStringLiteral("longsword") },
	{ RpgGameData::Weapon::WeaponShortbow, QStringLiteral("shortbow") },
	{ RpgGameData::Weapon::WeaponLongbow, QStringLiteral("longbow") },
	{ RpgGameData::Weapon::WeaponDagger, QStringLiteral("dagger") },
	{ RpgGameData::Weapon::WeaponBroadsword, QStringLiteral("broadsword") },
	{ RpgGameData::Weapon::WeaponMageStaff, QStringLiteral("magestaff") },
	{ RpgGameData::Weapon::WeaponAxe, QStringLiteral("axe") },
	{ RpgGameData::Weapon::WeaponMace, QStringLiteral("mace") },
	{ RpgGameData::Weapon::WeaponHammer, QStringLiteral("hammer") },
	/*{ RpgGameData::Weapon::WeaponShield, QStringLiteral("shield") }*/
};


/**
 * @brief RpgArmory::RpgArmory
 * @param parentObject
 * @param parent
 */

RpgArmory::RpgArmory(TiledObject *parentObject, QObject *parent)
	: QObject{parent}
	, m_parentObject(parentObject)
	, m_weaponList(new RpgWeaponList)
{

}



/**
 * @brief RpgArmory::~RpgArmory
 */

RpgArmory::~RpgArmory()
{

}


/**
 * @brief RpgArmory::changeToNextWeapon
 * @return
 */

bool RpgArmory::changeToNextWeapon()
{
	if (!m_nextWeapon)
		return false;

	setCurrentWeapon(m_nextWeapon);
	return true;
}





/**
 * @brief RpgArmory::weaponList
 * @return
 */

RpgWeaponList *RpgArmory::weaponList() const
{
	return m_weaponList.get();
}


/**
 * @brief RpgArmory::weaponFind
 * @param type
 * @return
 */

RpgWeapon *RpgArmory::weaponFind(const RpgGameData::Weapon::WeaponType &type) const
{
	for (RpgWeapon *w : std::as_const(*m_weaponList)) {
		if (w->weaponType() == type)
			return w;
	}

	return nullptr;
}



/**
 * @brief RpgArmory::weaponAdd
 * @param type
 * @return
 */

RpgWeapon *RpgArmory::weaponAdd(const RpgGameData::Weapon::WeaponType &type)
{
	if (RpgWeapon *w = weaponFind(type)) {
		return w;
	}

	RpgWeapon *weapon = nullptr;

	switch (type) {
		case RpgGameData::Weapon::WeaponLongsword:
			weapon = weaponAdd(new RpgLongsword);
			break;

		case RpgGameData::Weapon::WeaponShortbow:
			weapon = weaponAdd(new RpgShortbow);
			break;

		case RpgGameData::Weapon::WeaponLongbow:
			weapon = weaponAdd(new RpgLongbow);
			break;

		case RpgGameData::Weapon::WeaponDagger:
			weapon = weaponAdd(new RpgDagger);
			break;

		case RpgGameData::Weapon::WeaponBroadsword:
			weapon = weaponAdd(new RpgBroadsword);
			break;

		case RpgGameData::Weapon::WeaponAxe:
			weapon = weaponAdd(new RpgAxe);
			break;

		case RpgGameData::Weapon::WeaponMace:
			weapon = weaponAdd(new RpgMace);
			break;

		case RpgGameData::Weapon::WeaponHammer:
			weapon = weaponAdd(new RpgHammer);
			break;

		case RpgGameData::Weapon::WeaponShield:
			weapon = weaponAdd(new RpgShield);
			break;

		case RpgGameData::Weapon::WeaponHand:
			weapon = weaponAdd(new RpgWeaponHand);
			break;

		case RpgGameData::Weapon::WeaponMageStaff:
		case RpgGameData::Weapon::WeaponGreatHand:
		case RpgGameData::Weapon::WeaponLightningWeapon:
		case RpgGameData::Weapon::WeaponFireFogWeapon:
		case RpgGameData::Weapon::WeaponInvalid:
			LOG_CERROR("game") << "Invalid weapon type" << type;
			return nullptr;
	}

	return weapon;
}





/**
 * @brief RpgArmory::weaponAdd
 * @param weapon
 * @return
 */

RpgWeapon *RpgArmory::weaponAdd(RpgWeapon *weapon)
{
	Q_ASSERT(weapon);

	m_weaponList->append(weapon);
	weapon->setParentObject(m_parentObject);
	updateLayers();

	if (weapon->weaponType() == RpgGameData::Weapon::WeaponMageStaff)
		emit mageStaffChanged();

	return weapon;
}


/**
 * @brief RpgArmory::weaponRemove
 * @param weapon
 */

void RpgArmory::weaponRemove(RpgWeapon *weapon)
{
	if (!weapon)
		return;

	weapon->setParentObject(nullptr);
	m_weaponList->remove(weapon);

	if (weapon->weaponType() == RpgGameData::Weapon::WeaponMageStaff)
		emit mageStaffChanged();

	if (m_currentWeapon == weapon) {
		if (m_nextWeapon == weapon) {
			setNextWeapon(nullptr);
			if (m_weaponList->empty()) {
				setCurrentWeapon(nullptr);
			} else {
				setCurrentWeapon(m_weaponList->first());
			}
		} else {
			if (m_nextWeapon)
				setCurrentWeapon(m_nextWeapon);
			else if (m_weaponList->empty())
				setCurrentWeapon(nullptr);
			else
				setCurrentWeapon(m_weaponList->first());
		}
	} else {
		setNextWeapon(getNextWeapon());
	}
}


/**
 * @brief RpgArmory::currentWeapon
 * @return
 */

RpgWeapon *RpgArmory::currentWeapon() const
{
	return m_currentWeapon;
}

void RpgArmory::setCurrentWeapon(RpgWeapon *newCurrentWeapon)
{
	if (m_currentWeapon == newCurrentWeapon)
		return;
	m_currentWeapon = newCurrentWeapon;
	emit currentWeaponChanged();
	updateLayers();

	setNextWeapon(getNextWeapon());
}


/**
 * @brief RpgArmory::baseLayers
 * @return
 */

QStringList RpgArmory::baseLayers() const
{
	return m_baseLayers;
}

void RpgArmory::setBaseLayers(const QStringList &newBaseLayers)
{
	if (m_baseLayers == newBaseLayers)
		return;
	m_baseLayers = newBaseLayers;
	emit baseLayersChanged();
	updateLayers();
}


/**
 * @brief RpgArmory::updateNextWeapon
 */

void RpgArmory::updateNextWeapon()
{
	setNextWeapon(getNextWeapon());
}


/**
 * @brief RpgArmory::setCurrentWeaponIf
 * @param newCurrentWeapon
 * @param currentType
 */

void RpgArmory::setCurrentWeaponIf(RpgWeapon *newCurrentWeapon, const RpgGameData::Weapon::WeaponType &currentType)
{
	if (!m_currentWeapon || m_currentWeapon->weaponType() == currentType)
		setCurrentWeapon(newCurrentWeapon);
	else
		updateNextWeapon();
}



RpgWeapon *RpgArmory::nextWeapon() const
{
	return m_nextWeapon;
}

void RpgArmory::setNextWeapon(RpgWeapon *newNextWeapon)
{
	if (m_nextWeapon == newNextWeapon)
		return;
	m_nextWeapon = newNextWeapon;
	emit nextWeaponChanged();
}


/**
 * @brief RpgArmory::getNextWeapon
 * @return
 */

RpgWeapon *RpgArmory::getNextWeapon() const
{
	if (m_weaponList->empty())
		return nullptr;

	const int index = m_weaponList->indexOf(m_currentWeapon);

	QVector<RpgWeapon *> wList;
	RpgWeapon *weaponHand = nullptr;

	auto start = m_weaponList->constBegin();

	if (index != -1)
		start += index;

	auto it = (index != -1) ? start+1 : start;

	if (it == m_weaponList->constEnd())
		it = m_weaponList->constBegin();

	do {
		/*if ((*it)->canAttack()) { */
			if ((*it)->weaponType() == RpgGameData::Weapon::WeaponHand)
				weaponHand = *it;
			else if ((*it)->canHit() || (*it)->canShot()) {
				if (RpgPlayer *p = qobject_cast<RpgPlayer*>(m_parentObject); p && (*it)->weaponType() == RpgGameData::Weapon::WeaponMageStaff) {
					if (p->mp() > 0)
						wList.append(*it);
				} else {
					wList.append(*it);
				}
			}
		/*}*/

		++it;

		if (it == m_weaponList->constEnd())
			it = m_weaponList->constBegin();
	} while (it != start);

	if (wList.isEmpty()) {
		if (weaponHand)
			return weaponHand;

		LOG_CWARNING("game") << "No available weapon";
		return nullptr;
	}

	return wList.first();
}


/**
 * @brief RpgArmory::mageStaff
 * @return
 */

RpgMageStaff *RpgArmory::mageStaff() const
{
	return qobject_cast<RpgMageStaff*>(weaponFind(RpgGameData::Weapon::WeaponMageStaff));
}






/**
 * @brief RpgArmory::serialize
 * @return
 */

RpgGameData::Armory RpgArmory::serialize() const
{
	RpgGameData::Armory arm;

	for (RpgWeapon *weapon : std::as_const(*m_weaponList))
		arm.wl.append(weapon->serialize());

	std::sort(arm.wl.begin(), arm.wl.end(), [](const RpgGameData::Weapon &w1, const RpgGameData::Weapon &w2) {
		return w1.t < w2.t;
	});

	arm.cw = m_currentWeapon ? m_currentWeapon->weaponType() : RpgGameData::Weapon::WeaponInvalid;

	return arm;
}





/**
 * @brief RpgArmory::updateFromSnapshot
 * @param armory
 * @return
 */

bool RpgArmory::updateFromSnapshot(const RpgGameData::Armory &armory)
{
	QList<RpgWeapon *> tmp;

	tmp.reserve(m_weaponList->size());
	for (RpgWeapon *w : *m_weaponList)
		tmp.append(w);

	for (const RpgGameData::Weapon &w : armory.wl) {
		if (RpgWeapon *wp = weaponFind(w.t)) {
			wp->updateFromSnapshot(w);
			tmp.removeAll(wp);
		} else {
			if (RpgWeapon *ww = weaponAdd(w.t))
				ww->updateFromSnapshot(w.t);
		}
	}

	for (RpgWeapon *w : tmp)
		weaponRemove(w);

	return true;
}




/**
 * @brief RpgArmory::updateLayers
 */

void RpgArmory::updateLayers()
{
	if (!m_parentObject || !m_parentObject->spriteHandler())
		return;

	TiledSpriteHandler *handler = m_parentObject->spriteHandler();

	QStringList layers = m_baseLayers;
	QStringList loadableLayers;

	bool addMageStaff = false;

	if (m_currentWeapon && !m_currentWeapon->excludeFromLayers()) {
		switch (m_currentWeapon->weaponType()) {
			case RpgGameData::Weapon::WeaponShortbow:
			case RpgGameData::Weapon::WeaponLongbow:
			case RpgGameData::Weapon::WeaponLongsword:
			case RpgGameData::Weapon::WeaponDagger:
			case RpgGameData::Weapon::WeaponBroadsword:
			case RpgGameData::Weapon::WeaponAxe:
			case RpgGameData::Weapon::WeaponMace:
			case RpgGameData::Weapon::WeaponHammer:
				layers.append(m_layerInfoHash.value(m_currentWeapon->weaponType()));
				loadableLayers.append(m_layerInfoHash.value(m_currentWeapon->weaponType()));
				break;

			case RpgGameData::Weapon::WeaponMageStaff:
			case RpgGameData::Weapon::WeaponShield:
			case RpgGameData::Weapon::WeaponHand:
			case RpgGameData::Weapon::WeaponGreatHand:
			case RpgGameData::Weapon::WeaponLightningWeapon:
			case RpgGameData::Weapon::WeaponFireFogWeapon:
			case RpgGameData::Weapon::WeaponInvalid:
				addMageStaff = true;
				break;
		}
	}

	if (addMageStaff) {
		if (auto it = std::find_if(m_weaponList->cbegin(), m_weaponList->cend(),
								   [](RpgWeapon *w) {
								   return w->weaponType() == RpgGameData::Weapon::WeaponMageStaff && !w->excludeFromLayers();
	}); it != m_weaponList->cend()) {
			layers.append(m_layerInfoHash.value(RpgGameData::Weapon::WeaponMageStaff));
			loadableLayers.append(m_layerInfoHash.value(RpgGameData::Weapon::WeaponMageStaff));
		}
	}

	if (auto it = std::find_if(m_weaponList->cbegin(), m_weaponList->cend(),
							   [](RpgWeapon *w) {
							   return w->weaponType() == RpgGameData::Weapon::WeaponShield && !w->excludeFromLayers();
}); it != m_weaponList->cend()) {
		if ((*it)->bulletCount() > 0)
			layers.append(QStringLiteral("shield"));
	}


	for (const QString &s : loadableLayers) {
		if (auto *h = m_parentObject->spriteHandler(); !h->layers().contains(s)) {
			RpgGame::loadBaseTextureSprites(h, QStringLiteral(":/rpg/")+s+QStringLiteral("/"), s);
		}
	}

	handler->setVisibleLayers(layers);
}



/**
 * @brief RpgArmory::getShieldCount
 * @return
 */

int RpgArmory::getShieldCount() const
{
	int n = 0;

	for (RpgWeapon *w : *m_weaponList) {
		if (w->weaponType() == RpgGameData::Weapon::WeaponShield) {
			if (w->bulletCount() > 0)
				n += w->bulletCount();
		}
	}

	return n;
}




/**
 * @brief RpgWeapon::RpgWeapon
 * @param type
 * @param parent
 */

RpgWeapon::RpgWeapon(const RpgGameData::Weapon::WeaponType &type, QObject *parent)
	: TiledWeapon(parent)
	, m_weaponType(type)
{

}




/**
 * @brief RpgWeapon::weaponName
 * @param type
 * @return
 */

QString RpgWeapon::weaponName(const RpgGameData::Weapon::WeaponType &type)
{
	switch (type) {
		case RpgGameData::Weapon::WeaponShortbow: return tr("íj");
		case RpgGameData::Weapon::WeaponLongbow: return tr("longbow");
		case RpgGameData::Weapon::WeaponGreatHand: return tr("kéz");
		case RpgGameData::Weapon::WeaponHand: return tr("kéz");
		case RpgGameData::Weapon::WeaponShield: return tr("pajzs");
		case RpgGameData::Weapon::WeaponLongsword: return tr("kard");
		case RpgGameData::Weapon::WeaponDagger: return tr("tőr");
		case RpgGameData::Weapon::WeaponBroadsword: return tr("pallos");
		case RpgGameData::Weapon::WeaponMageStaff: return QStringLiteral("varázsbot");
		case RpgGameData::Weapon::WeaponLightningWeapon: return QStringLiteral("villám");
		case RpgGameData::Weapon::WeaponFireFogWeapon: return QStringLiteral("tűz");
		case RpgGameData::Weapon::WeaponAxe: return tr("balta");
		case RpgGameData::Weapon::WeaponMace: return tr("buzogány");
		case RpgGameData::Weapon::WeaponHammer: return tr("kalapács");
		case RpgGameData::Weapon::WeaponInvalid: return tr("érvénytelen");
	}

	return {};
}




/**
 * @brief RpgWeapon::weaponNameEn
 * @param type
 * @return
 */

QString RpgWeapon::weaponNameEn(const RpgGameData::Weapon::WeaponType &type)
{
	switch (type) {
		case RpgGameData::Weapon::WeaponShortbow: return QStringLiteral("Shortbow");
		case RpgGameData::Weapon::WeaponLongbow: return QStringLiteral("Longbow");
		case RpgGameData::Weapon::WeaponGreatHand: return QStringLiteral("Hand");
		case RpgGameData::Weapon::WeaponHand: return QStringLiteral("Hand");
		case RpgGameData::Weapon::WeaponShield: return QStringLiteral("Shield");
		case RpgGameData::Weapon::WeaponLongsword: return QStringLiteral("Sword");
		case RpgGameData::Weapon::WeaponBroadsword: return QStringLiteral("Broadsword");
		case RpgGameData::Weapon::WeaponDagger: return QStringLiteral("Dagger");
		case RpgGameData::Weapon::WeaponMageStaff: return QStringLiteral("Mage staff");
		case RpgGameData::Weapon::WeaponAxe: return tr("Axe");
		case RpgGameData::Weapon::WeaponMace: return tr("Mace");
		case RpgGameData::Weapon::WeaponHammer: return tr("Hammer");
		case RpgGameData::Weapon::WeaponLightningWeapon: return QStringLiteral("Lightning");
		case RpgGameData::Weapon::WeaponFireFogWeapon: return QStringLiteral("Fire fog");
		case RpgGameData::Weapon::WeaponInvalid: return QStringLiteral("Invalid");
	}

	return {};
}



/**
 * @brief RpgWeapon::serialize
 * @return
 */

RpgGameData::Weapon RpgWeapon::serialize() const
{
	RpgGameData::Weapon w;
	w.t = m_weaponType;
	w.b = m_bulletCount;
	return w;
}




/**
 * @brief RpgWeapon::updateFromSnapshot
 * @param weapon
 * @return
 */

bool RpgWeapon::updateFromSnapshot(const RpgGameData::Weapon &weapon)
{
	if (weapon.t != m_weaponType) {
		LOG_CERROR("game") << "Snapshot type mismatch" << this << weapon.t;
		return false;
	}

	setBulletCount(weapon.b);

	return true;
}





/**
 * @brief RpgBullet::RpgBullet
 * @param scene
 */

RpgBullet::RpgBullet(const RpgGameData::Weapon::WeaponType &weaponType, TiledScene *scene)
	: IsometricBullet(scene)
	, RpgGameDataInterface<RpgGameData::Bullet, RpgGameData::BulletBaseData>()
	, m_weaponType(weaponType)
{

}



/**
 * @brief RpgBullet::~RpgBullet
 */

RpgBullet::~RpgBullet()
{

}


/**
 * @brief RpgBullet::baseData
 * @return
 */

RpgGameData::BulletBaseData RpgBullet::baseData() const
{
	RpgGameData::BulletBaseData d = RpgGameDataInterface::baseData();

	d.t = m_weaponType;
	d.own = m_owner;
	d.tar = m_targets;

	return d;
}



/**
 * @brief RpgBullet::serializeThis
 * @return
 */

std::unique_ptr<RpgGameData::Body> RpgBullet::serializeThis() const
{
	RpgGameData::Bullet *p = new RpgGameData::Bullet();

	b2Vec2 pos = body().GetPosition();
	p->p = { pos.x, pos.y };
	p->a = currentAngle();

	if (TiledScene *s = scene())
		p->sc = s->sceneId();

	std::unique_ptr<RpgGameData::Body> ptr(std::move(p));
	return ptr;
}





/**
 * @brief RpgBullet::impactEvent
 * @param base
 */

void RpgBullet::impactEvent(TiledObjectBody *base)
{
	/*const FixtureCategories categories = FixtureCategories::fromInt(other.GetFilter().categoryBits);
	IsometricEnemy *enemy = categories.testFlag(FixtureTarget) || categories.testFlag(FixtureEnemyBody) ?
								dynamic_cast<IsometricEnemy*>(base) :
								nullptr;

	IsometricPlayer *player = categories.testFlag(FixtureTarget) || categories.testFlag(FixturePlayerBody)  ?
								  dynamic_cast<IsometricPlayer*>(base) :
								  nullptr;



	if (categories.testFlag(TiledObjectBody::FixtureGround) && base->opaque()) {
		setImpacted(true);
		stop();
		groundEvent(base);
		doAutoDelete();
		return;
	}


	bool hasTarget = false;

	if (m_targets.testFlag(TargetEnemy) && enemy) {
		hasTarget = enemy->canBulletImpact(d->m_fromWeaponType);
	}

	if (m_targets.testFlag(TargetPlayer) && player && !player->isLocked()) {
		hasTarget = true;
	}

	if (!hasTarget)
		return;



	if (!d->m_owner) {
		LOG_CWARNING("game") << "Missing owner, bullet automatic impact event failed";
		return;
	}

	TiledGame *game = d->m_owner->game();

	if (!game) {
		LOG_CWARNING("game") << "Missing game, bullet automatic impact event failed";
		return;
	}


	IsometricEnemy *enemy = dynamic_cast<IsometricEnemy*>(base);
	IsometricPlayer *player = dynamic_cast<IsometricPlayer*>(base);

	LOG_CINFO("game") << "IMPACT" << d->m_owner << enemy << player << d->m_fromWeaponType;

	if (enemy)
		game->playerAttackEnemy(d->m_owner, enemy, d->m_fromWeaponType);

	if (player)
		game->enemyAttackPlayer(d->m_owner, player, d->m_fromWeaponType);

	/// TODO: player attack player?

	setImpacted(true);
	stop();
	doAutoDelete();*/
}



/**
 * @brief RpgBullet::rpgGame
 * @return
 */

RpgGame *RpgBullet::rpgGame() const
{
	return qobject_cast<RpgGame*>(game());
}



/**
 * @brief RpgBullet::weaponType
 * @return
 */

RpgGameData::Weapon::WeaponType RpgBullet::weaponType() const
{
	return m_weaponType;
}

RpgGameData::BulletBaseData::Targets RpgBullet::targets() const
{
	return m_targets;
}

void RpgBullet::setTargets(const RpgGameData::BulletBaseData::Targets &newTargets)
{
	if (m_targets == newTargets)
		return;
	m_targets = newTargets;
	emit targetsChanged();
}



/**
 * @brief RpgBullet::owner
 * @return
 */

RpgGameData::BulletBaseData::Owner RpgBullet::owner() const
{
	return m_owner;
}

void RpgBullet::setOwner(const RpgGameData::BulletBaseData::Owner &newOwner)
{
	if (m_owner == newOwner)
		return;
	m_owner = newOwner;
	emit ownerChanged();
}



/**
 * @brief RpgBullet::ownerEntity
 * @return
 */

IsometricEntity *RpgBullet::ownerEntity() const
{
	return m_ownerEntity;
}


/**
 * @brief RpgBullet::setOwnerEntity
 * @param newOwnerEntity
 */

void RpgBullet::setOwnerEntity(IsometricEntity *newOwnerEntity)
{
	m_ownerEntity = newOwnerEntity;
}



/**
 * @brief RpgWeaponHand::RpgWeaponHand
 * @param parent
 */

RpgWeaponHand::RpgWeaponHand(QObject *parent)
	: RpgWeapon(RpgGameData::Weapon::WeaponHand, parent)
{
		m_bulletCount = -1;
		m_canHit = true;
		m_icon = QStringLiteral("qrc:/internal/medal/Icon.3_31.png");
}



/**
 * @brief RpgWeaponHand::eventAttack
 * @param target
 */

void RpgWeaponHand::eventAttack(TiledObject *target)
{
	if (!m_parentObject) {
		LOG_CERROR("game") << "Missing parent object" << this;
		return;
	}

	TiledObject *p = m_parentObject.data();

	if (TiledGame *g = p->game(); g && target) {
		g->playSfx(QStringLiteral(":/rpg/common/hit.mp3"), p->scene(), p->bodyPosition());
	}
}
