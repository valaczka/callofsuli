/*
 * ---- Call of Suli ----
 *
 * isometricbullet.h
 *
 * Created on: 2024. 03. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * IsometricBullet
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

#ifndef ISOMETRICBULLET_H
#define ISOMETRICBULLET_H

#include "isometricobject.h"
#include <QQmlEngine>


class IsometricBulletPrivate;
class TiledWeapon;

#if QT_VERSION >= 0x060000

#ifndef OPAQUE_PTR_TiledWeapon
#define OPAQUE_PTR_TiledWeapon
Q_DECLARE_OPAQUE_POINTER(TiledWeapon*)
#endif

#endif


/**
 * @brief The IsometricBullet class
 */

class IsometricBullet : public IsometricObjectCircle
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(Targets targets READ targets WRITE setTargets NOTIFY targetsChanged FINAL)
	Q_PROPERTY(bool impacted READ impacted WRITE setImpacted NOTIFY impactedChanged FINAL)
	Q_PROPERTY(qreal maxDistance READ maxDistance WRITE setMaxDistance NOTIFY maxDistanceChanged FINAL)
	Q_PROPERTY(qint64 bulletId READ bulletId WRITE setBulletId NOTIFY bulletIdChanged FINAL)
	Q_PROPERTY(bool autoDelete READ autoDelete WRITE setAutoDelete NOTIFY autoDeleteChanged FINAL)

public:
	explicit IsometricBullet(QQuickItem *parent = nullptr);
	virtual ~IsometricBullet();

	enum Target {
		TargetNone = 0,
		TargetEnemy = 1,
		TargetPlayer = 1 << 1,

		TargetAll = TargetEnemy|TargetPlayer
	};

	Q_ENUM(Target)
	Q_DECLARE_FLAGS(Targets, Target)
	Q_FLAG(Targets)


	//static IsometricBullet* createBullet(TiledGame *game, TiledScene *scene);

	void initialize();

	virtual void shot(const QPointF &from, const Direction &direction);
	virtual void shot(const QPointF &from, const qreal &angle);

	void shot(const Targets &targets, const QPointF &from, const Direction &direction);
	void shot(const Targets &targets, const QPointF &from, const qreal &angle);

	void worldStep(const qreal &factor) override;

	bool impacted() const;
	void setImpacted(bool newImpacted);

	qreal maxDistance() const;
	void setMaxDistance(qreal newMaxDistance);

	qint64 bulletId() const;
	void setBulletId(qint64 newBulletId);

	bool autoDelete() const;
	void setAutoDelete(bool newAutoDelete);

	Targets targets() const;
	void setTargets(const Targets &newTargets);

	void setFromWeapon(TiledWeapon *newFromWeapon);

signals:
	void impactedChanged();
	void maxDistanceChanged();
	void bulletIdChanged();
	void autoDeleteChanged();
	void targetsChanged();

protected:
	virtual void load() = 0;
	virtual void impactEvent(TiledObjectBase *base);
	virtual void groundEvent(TiledObjectBase *base) { Q_UNUSED(base); }
	virtual void overshootEvent() {}

	void doAutoDelete();

private:
	void fixtureBeginContact(Box2DFixture *other);
	//void fixtureEndContact(Box2DFixture *other);

protected:
	QPointF m_startPoint;
	Direction m_direction = Invalid;
	qreal m_angle = 0.;
	qreal m_speed = 20.;
	qreal m_maxDistance = 700.;
	qint64 m_bulletId = 0;
	Targets m_targets = TargetNone;

	IsometricBulletPrivate *d;

private:
	bool m_impacted = false;
	bool m_autoDelete = true;
};


Q_DECLARE_OPERATORS_FOR_FLAGS(IsometricBullet::Targets);

#endif // ISOMETRICBULLET_H
