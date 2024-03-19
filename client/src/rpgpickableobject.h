/*
 * ---- Call of Suli ----
 *
 * rpgpickableobject.h
 *
 * Created on: 2024. 03. 19.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgPickableObject
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

#ifndef RPGPICKABLEOBJECT_H
#define RPGPICKABLEOBJECT_H

#include "tiledpickableiface.h"
#include "isometricobject.h"
#include <QQmlEngine>


class RpgPlayer;

/**
 * @brief The RpgPickableObject class
 */

class RpgPickableObject : public IsometricObjectCircle, public TiledPickableIface
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(PickableType pickableType READ pickableType CONSTANT FINAL)
	Q_PROPERTY(bool isActive READ isActive WRITE setIsActive NOTIFY isActiveChanged FINAL)

public:
	enum PickableType {
		PickableInvalid = 0,
		PickableHp,
		PickableArrow,
		PickableLongsword,
		PickableShield
	};

	Q_ENUM(PickableType);

	RpgPickableObject(const PickableType &type, QQuickItem *parent = nullptr);
	virtual ~RpgPickableObject() {}

	void initialize();

	static PickableType typeFromString(const QString &type) { return m_typeHash.value(type, PickableInvalid); }

	PickableType pickableType() const;

	virtual void playerPick(RpgPlayer *player) = 0;
	virtual void playerThrow(RpgPlayer *player) = 0;

signals:
	void isActiveChanged() override final;

protected:
	virtual void onActivated() override;
	virtual void onDeactivated() override;

	virtual void load() = 0;

private:
	const PickableType m_pickableType;

	static const QHash<QString, PickableType> m_typeHash;
};

#endif // RPGPICKABLEOBJECT_H
