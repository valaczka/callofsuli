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

public:
	explicit RpgPlayer(QQuickItem *parent = nullptr);
	virtual ~RpgPlayer();

	static RpgPlayer* createPlayer(TiledRpgGame *game, TiledScene *scene, const QString &character);

	static void reloadAvailableCharacters();
	static const QStringList &availableCharacters() { return m_availableCharacters; }

	QString character() const;
	void setCharacter(const QString &newCharacter);

signals:
	void characterChanged();

protected:
	void load() override final;
	void updateSprite() override final;

	void onAlive() override final;
	void onDead() override final;

	void attackedByEnemy(IsometricEnemy */*enemy*/) override final;
	void onEnemyReached(IsometricEnemy */*enemy*/) override final {}
	void onEnemyLeft(IsometricEnemy */*enemy*/) override final {}
	void onTransportReached(TiledTransport */*transport*/) override final {}
	void onTransportLeft(TiledTransport */*transport*/) override final {}

private:
	void playHealEffect();

private:
	static QStringList m_availableCharacters;
	QString m_character;

	int m_pNum = 1;

	friend class TiledRpgGame;
};

#endif // RPGPLAYER_H
