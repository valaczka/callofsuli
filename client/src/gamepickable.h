/*
 * ---- Call of Suli ----
 *
 * gamepickable.h
 *
 * Created on: 2022. 12. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GamePickable
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

#ifndef GAMEPICKABLE_H
#define GAMEPICKABLE_H

#include "gameobject.h"
#include <QObject>

class GamePlayer;

/**
 * @brief The GamePickable class
 */

class GamePickable : public GameObject
{
	Q_OBJECT

	Q_PROPERTY(QPointF bottomPoint READ bottomPoint WRITE setBottomPoint NOTIFY bottomPointChanged)
	Q_PROPERTY(GamePickableData pickableData READ pickableData WRITE setPickableData NOTIFY pickableDataChanged)

	Q_PROPERTY(QString id READ id NOTIFY idChanged)
	Q_PROPERTY(QString name READ name NOTIFY nameChanged)
	Q_PROPERTY(PickableType type READ type NOTIFY typeChanged)
	Q_PROPERTY(QString image READ image NOTIFY imageChanged)
	Q_PROPERTY(PickableFormat format READ format NOTIFY formatChanged)

public:
	GamePickable(QQuickItem *parent = nullptr);
	~GamePickable();

	/**
	 * @brief The PickableType enum
	 */

	enum PickableType {
		PickableInvalid = 0,
		PickableHealth,
		PickableTime30,
		PickableTime60,
		PickableShield1,
		PickableShield2,
		PickableShield3,
		PickableShield5,
		PickableWater,
		PickablePliers,
		PickableCamouflage,
		PickableTeleporter

	};

	Q_ENUM(PickableType)



	/**
	 * @brief The PickableFormat enum
	 */

	enum PickableFormat {
		FormatPixmap,
		FormatAnimated
	};

	Q_ENUM(PickableFormat)


	/**
	 * @brief The GamePickableData class
	 */

	struct GamePickableData
	{
		QString id;
		QString name;
		PickableType type = PickableInvalid;
		QString image;
		PickableFormat format = FormatPixmap;

		GamePickableData() {}

		GamePickableData(const QString &i, const QString &n, const PickableType &t, const QString &iG, const PickableFormat &f)
			: id(i), name(n), type(t), image(iG), format(f)
		{}
	};

	static const QVector<GamePickableData> &pickableDataTypes();
	static QHash<QString, GamePickableData> pickableDataHash();


	const QString &id() const { return m_pickableData.id; }
	const QString &name() const { return m_pickableData.name; }
	GamePickable::PickableType type() const { return m_pickableData.type; }
	const QString &image() const { return m_pickableData.image; }
	PickableFormat format() const { return m_pickableData.format; }

	const GamePickableData &pickableData() const;
	void setPickableData(const GamePickableData &newData);

	QPointF bottomPoint() const;
	void setBottomPoint(QPointF newBottomPoint);

	void pick(ActionGame *game);

signals:
	void pickFinished();
	void idChanged();
	void nameChanged();
	void typeChanged();
	void imageChanged();
	void formatChanged();

	void pickableDataChanged();
	void bottomPointChanged();

private:
	static const QVector<GamePickableData> m_pickableDataTypes;
	GamePickableData m_pickableData = GamePickableData();
	QPointF m_bottomPoint;
};

Q_DECLARE_METATYPE(GamePickable::GamePickableData)

#endif // GAMEPICKABLE_H
