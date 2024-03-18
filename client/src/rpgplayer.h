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
#include "rpgshortbow.h"
#include "tiledgamesfx.h"
#include <QQmlEngine>

class TiledRpgGame;


/**
 * @brief The RpgPlayer class
 */

class RpgPlayer : public IsometricPlayer
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(QString character READ character WRITE setCharacter NOTIFY characterChanged FINAL)
	Q_PROPERTY(TiledWeaponList* weaponList READ weaponList CONSTANT FINAL)
	Q_PROPERTY(TiledWeapon *currentWeapon READ currentWeapon WRITE setCurrentWeapon NOTIFY currentWeaponChanged FINAL)

public:
	explicit RpgPlayer(QQuickItem *parent = nullptr);
	virtual ~RpgPlayer();

	static RpgPlayer* createPlayer(TiledRpgGame *game, TiledScene *scene, const QString &character);

	static void reloadAvailableCharacters();
	static const QStringList &availableCharacters() { return m_availableCharacters; }

	Q_INVOKABLE void attack(TiledWeapon *weapon);
	Q_INVOKABLE void attackCurrentWeapon() { attack(m_currentWeapon); }

	Q_INVOKABLE void nextWeapon();

	QString character() const;
	void setCharacter(const QString &newCharacter);

	TiledWeaponList *weaponList() const;

	TiledWeapon *weaponAdd(TiledWeapon *weapon);
	void weaponRemove(TiledWeapon *weapon);

	TiledWeapon *currentWeapon() const;
	void setCurrentWeapon(TiledWeapon *newCurrentWeapon);

signals:
	void characterChanged();
	void currentWeaponChanged();

protected:
	void load() override final;
	void updateSprite() override final;

	void attackedByEnemy(IsometricEnemy */*enemy*/, const TiledWeapon::WeaponType &weaponType) override final;
	void onEnemyReached(IsometricEnemy */*enemy*/) override final {}
	void onEnemyLeft(IsometricEnemy */*enemy*/) override final {}
	void onTransportReached(TiledTransport */*transport*/) override final {}
	void onTransportLeft(TiledTransport */*transport*/) override final {}

private:
	void loadDefaultWeapons();
	void updateLayers();
	void onCurrentSpriteChanged();
	void playAliveEffect();
	void playHurtEffect();
	void playHealedEffect();
	void playDeadEffect();
	void playAttackEffect(TiledWeapon *weapon);

private:
	static QStringList m_availableCharacters;
	QString m_character;

	TiledGameSfx m_sfxPain;
	TiledGameSfx m_sfxFootStep;

	std::unique_ptr<TiledWeaponList> m_weaponList;
	TiledWeapon *m_currentWeapon = nullptr;

	friend class TiledRpgGame;
};

#endif // RPGPLAYER_H
