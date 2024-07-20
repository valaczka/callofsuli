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
#include "rpgconfig.h"
#include "rpgpickableobject.h"
#include "rpgshortbow.h"
#include "tiledeffect.h"
#include "tiledgamesfx.h"
#include <QQmlEngine>

class RpgGame;


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
		CastFireball		= 2
	};

	Q_ENUM(CastType);

	RpgPlayerCharacterConfig() : QSerializer()
	  , cast(CastInvalid)
	  , mpLoss(1)					// MP loss / 0.1 mp
	  , mpMax(100)
	  , mpStart(10)
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
	QS_FIELD(int, mpLoss)
	QS_FIELD(int, mpMax)
	QS_FIELD(int, mpStart)
};



/**
 * @brief The RpgPlayer class
 */

class RpgPlayer : public IsometricPlayer
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(RpgPlayerCharacterConfig config READ config WRITE setConfig NOTIFY configChanged FINAL)
	Q_PROPERTY(RpgArmory *armory READ armory CONSTANT FINAL)
	Q_PROPERTY(int shieldCount READ shieldCount WRITE setShieldCount NOTIFY shieldCountChanged FINAL)
	Q_PROPERTY(RpgInventoryList *inventory READ inventory CONSTANT FINAL)
	Q_PROPERTY(int mp READ mp WRITE setMp NOTIFY mpChanged FINAL)
	Q_PROPERTY(int maxMp READ maxMp WRITE setMaxMp NOTIFY maxMpChanged FINAL)

public:
	explicit RpgPlayer(QQuickItem *parent = nullptr);
	virtual ~RpgPlayer();

	static RpgPlayer* createPlayer(RpgGame *game, TiledScene *scene, const RpgPlayerCharacterConfig &config);

	Q_INVOKABLE void attack(TiledWeapon *weapon);
	Q_INVOKABLE void attackCurrentWeapon() { attack(m_armory->currentWeapon()); }

	Q_INVOKABLE void cast();

	void attackToPoint(const qreal &x, const qreal &y);
	void attackToPoint(const QPointF &point) { attackToPoint(point.x(), point.y()); }

	Q_INVOKABLE void pick(RpgPickableObject *object);

	Q_INVOKABLE void useContainer(TiledContainer *container);
	Q_INVOKABLE void useCurrentContainer() { useContainer(currentContainer()); }
	Q_INVOKABLE void useCurrentObjects();

	RpgArmory *armory() const;

	QPointF currentSceneStartPosition() const;
	void setCurrentSceneStartPosition(QPointF newCurrentSceneStartPosition);

	int shieldCount() const;
	void setShieldCount(int newShieldCount);

	const RpgPlayerCharacterConfig &config() const;
	void setConfig(const RpgPlayerCharacterConfig &newConfig);

	void inventoryAdd(RpgPickableObject *object);
	void inventoryAdd(const RpgPickableObject::PickableType &type, const QString &name = {});
	void inventoryRemove(const RpgPickableObject::PickableType &type);
	void inventoryRemove(const RpgPickableObject::PickableType &type, const QString &name);
	bool inventoryContains(const RpgPickableObject::PickableType &type) const;
	bool inventoryContains(const RpgPickableObject::PickableType &type, const QString &name) const;

	RpgInventoryList *inventory() const;

	int mp() const;
	void setMp(int newMp);

	int maxMp() const;
	void setMaxMp(int newMaxMp);

	bool isDiscoverable() const override final;

	bool castTimerActive() const { return m_castTimer.isActive(); }

signals:
	void attackDone();
	void characterChanged();
	void shieldCountChanged();
	void configChanged();
	void mpChanged();
	void maxMpChanged();

protected:
	void load() override final;
	void updateSprite() override final;

	bool protectWeapon(const TiledWeapon::WeaponType &weaponType) override final;
	void attackedByEnemy(IsometricEnemy */*enemy*/, const TiledWeapon::WeaponType &weaponType,
						 const bool &isProtected) override final;
	void onPickableReached(TiledObject *object) override final;
	void onPickableLeft(TiledObject */*object*/) override final {};
	void onEnemyReached(IsometricEnemy */*enemy*/) override final {}
	void onEnemyLeft(IsometricEnemy */*enemy*/) override final {}
	void onTransportReached(TiledTransport */*transport*/) override final {}
	void onTransportLeft(TiledTransport */*transport*/) override final {}

	void atDestinationPointEvent() override final;

	void onCurrentTransportChanged();

private:
	void loadDefaultWeapons();
	void loadSfx();
	void onCurrentSpriteChanged();
	void playAliveEffect();
	void playHurtEffect();
	void playHealedEffect();
	void playDeadEffect();
	void playAttackEffect(TiledWeapon *weapon);
	void playWeaponChangedEffect();
	void playShieldEffect();
	void messageEmptyBullet(const TiledWeapon::WeaponType &weaponType);
	void onCastTimerTimeout();

private:
	RpgPlayerCharacterConfig m_config;

	TiledGameSfx m_sfxPain;
	TiledGameSfx m_sfxFootStep;
	TiledGameSfx m_sfxAccept;
	TiledGameSfx m_sfxDecline;

	std::unique_ptr<RpgArmory> m_armory;
	std::unique_ptr<RpgInventoryList> m_inventory;

	TiledEffectHealed m_effectHealed;
	TiledEffectShield m_effectShield;
	TiledEffectSmoke m_effectSmoke;

	QPointF m_currentSceneStartPosition;
	bool m_pickAtDestination = false;
	int m_shieldCount = 0;
	int m_mp = 0;
	int m_maxMp = 0;

	QTimer m_castTimer;
	QDeadlineTimer m_timerRepeater;

	friend class RpgGame;
	friend class ActionRpgGame;
};

#endif // RPGPLAYER_H
