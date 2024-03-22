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
#include "rpgpickableobject.h"
#include "rpgshortbow.h"
#include "tiledeffect.h"
#include "tiledgamesfx.h"
#include <QQmlEngine>

class RpgGame;


/**
 * @brief The RpgPlayer class
 */

class RpgPlayer : public IsometricPlayer
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(QString character READ character WRITE setCharacter NOTIFY characterChanged FINAL)
	Q_PROPERTY(RpgArmory *armory READ armory CONSTANT FINAL)

public:
	explicit RpgPlayer(QQuickItem *parent = nullptr);
	virtual ~RpgPlayer();

	static RpgPlayer* createPlayer(RpgGame *game, TiledScene *scene, const QString &character);

	static void reloadAvailableCharacters();
	static const QStringList &availableCharacters() { return m_availableCharacters; }

	Q_INVOKABLE void attack(TiledWeapon *weapon);
	Q_INVOKABLE void attackCurrentWeapon() { attack(m_armory->currentWeapon()); }

	Q_INVOKABLE void pick(RpgPickableObject *object);
	Q_INVOKABLE void pickCurrentObject() { pick(qobject_cast<RpgPickableObject*>(currentPickable())); }

	QString character() const;
	void setCharacter(const QString &newCharacter);

	RpgArmory *armory() const;

signals:
	void characterChanged();

protected:
	void load() override final;
	void updateSprite() override final;

	bool protectWeapon(const TiledWeapon::WeaponType &weaponType) override final;
	void attackedByEnemy(IsometricEnemy */*enemy*/, const TiledWeapon::WeaponType &weaponType,
						 const bool &isProtected) override final;
	void onEnemyReached(IsometricEnemy */*enemy*/) override final {}
	void onEnemyLeft(IsometricEnemy */*enemy*/) override final {}
	void onTransportReached(TiledTransport */*transport*/) override final {}
	void onTransportLeft(TiledTransport */*transport*/) override final {}

private:
	void loadDefaultWeapons();
	void onCurrentSpriteChanged();
	void playAliveEffect();
	void playHurtEffect();
	void playHealedEffect();
	void playDeadEffect();
	void playAttackEffect(TiledWeapon *weapon);
	void playWeaponChangedEffect();

private:
	static QStringList m_availableCharacters;
	QString m_character;

	TiledGameSfx m_sfxPain;
	TiledGameSfx m_sfxFootStep;

	std::unique_ptr<RpgArmory> m_armory;

	TiledEffectHealed m_effectHealed;

	friend class RpgGame;
};

#endif // RPGPLAYER_H
