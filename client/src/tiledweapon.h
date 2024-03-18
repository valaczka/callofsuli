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
	Q_PROPERTY(bool canShot READ canShot NOTIFY canShotChanged FINAL)
	Q_PROPERTY(bool canHit READ canHit WRITE setCanHit NOTIFY canHitChanged FINAL)
	Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged FINAL)

public:
	enum WeaponType {
		WeaponInvalid = 0,
		WeaponHand,
		WeaponSword,
		WeaponShortbow,
		WeaponShield
	};

	Q_ENUM (WeaponType);

	explicit TiledWeapon(const WeaponType &type, QObject *parent = nullptr);
	virtual ~TiledWeapon() {}

	WeaponType weaponType() const;

	bool shot(const IsometricBullet::Targets &targets, const QPointF &from, const TiledObject::Direction &direction);
	bool shot(const IsometricBullet::Targets &targets, const QPointF &from, const qreal &angle);

	bool canShot() const { return m_bulletCount != 0; }

	virtual bool protect(const WeaponType &weapon) = 0;
	virtual bool canProtect(const WeaponType &weapon) const = 0;

	void hit(TiledObject *target);

	bool canHit() const;
	void setCanHit(bool newCanHit);

	TiledObject *parentObject() const;
	void setParentObject(TiledObject *newParentObject);

	int bulletCount() const;
	void setBulletCount(int newBulletCount);

	QString icon() const;
	void setIcon(const QString &newIcon);

signals:
	void parentObjectChanged();
	void bulletCountChanged();
	void canShotChanged();
	void canHitChanged();
	void iconChanged();

protected:
	virtual IsometricBullet *createBullet() = 0;
	virtual void eventAttack() {}

	QPointer<TiledObject> m_parentObject;
	int m_bulletCount = 0;
	bool m_canHit = false;
	QString m_icon;

private:
	const WeaponType m_weaponType;

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

protected:
	IsometricBullet *createBullet() override final { return nullptr; }

};

#endif // TILEDWEAPON_H
