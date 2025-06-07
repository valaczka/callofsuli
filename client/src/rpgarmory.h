/*
 * ---- Call of Suli ----
 *
 * rpgarmory.h
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

#ifndef RPGARMORY_H
#define RPGARMORY_H

#include "rpggamedataiface.h"
#include "tiledweapon.h"
#include "rpgconfig.h"
#include "isometricbullet.h"
#include <QObject>



#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"

class RpgWeapon;
using RpgWeaponList = qolm::QOlm<RpgWeapon>;
Q_DECLARE_METATYPE(RpgWeaponList*)



class RpgGame;




/**
 * @brief The RpgWeapon class
 */

class RpgWeapon : public TiledWeapon
{
	Q_OBJECT

public:
	explicit RpgWeapon(const RpgGameData::Weapon::WeaponType &type, QObject *parent = nullptr);
	explicit RpgWeapon(QObject *parent = nullptr)
		: RpgWeapon(RpgGameData::Weapon::WeaponInvalid, parent) {}

	RpgGameData::Weapon::WeaponType weaponType() const { return m_weaponType; }

	static QString weaponName(const RpgGameData::Weapon::WeaponType &type);
	QString weaponName() const { return weaponName(m_weaponType); }

	static QString weaponNameEn(const RpgGameData::Weapon::WeaponType &type);
	QString weaponNameEn() const { return weaponNameEn(m_weaponType); }

	RpgGameData::Weapon serialize() const;
	bool updateFromSnapshot(const RpgGameData::Weapon &weapon);

	void playAttack(TiledObject *target) {
		eventAttack(target);
	}

protected:
	const RpgGameData::Weapon::WeaponType m_weaponType;


};









/**
 * @brief The RpgBullet class
 */

class RpgBullet : public IsometricBullet,
		public RpgGameDataInterface<RpgGameData::Bullet, RpgGameData::BulletBaseData>,
		public RpgGameData::LifeCycle
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(RpgGameData::Weapon::WeaponType weaponType READ weaponType CONSTANT FINAL)
	Q_PROPERTY(RpgGameData::BulletBaseData::Owner owner READ owner WRITE setOwner NOTIFY ownerChanged FINAL)
	Q_PROPERTY(RpgGameData::BulletBaseData::Targets targets READ targets WRITE setTargets NOTIFY targetsChanged FINAL)

public:
	explicit RpgBullet(const RpgGameData::Weapon::WeaponType &weaponType, TiledGame *game, const cpBodyType &type = CP_BODY_TYPE_DYNAMIC);
	virtual ~RpgBullet();

	virtual ObjectId objectId() const override final { return ifaceObjectId(); }
	virtual void setObjectId(const ObjectId &newObjectId) override final { ifaceSetObjectId(newObjectId); }
	virtual void setObjectId(const int &ownerId, const int &sceneId, const int &id) override final { ifaceSetObjectId(ownerId, sceneId, id); }

	void shot(const RpgGameData::BulletBaseData &baseData);
	virtual void shot(const cpVect &from, const qreal &angle) override;

	RpgGameData::BulletBaseData::Owner owner() const;
	void setOwner(const RpgGameData::BulletBaseData::Owner &newOwner);

	RpgGameData::BulletBaseData::Targets targets() const;
	void setTargets(const RpgGameData::BulletBaseData::Targets &newTargets);

	RpgGameData::Weapon::WeaponType weaponType() const;

	virtual void updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::Bullet> &snapshot) override;
	virtual void updateFromSnapshot(const RpgGameData::Bullet &snap) override;

	const RpgGameData::BaseData &ownerId() const;
	void setOwnerId(const RpgGameData::BaseData &newOwnerId);

	virtual Stage stage() const override { return m_stage; }
	virtual void setStage(const Stage &newStage) override { m_stage = newStage; }

signals:
	void ownerChanged();
	void targetsChanged();

protected:
	RpgGameData::Bullet serializeThis() const override;
	virtual void impactEvent(TiledObjectBody *base, cpShape *shape) override;
	virtual void overshootEvent() override;

	RpgGame *rpgGame() const;

	RpgGameData::BaseData m_impactedObject;

	RpgGameData::LifeCycle::Stage m_stage = StageInvalid;
	bool m_impactRendered = false;


	friend class ActionRpgMultiplayerGame;
};





/**
 * @brief The RpgArmory class
 */

class RpgArmory : public QObject
{
	Q_OBJECT

	Q_PROPERTY(RpgWeaponList *weaponList READ weaponList CONSTANT FINAL)
	Q_PROPERTY(RpgWeapon *currentWeapon READ currentWeapon WRITE setCurrentWeapon NOTIFY currentWeaponChanged FINAL)
	Q_PROPERTY(RpgWeapon *nextWeapon READ nextWeapon WRITE setNextWeapon NOTIFY nextWeaponChanged FINAL)
	Q_PROPERTY(QStringList baseLayers READ baseLayers WRITE setBaseLayers NOTIFY baseLayersChanged FINAL)

public:
	explicit RpgArmory(TiledObject *parentObject, QObject *parent = nullptr);
	virtual ~RpgArmory();

	static const QHash<RpgGameData::Weapon::WeaponType, QString> &weaponHash() { return m_layerInfoHash; }

	Q_INVOKABLE bool changeToNextWeapon();

	void updateLayers();

	int getShieldCount() const;

	RpgWeaponList *weaponList() const;
	RpgWeapon *weaponFind(const RpgGameData::Weapon::WeaponType &type) const;
	RpgWeapon *weaponAdd(const RpgGameData::Weapon::WeaponType &type);

	static std::unique_ptr<RpgWeapon> weaponCreate(const RpgGameData::Weapon::WeaponType &type, QObject *parent = nullptr);

	RpgWeapon *currentWeapon() const;
	void setCurrentWeapon(RpgWeapon *newCurrentWeapon);

	QStringList baseLayers() const;
	void setBaseLayers(const QStringList &newBaseLayers);

	void updateNextWeapon();
	void setCurrentWeaponIf(RpgWeapon *newCurrentWeapon, const RpgGameData::Weapon::WeaponType &currentType);

	RpgWeapon *nextWeapon() const;
	void setNextWeapon(RpgWeapon *newNextWeapon);

	RpgGameData::Armory serialize() const;
	bool updateFromSnapshot(const RpgGameData::Armory &armory);

	TiledObject *parentObject() const;

signals:
	void currentWeaponChanged();
	void baseLayersChanged();
	void nextWeaponChanged();

private:
	RpgWeapon *weaponAdd(RpgWeapon *weapon);
	void weaponRemove(RpgWeapon *weapon);

	RpgWeapon *getNextWeapon() const;

	TiledObject *m_parentObject = nullptr;
	std::unique_ptr<RpgWeaponList> m_weaponList;
	RpgWeapon *m_currentWeapon = nullptr;
	RpgWeapon *m_nextWeapon = nullptr;
	QStringList m_baseLayers = { QStringLiteral("default") };

	static const QHash<RpgGameData::Weapon::WeaponType, QString> m_layerInfoHash;

};







/**
 * @brief The RpgWeaponHand class
 */

class RpgWeaponHand : public RpgWeapon
{
	Q_OBJECT

public:
	RpgWeaponHand(QObject *parent = nullptr);

protected:
	virtual void eventAttack(TiledObject *target) override;

};
#endif // RPGARMORY_H
