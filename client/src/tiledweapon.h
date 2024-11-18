/*
 * ---- Call of Suli ----
 *
 * tiledweapon.h
 *
 * Created on: 2024. 03. 18.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledWeapon
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

#ifndef TILEDWEAPON_H
#define TILEDWEAPON_H

#include "tiledobject.h"
#include "isometricbullet.h"
#include <QObject>

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"

class TiledWeapon;
using TiledWeaponList = qolm::QOlm<TiledWeapon>;
Q_DECLARE_METATYPE(TiledWeaponList*)


class IsometricEnemy;


/**
 * @brief The TiledWeapon class
 */

class TiledWeapon : public QObject
{
	Q_OBJECT

	Q_PROPERTY(WeaponType weaponType READ weaponType CONSTANT FINAL)
	Q_PROPERTY(TiledObject *parentObject READ parentObject WRITE setParentObject NOTIFY parentObjectChanged FINAL)
	Q_PROPERTY(int bulletCount READ bulletCount WRITE setBulletCount NOTIFY bulletCountChanged FINAL)
	Q_PROPERTY(int maxBulletCount READ maxBulletCount WRITE setMaxBulletCount NOTIFY maxBulletCountChanged FINAL)
	Q_PROPERTY(bool canShot READ canShot NOTIFY canShotChanged FINAL)
	Q_PROPERTY(bool canHit READ canHit WRITE setCanHit NOTIFY canHitChanged FINAL)
	Q_PROPERTY(bool canCast READ canCast WRITE setCanCast NOTIFY canCastChanged FINAL)
	Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged FINAL)
	Q_PROPERTY(qint64 repeaterIdle READ repeaterIdle WRITE setRepeaterIdle NOTIFY repeaterIdleChanged FINAL)
	Q_PROPERTY(bool excludeFromLayers READ excludeFromLayers WRITE setExcludeFromLayers NOTIFY excludeFromLayersChanged FINAL)

public:
	enum WeaponType {
		WeaponInvalid = 0,
		WeaponHand,
		WeaponDagger,
		WeaponLongsword,
		WeaponShortbow,
		WeaponLongbow,
		WeaponBroadsword,
		WeaponAxe,
		WeaponHammer,
		WeaponMace,
		WeaponGreatHand,
		WeaponMageStaff,
		WeaponLightningWeapon,
		WeaponFireFogWeapon,
		WeaponShield
	};

	Q_ENUM (WeaponType);

	explicit TiledWeapon(const WeaponType &type, QObject *parent = nullptr);
	virtual ~TiledWeapon();

	WeaponType weaponType() const;

	static QString weaponName(const WeaponType &type);
	QString weaponName() const { return weaponName(m_weaponType); }

	static QString weaponNameEn(const WeaponType &type);
	QString weaponNameEn() const { return weaponNameEn(m_weaponType); }

	bool shot(const IsometricBullet::Targets &targets, const QPointF &from, const TiledObject::Direction &direction,
			  const qreal &distance = 0.);
	bool shot(const IsometricBullet::Targets &targets, const QPointF &from, const qreal &angle,
			  const qreal &distance = 0.);

	bool canShot() const { return !m_canHit && m_bulletCount != 0; }

	virtual bool protect(const WeaponType &weapon) = 0;
	virtual bool canProtect(const WeaponType &weapon) const = 0;
	virtual bool canAttack() const = 0;

	bool canHit() const { return m_canHit && m_bulletCount != 0; }

	void setCanHit(bool newCanHit);

	bool hit(TiledObject *target);

	TiledObject *parentObject() const;
	void setParentObject(TiledObject *newParentObject);

	int bulletCount() const;
	void setBulletCount(int newBulletCount);

	QString icon() const;
	void setIcon(const QString &newIcon);

	qint64 repeaterIdle() const;
	void setRepeaterIdle(qint64 newRepeaterIdle);

	bool canThrow() const;
	void setCanThrow(bool newCanThrow);

	bool canThrowBullet() const;
	void setCanThrowBullet(bool newCanThrowBullet);

	int maxBulletCount() const;
	void setMaxBulletCount(int newMaxBulletCount);

	bool excludeFromLayers() const;
	void setExcludeFromLayers(bool newExcludeFromLayers);

	int pickedBulletCount() const;
	void setPickedBulletCount(int newPickedBulletCount);

	bool canCast() const;
	void setCanCast(bool newCanCast);

	bool disableTimerRepeater() const;
	void setDisableTimerRepeater(bool newDisableTimerRepeater);

signals:
	void parentObjectChanged();
	void bulletCountChanged();
	void canShotChanged();
	void canHitChanged();
	void iconChanged();
	void repeaterIdleChanged();
	void maxBulletCountChanged();
	void excludeFromLayersChanged();
	void canCastChanged();

protected:
	virtual IsometricBullet *createBullet(const qreal &distance = 0.) = 0;
	virtual void eventAttack(TiledObject *target) { Q_UNUSED(target); }
	virtual void eventProtect() {}

	QPointer<TiledObject> m_parentObject;
	int m_bulletCount = 0;
	int m_maxBulletCount = 0;
	bool m_canHit = false;
	bool m_canCast = false;
	QString m_icon;
	qint64 m_repeaterIdle = 125;
	bool m_excludeFromLayers = false;
	bool m_disableTimerRepeater = false;

private:
	const WeaponType m_weaponType;
	qint64 m_timerRepeater = -1.;
	bool m_canThrow = false;
	bool m_canThrowBullet = true;
	int m_pickedBulletCount = 0;
};




/**
 * @brief The TiledWeaponHand class
 */

class TiledWeaponHand : public TiledWeapon
{
	Q_OBJECT

public:
	TiledWeaponHand(QObject *parent = nullptr);

	bool protect(const WeaponType &) override final { return false; }
	bool canProtect(const WeaponType &) const override final { return false; }
	bool canAttack() const override final { return true; }

protected:
	IsometricBullet *createBullet(const qreal & = 0.) override final { return nullptr; }
	virtual void eventAttack(TiledObject *target) override;

};

#endif // TILEDWEAPON_H
