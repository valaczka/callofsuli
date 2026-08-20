/*
 * ---- Call of Suli ----
 *
 * rpgchanger.h
 *
 * Created on: 2026. 07. 12.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgChanger
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

#ifndef RPGCHANGER_H
#define RPGCHANGER_H

#include "rpggame.h"
#include "rpgplayer.h"
#include <QQuickItem>


/**
 * @brief The RpgChanger class
 */

class RpgChanger : public QQuickItem
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged FINAL)
	Q_PROPERTY(RpgGame *game READ game WRITE setGame NOTIFY gameChanged FINAL)
	Q_PROPERTY(RpgPlayer *player READ player WRITE setPlayer NOTIFY playerChanged FINAL)
	Q_PROPERTY(QVariantList availableDefenders READ availableDefenders WRITE setAvailableDefenders NOTIFY availableDefendersChanged FINAL)
	Q_PROPERTY(QVariantList availableUtilites READ availableUtilites WRITE setAvailableUtilites NOTIFY availableUtilitesChanged FINAL)
	Q_PROPERTY(bool replaceEnabled READ replaceEnabled WRITE setReplaceEnabled NOTIFY replaceEnabledChanged FINAL)
	Q_PROPERTY(bool isBlocked READ isBlocked WRITE setIsBlocked NOTIFY isBlockedChanged FINAL)

public:
	RpgChanger(QQuickItem *parent = nullptr);

	Q_INVOKABLE void open() { setActive(true); }
	Q_INVOKABLE void close() { setActive(false); }

	Q_INVOKABLE void useWeapon(const bool &force = false);
	Q_INVOKABLE void useDefender();
	Q_INVOKABLE void useUtility();

	Q_INVOKABLE void use(const QString &mode);

	Q_INVOKABLE int currentDefender();
	Q_INVOKABLE int currentUtility();

	Q_INVOKABLE void setDefender(const int &key);
	Q_INVOKABLE void setUtility(const int &key);

	Q_INVOKABLE void set(const QString &mode, const int &key);

	Q_INVOKABLE static QVariantMap availableWeapon();
	Q_INVOKABLE void checkBlocked();

	bool active() const;
	void setActive(bool newActive);

	RpgGame *game() const;
	void setGame(RpgGame *newGame);

	RpgPlayer *player() const;
	void setPlayer(RpgPlayer *newPlayer);

	QVariantList availableDefenders() const;
	void setAvailableDefenders(const QVariantList &newAvailableDefenders);

	QVariantList availableUtilites() const;
	void setAvailableUtilites(const QVariantList &newAvailableUtilites);

	bool replaceEnabled() const;
	void setReplaceEnabled(bool newReplaceEnabled);

	static const QHash<RpgStream::BaseDefenderObject::Type, QVariantMap> &dataDefenders();
	static const QHash<RpgStream::PlayerConfig::Utility, QVariantMap> &dataUtilities();

	bool isBlocked() const;
	void setIsBlocked(bool newIsBlocked);

signals:
	void playerReloaded();

	void activeChanged();
	void gameChanged();
	void playerChanged();
	void availableDefendersChanged();
	void availableUtilitesChanged();
	void replaceEnabledChanged();
	void isBlockedChanged();

private:
	void connectPlayer();
	void updatePlayerDefender();
	void reloadDefenders();
	void reloadUtilities();

private:
	bool m_active = false;
	RpgGame *m_game = nullptr;
	RpgPlayer *m_player = nullptr;
	QVariantList m_availableDefenders;
	QVariantList m_availableUtilites;

	bool m_replaceEnabled = true;
	bool m_isBlocked = false;

	static const QHash<RpgStream::BaseDefenderObject::Type, QVariantMap> m_dataDefenders;
	static const QHash<RpgStream::PlayerConfig::Utility, QVariantMap> m_dataUtilities;

};

#endif // RPGCHANGER_H
