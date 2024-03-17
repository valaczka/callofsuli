/*
 * ---- Call of Suli ----
 *
 * isometricwerebear.h
 *
 * Created on: 2024. 03. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * IsometricWerebear
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

#ifndef ISOMETRICWEREBEAR_H
#define ISOMETRICWEREBEAR_H

#include "isometricenemy.h"
#include "qtimer.h"
#include <QQmlEngine>

class IsometricWerebear : public IsometricEnemy
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(WerebearType werebearType READ werebearType NOTIFY werebearTypeChanged FINAL)

public:
	explicit IsometricWerebear(QQuickItem *parent = nullptr);
	virtual ~IsometricWerebear();

	enum WerebearType {
		WerebearDefault = 0,
		WerebearBrownArmor,
		WerebearBrownBare,
		WerebearBrownShirt,
		WerebearWhiteArmor,
		WerebearWhiteBare,
		WerebearWhiteShirt,
	};

	Q_ENUM(WerebearType);

	WerebearType werebearType() const;
	void setWerebearType(const WerebearType &newWerebearType);
	void setWerebearType(const QString &text);

signals:
	void werebearTypeChanged();

protected:
	//bool enemyWorldStep() override final;

	void load() override final;
	void onPlayerReached(IsometricPlayer */*player*/) override final {}
	void onPlayerLeft(IsometricPlayer */*player*/) override final {}

private:
	void onFootSoundTimerTimeout();

	WerebearType m_werebearType = WerebearDefault;
	QTimer m_footSoundTimer;
	int m_pNum = 7;
};

#endif // ISOMETRICWEREBEAR_H
