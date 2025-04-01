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
#include <QObject>



/**
 * @brief The TiledWeapon class
 */

class TiledWeapon : public QObject
{
	Q_OBJECT

	Q_PROPERTY(TiledObject *parentObject READ parentObject WRITE setParentObject NOTIFY parentObjectChanged FINAL)
	Q_PROPERTY(int bulletCount READ bulletCount WRITE setBulletCount NOTIFY bulletCountChanged FINAL)
	Q_PROPERTY(int maxBulletCount READ maxBulletCount WRITE setMaxBulletCount NOTIFY maxBulletCountChanged FINAL)
	Q_PROPERTY(bool canShot READ canShot NOTIFY canShotChanged FINAL)
	Q_PROPERTY(bool canHit READ canHit WRITE setCanHit NOTIFY canHitChanged FINAL)
	Q_PROPERTY(bool canCast READ canCast WRITE setCanCast NOTIFY canCastChanged FINAL)
	Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged FINAL)
	Q_PROPERTY(qint64 repeaterIdle READ repeaterIdle WRITE setRepeaterIdle NOTIFY repeaterIdleChanged FINAL)
	Q_PROPERTY(bool excludeFromLayers READ excludeFromLayers WRITE setExcludeFromLayers NOTIFY excludeFromLayersChanged FINAL)
	Q_PROPERTY(qreal bulletDistance READ bulletDistance WRITE setBulletDistance NOTIFY bulletDistanceChanged FINAL)

public:
	explicit TiledWeapon(QObject *parent = nullptr);
	virtual ~TiledWeapon();


	bool canShot() const { return !m_canHit && m_bulletCount != 0; }

	bool canHit() const { return m_canHit && m_bulletCount != 0; }
	void setCanHit(bool newCanHit);

	bool shot(const bool &forced = false);
	bool hit(TiledObject *target, const bool &forced = false);

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

	qreal bulletDistance() const;
	void setBulletDistance(qreal newBulletDistance);

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
	void bulletDistanceChanged();

protected:
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
	qreal m_bulletDistance = 700;

private:
	qint64 m_timerRepeater = -1.;
	bool m_canThrow = false;
	bool m_canThrowBullet = true;
	int m_pickedBulletCount = 0;
};


#endif // TILEDWEAPON_H
