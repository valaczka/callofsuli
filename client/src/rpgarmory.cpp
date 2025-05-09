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
#include "actionrpgmultiplayergame.h"
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
#include "actionrpggame.h"


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

	auto ptr = weaponCreate(type);

	if (!ptr) {
		return nullptr;
	} else {
		return weaponAdd(ptr.release());
	}
}



/**
 * @brief RpgArmory::weaponCreate
 * @param type
 * @return
 */

std::unique_ptr<RpgWeapon> RpgArmory::weaponCreate(const RpgGameData::Weapon::WeaponType &type, QObject *parent)
{
	switch (type) {
		case RpgGameData::Weapon::WeaponLongsword:
			return std::make_unique<RpgLongsword>(parent);

		case RpgGameData::Weapon::WeaponShortbow:
			return std::make_unique<RpgShortbow>(parent);

		case RpgGameData::Weapon::WeaponLongbow:
			return std::make_unique<RpgLongbow>(parent);

		case RpgGameData::Weapon::WeaponDagger:
			return std::make_unique<RpgDagger>(parent);

		case RpgGameData::Weapon::WeaponBroadsword:
			return std::make_unique<RpgBroadsword>(parent);

		case RpgGameData::Weapon::WeaponAxe:
			return std::make_unique<RpgAxe>(parent);

		case RpgGameData::Weapon::WeaponMace:
			return std::make_unique<RpgMace>(parent);

		case RpgGameData::Weapon::WeaponHammer:
			return std::make_unique<RpgHammer>(parent);

		case RpgGameData::Weapon::WeaponShield:
			return std::make_unique<RpgShield>(parent);

		case RpgGameData::Weapon::WeaponHand:
			return std::make_unique<RpgWeaponHand>(parent);

		case RpgGameData::Weapon::WeaponMageStaff:
		case RpgGameData::Weapon::WeaponGreatHand:
		case RpgGameData::Weapon::WeaponLightningWeapon:
		case RpgGameData::Weapon::WeaponFireFogWeapon:
		case RpgGameData::Weapon::WeaponInvalid:
			LOG_CERROR("game") << "Invalid weapon type" << type;
	}

	return nullptr;
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
 * @brief RpgArmory::parentObject
 * @return
 */

TiledObject *RpgArmory::parentObject() const
{
	return m_parentObject;
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
				ww->updateFromSnapshot(w);
		}
	}

	for (RpgWeapon *w : tmp)
		weaponRemove(w);


	if (RpgWeapon *w = weaponFind(armory.cw))
		setCurrentWeapon(w);

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

RpgBullet::RpgBullet(const RpgGameData::Weapon::WeaponType &weaponType, TiledGame *game, const cpBodyType &type)
	: IsometricBullet(game, type)
	, RpgGameDataInterface<RpgGameData::Bullet, RpgGameData::BulletBaseData>()
	, RpgGameData::LifeCycle()
	, m_weaponType(weaponType)
{
	m_path.first = cpvzero;
	m_path.second = cpvzero;
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
	d.ownId = m_ownerId;
	d.pth = QList<float>({
							 (float) m_path.first.x,
							 (float) m_path.first.y,
							 (float) m_path.second.x,
							 (float) m_path.second.y
						 });


	return d;
}


/**
 * @brief RpgBullet::shot
 * @param baseData
 */

void RpgBullet::shot(const RpgGameData::BulletBaseData &baseData)
{
	if (baseData.pth.size() < 4) {
		LOG_CERROR("game") << "Invalid path" << baseData.pth;
		m_stage = StageDead;
		return;
	}

	m_path.first = cpv(baseData.pth.at(0), baseData.pth.at(1));
	m_path.second = cpv(baseData.pth.at(2),baseData.pth.at(3));

	IsometricBullet::shot(m_path.first, cpvtoangle(cpvsub(m_path.second, m_path.first)));

	m_stage = StageLive;

}



/**
 * @brief RpgBullet::shot
 * @param from
 * @param angle
 */

void RpgBullet::shot(const cpVect &from, const qreal &angle)
{
	m_path.first = from;
	m_path.second = cpvadd(from, vectorFromAngle(angle, m_maxDistance));

	IsometricBullet::shot(from, angle);

	m_stage = StageLive;
}



/**
 * @brief RpgBullet::serializeThis
 * @return
 */

RpgGameData::Bullet RpgBullet::serializeThis() const
{
	RpgGameData::Bullet p;

	float progress = 0.;

	shortestDistance(bodyPosition(), m_path.first, m_path.second, nullptr, &progress);

	p.p = progress;
	p.st = m_stage;
	p.tg = m_impactedObject;

	if (TiledScene *s = scene())
		p.sc = s->sceneId();

	return p;
}



/**
 * @brief RpgBullet::updateFromSnapshot
 * @param snapshot
 */

void RpgBullet::updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::Bullet> &snapshot)
{
	updateFromSnapshot(snapshot.s1);

	if (snapshot.s1.f < 0 && snapshot.last.f < 0) {
		LOG_CERROR("scene") << "Invalid tick" << snapshot.s1.f << snapshot.s2.f << snapshot.last.f << snapshot.current;
		return stop();
	}

	if ((m_stage == StageDestroy || m_stage == StageDead) && m_impactedObject.isValid() && !m_impactRendered) {
		if (RpgGame *g = qobject_cast<RpgGame*>(m_game); g && g->actionRpgGame()) {
			if (RpgPlayer *player = dynamic_cast<RpgPlayer*>(g->findBody(
																 TiledObjectBody::ObjectId{
																 .ownerId = snapshot.s1.tg.o,
																 .sceneId = snapshot.s1.tg.s,
																 .id = snapshot.s1.tg.id
		}))) {
				RpgEnemy *enemy = dynamic_cast<RpgEnemy*>(g->findBody(
															  TiledObjectBody::ObjectId{
																  .ownerId = m_ownerId.o,
																  .sceneId = m_ownerId.s,
																  .id = m_ownerId.id,
															  }));

				LOG_CINFO("scene") << "REAL BULLET ATTACK PLAYER" << snapshot.current << snapshot.s1.f <<
									  snapshot.s1.p << player->objectId().ownerId << enemy;

				player->attackedByEnemy(g->controlledPlayer() == player ? enemy : nullptr,
										m_weaponType, false);
			}

		}

		LOG_CINFO("scene") << "attacked";

		m_impactRendered = true;
	}

	if (m_stage != StageLive)
		return;


	const RpgGameData::Bullet &from = snapshot.s1.f >= 0 ? snapshot.s1 : snapshot.last;
	const RpgGameData::Bullet &to = snapshot.s2.f >= 0 ? snapshot.s2 : snapshot.last;

	if (from.f < 0 || to.f < 0 || from.f > to.f)
		return stop();


	const cpVect line = cpvsub(m_path.second, m_path.first);
	if (cpvlengthsq(line) < m_speed/60.) {
		LOG_CERROR("scene") << "Invalid path" << m_path.first.x << m_path.first.y << "->" << m_path.second.x << m_path.second.y;
		return stop();
	}

	const cpVect speed = cpvmult(cpvnormalize(line), m_speed);

	const cpVect final = cpvlerp(m_path.first, m_path.second, to.p);


	if (to.f <= snapshot.current) {
		const int frames = snapshot.current-to.f+1;
		const cpVect pos = cpvadd(final, cpvmult(speed, frames/60.));

		moveToPoint(pos, frames);
		return;
	} else {
		const int frames = to.f-snapshot.current;
		moveToPoint(final, frames);
		return;
	}
}




/**
 * @brief RpgBullet::updateFromSnapshot
 * @param snap
 */

void RpgBullet::updateFromSnapshot(const RpgGameData::Bullet &snap)
{
	if (((snap.st == StageDead || snap.st == StageDestroy) &&
		 m_stage != StageDead && m_stage != StageDestroy))
	{
		disableBullet();
	}

	m_stage = snap.st;
	m_impactedObject = snap.tg;
}






/**
 * @brief RpgBullet::impactEvent
 * @param base
 */

void RpgBullet::impactEvent(TiledObjectBody *base, cpShape *shape)
{
	if (m_stage != StageLive) {
		disableBullet();
		return;
	}

	if (!base)
		return;


	// Multiplayerben ez nem nem érdekes, ha nem a sajátunk

	ActionRpgMultiplayerGame *g = dynamic_cast<ActionRpgMultiplayerGame*>(rpgGame()->actionRpgGame());

	if (g) {
		if (m_owner == RpgGameData::BulletBaseData::OwnerPlayer && m_ownerId.o != g->playerId())
			return;

		if (m_owner == RpgGameData::BulletBaseData::OwnerEnemy && g->gameMode() != ActionRpgGame::MultiPlayerHost)
			return;
	}

	const FixtureCategories categories = FixtureCategories::fromInt(cpShapeGetFilter(shape).categories);

	if (categories.testFlag(TiledObjectBody::FixtureGround) && base->opaque()) {
		m_impactedObject = RpgGameData::BaseData(
							   base->objectId().ownerId,
							   base->objectId().sceneId,
							   base->objectId().id
							   );

		if (RpgGame *g = rpgGame())
			g->bulletImpact(this, base);

		return;
	}

	RpgEnemy *enemy = categories.testFlag(FixtureTarget) || categories.testFlag(FixtureEnemyBody) ?
						  dynamic_cast<RpgEnemy*>(base) :
						  nullptr;

	RpgPlayer *player = categories.testFlag(FixtureTarget) || categories.testFlag(FixturePlayerBody) ?
							dynamic_cast<RpgPlayer*>(base) :
							nullptr;



	bool hasTarget = false;

	if (m_targets.testFlag(RpgGameData::BulletBaseData::TargetEnemy) && enemy && enemy->isAlive()) {
		hasTarget = enemy->canBulletImpact(m_weaponType);
	}

	if (m_targets.testFlag(RpgGameData::BulletBaseData::TargetPlayer) && player && player->isAlive() && !player->isLocked()) {
		hasTarget = true;
	}

	if (!hasTarget)
		return;

	m_impactedObject = RpgGameData::BaseData(
						   base->objectId().ownerId,
						   base->objectId().sceneId,
						   base->objectId().id
						   );

	if (RpgGame *g = rpgGame())
		g->bulletImpact(this, base);
}


/**
 * @brief RpgBullet::overshootEvent
 */

void RpgBullet::overshootEvent()
{
	if (m_stage != StageLive) {
		disableBullet();
		return;
	}

	m_impactedObject = {};
	m_stage = StageDead;
	disableBullet();
}



/**
 * @brief RpgBullet::rpgGame
 * @return
 */

RpgGame *RpgBullet::rpgGame() const
{
	return qobject_cast<RpgGame*>(game());
}

const RpgGameData::BaseData &RpgBullet::ownerId() const
{
	return m_ownerId;
}

void RpgBullet::setOwnerId(const RpgGameData::BaseData &newOwnerId)
{
	m_ownerId = newOwnerId;
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
		g->playSfx(QStringLiteral(":/rpg/common/hit.mp3"), p->scene(), p->bodyPositionF());
	}
}
