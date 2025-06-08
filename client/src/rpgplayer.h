/*
 * ---- Call of Suli ----
 *
 * rpgplayer.h
 *
 * Created on: 2024. 03. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgPlayer
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

#ifndef RPGPLAYER_H
#define RPGPLAYER_H

#include "isometricplayer.h"
#include "rpgarmory.h"
#include "rpggamedataiface.h"
#include "rpgshortbow.h"
#include "tiledeffect.h"
#include "tiledgamesfx.h"
#include <QQmlEngine>

class RpgGame;
class RpgActiveControlObject;

#ifndef OPAQUE_PTR_RpgActiveControlObject
#define OPAQUE_PTR_RpgActiveControlObject
Q_DECLARE_OPAQUE_POINTER(RpgActiveControlObject*)
#endif



/**
 * @brief The RpgPlayerConfig class
 */

class RpgPlayerCharacterConfig : public QSerializer
{
	Q_GADGET

public:
	enum CastType {
		CastInvalid			= 0,
		CastInvisible		= 1,
		CastFireball		= 2,
		CastFireballTriple	= 3,
		CastLightning		= 4,
		CastFireFog			= 5,
		CastArrowQuintuple	= 6,
		CastProtect			= 7,
	};

	Q_ENUM(CastType);

	RpgPlayerCharacterConfig() : QSerializer()
	  , cast(CastInvalid)
	  , mpMax(100)
	  , mpStart(10)
	  , inability(-1)
	{}

	void updateSfxPath(const QString &prefix);

	QString prefixPath;

	QS_SERIALIZABLE

	QS_FIELD(QString, name)
	QS_FIELD(QString, image)
	QS_FIELD(QString, base)			// Based on character (e.g. sfx, inventory,...)
	QS_FIELD(QString, shield)		// Character specific shield

	// Sfx sounds

	QS_FIELD(QString, sfxDead)
	QS_COLLECTION(QList, QString, sfxPain)
	QS_COLLECTION(QList, QString, sfxFootStep)
	QS_COLLECTION(QList, QString, sfxAccept)
	QS_COLLECTION(QList, QString, sfxDecline)

	// Inventory

	QS_COLLECTION(QList, QString, inventory)
	QS_COLLECTION(QList, QString, inventoryOnce)

	// Cast

	QS_FIELD(CastType, cast)
	QS_FIELD(int, mpMax)
	QS_FIELD(int, mpStart)

	// Disabled weapons

	QS_COLLECTION(QList, QString, disabledWeapons)
	QS_FIELD(int, inability)		// Base inability time
};





class RpgEnemy;
class RpgPlayerPrivate;


/**
 * @brief The RpgPlayer class
 */

class RpgPlayer : public IsometricPlayer, public RpgGameDataInterface<RpgGameData::Player, RpgGameData::PlayerBaseData>
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(RpgPlayerCharacterConfig config READ config WRITE setConfig NOTIFY configChanged FINAL)
	Q_PROPERTY(RpgArmory *armory READ armory CONSTANT FINAL)
	Q_PROPERTY(int shieldCount READ shieldCount WRITE setShieldCount NOTIFY shieldCountChanged FINAL)
	Q_PROPERTY(RpgActiveControlObject *currentControl READ currentControl WRITE setCurrentControl NOTIFY currentControlChanged FINAL)
	Q_PROPERTY(int mp READ mp WRITE setMp NOTIFY mpChanged FINAL)
	Q_PROPERTY(int maxMp READ maxMp WRITE setMaxMp NOTIFY maxMpChanged FINAL)
	Q_PROPERTY(int collection READ collection WRITE setCollection NOTIFY collectionChanged FINAL)
	Q_PROPERTY(int collectionRq READ collectionRq WRITE setCollectionRq NOTIFY collectionRqChanged FINAL)

public:
	explicit RpgPlayer(RpgGame *game, const qreal &radius = 10., const cpBodyType &type = CP_BODY_TYPE_DYNAMIC);
	virtual ~RpgPlayer();

	virtual ObjectId objectId() const override final { return ifaceObjectId(); }
	virtual void setObjectId(const ObjectId &newObjectId) override final { ifaceSetObjectId(newObjectId); }
	virtual void setObjectId(const int &ownerId, const int &sceneId, const int &id) override final { ifaceSetObjectId(ownerId, sceneId, id); }

	Q_INVOKABLE void attack(RpgWeapon *weapon);
	Q_INVOKABLE void attackCurrentWeapon() { attack(m_armory->currentWeapon()); }

	Q_INVOKABLE void cast();

	void attackToPoint(const qreal &x, const qreal &y);
	void attackToPoint(const QPointF &point) { attackToPoint(point.x(), point.y()); }

	Q_INVOKABLE void useCurrentControl();

	RpgArmory *armory() const;

	QPointF currentSceneStartPosition() const;
	void setCurrentSceneStartPosition(QPointF newCurrentSceneStartPosition);

	int shieldCount() const;
	void setShieldCount(int newShieldCount);

	const RpgPlayerCharacterConfig &config() const;
	void setConfig(const RpgPlayerCharacterConfig &newConfig);

	int mp() const;
	void setMp(int newMp);

	int maxMp() const;
	void setMaxMp(int newMaxMp);

	bool isDiscoverable() const override final;

	bool castTimerActive() const { return m_castTimer.isActive(); }

	virtual void worldStep() override;

	virtual void onShapeContactBegin(cpShape *self, cpShape *other) override;
	virtual void onShapeContactEnd(cpShape *self, cpShape *other) override;

	void updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::Player> &snapshot) override;
	void updateFromSnapshot(const RpgGameData::Player &snap) override;
	virtual bool isLastSnapshotValid(const RpgGameData::Player &snap, const RpgGameData::Player &lastSnap) const override;

	void attackedByEnemy(RpgEnemy *enemy, const RpgGameData::Weapon::WeaponType &weaponType, const bool &isProtected,
						 const bool &immediately = false);

	int nextObjectId() { return ++m_lastObjectId; }

	RpgActiveControlObject *currentControl() const;
	void setCurrentControl(RpgActiveControlObject *newCurrentControl);

	int collection() const;
	void setCollection(int newCollection);

	int collectionRq() const;
	void setCollectionRq(int newCollectionRq);

signals:
	void attackDone();
	void characterChanged();
	void shieldCountChanged();
	void configChanged();
	void mpChanged();
	void maxMpChanged();
	void currentControlChanged();
	void collectionChanged();
	void collectionRqChanged();

protected:
	void load() override final;
	void updateSprite() override final;

	RpgGameData::Player serializeThis() const override;

	/*virtual bool protectWeapon(const RpgGameData::Weapon::WeaponType &weaponType) = 0;
	virtual void attackedByEnemy(IsometricEnemy *enemy, const TiledWeapon::WeaponType &weaponType, const bool &isProtected) = 0;
	bool protectWeapon(TiledWeaponList *weaponList, const TiledWeapon::WeaponType &weaponType);
	bool protectWeapon(const RpgWeapon::WeaponType &weaponType) override final;
	void attackedByEnemy(IsometricEnemy *enemy, const RpgWeapon::WeaponType &weaponType,
						 const bool &isProtected) override final;*/

	void onEnemyReached(IsometricEnemy *enemy) override final;
	void onEnemyLeft(IsometricEnemy */*enemy*/) override final {}

	void atDestinationPointEvent() override final;

private:
	void updateConfig();
	void loadDefaultWeapons();
	void loadSfx();
	void onCurrentSpriteChanged();
	void playAliveEffect();
	void playHurtEffect();
	void playHealedEffect();
	void playDeadEffect();
	void playAttackEffect(const RpgGameData::Weapon::WeaponType &weaponType);
	void playAttackEffect(RpgWeapon *weapon);
	void playWeaponChangedEffect();
	void playShieldEffect();
	void messageEmptyBullet(const RpgGameData::Weapon::WeaponType &weaponType);
	void onCastTimerTimeout();
	void attackReachedEnemies(const RpgGameData::Weapon::WeaponType &weaponType);

private:
	RpgPlayerPrivate *const d;

	RpgPlayerCharacterConfig m_config;

	TiledGameSfx m_sfxPain;
	TiledGameSfx m_sfxFootStep;
	TiledGameSfx m_sfxAccept;
	TiledGameSfx m_sfxDecline;

	std::unique_ptr<RpgArmory> m_armory;

	TiledEffectHealed m_effectHealed;
	TiledEffectShield m_effectShield;
	TiledEffectRing m_effectRing;

	QPointF m_currentSceneStartPosition;
	bool m_pickAtDestination = false;
	int m_shieldCount = 0;
	int m_mp = 0;
	int m_maxMp = 0;
	int m_collection = 0;
	int m_collectionRq = 0;

	int m_lastObjectId = 0;

	QTimer m_castTimer;
	qint64 m_timerRepeater = -1;

	QHash<RpgGameData::Player::PlayerState, qint64> m_stateLastRenderedTicks;

	friend class RpgGame;
	friend class ActionRpgGame;
	friend class ActionRpgMultiplayerGame;
};

#endif // RPGPLAYER_H
