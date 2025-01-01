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

#include "tiledeffect.h"
#include "tiledpickableiface.h"
#include "isometricobject.h"
#include <QQmlEngine>

class RpgPlayer;


/**
 * @brief The RpgPickableObject class
 */

class RpgPickableObject : public IsometricObject, public TiledPickableIface
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(PickableType pickableType READ pickableType CONSTANT FINAL)
	Q_PROPERTY(bool isActive READ isActive WRITE setIsActive NOTIFY isActiveChanged FINAL)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)

public:
	enum PickableType {
		PickableInvalid = 0,
		PickableHp,
		PickableShortbow,
		PickableLongbow,
		/*PickableArrow [[deprecated]],
		PickableFireball [[deprecated]],
		PickableLightning [[deprecated]],*/
		PickableLongsword,
		PickableDagger,
		PickableShield,
		PickableTime,
		PickableMp,
		PickableCoin,
		PickableKey,
	};

	Q_ENUM(PickableType);

	RpgPickableObject(const PickableType &type, TiledScene *scene);
	virtual ~RpgPickableObject() {}

	void initialize();

	static PickableType typeFromString(const QString &type) { return m_typeHash.value(type, PickableInvalid); }
	static const QHash<QString, PickableType> &typeHash() { return m_typeHash; }

	static QString pickableName(const PickableType &type);
	static QString pickableNameEn(const PickableType &type);

	PickableType pickableType() const;

	virtual bool playerPick(RpgPlayer *player) = 0;

	QString name() const;
	void setName(const QString &newName);

	virtual void onShapeContactBegin(b2::ShapeRef self, b2::ShapeRef other) override;
	virtual void onShapeContactEnd(b2::ShapeRef self, b2::ShapeRef other) override;

signals:
	void isActiveChanged() override final;
	void nameChanged();

protected:
	virtual void onActivated() override;
	virtual void onDeactivated() override;


	virtual void load() = 0;

	void loadDefault(const QString &directory);

	std::unique_ptr<TiledEffect> m_activateEffect;
	std::unique_ptr<TiledEffect> m_deactivateEffect;

private:
	/*void fixtureBeginContact(Box2DFixture *other);
	void fixtureEndContact(Box2DFixture *other);*/

	const PickableType m_pickableType;
	QString m_name;

	static const QHash<QString, PickableType> m_typeHash;
};





/**
 * @brief The RpgInventory class
 */

class RpgInventory : public QObject
{
	Q_OBJECT

	Q_PROPERTY(RpgPickableObject::PickableType pickableType READ pickableType CONSTANT FINAL)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
	Q_PROPERTY(QString icon READ icon NOTIFY iconChanged FINAL)
	Q_PROPERTY(QColor iconColor READ iconColor NOTIFY iconColorChanged FINAL)

public:
	RpgInventory(const RpgPickableObject::PickableType &type, const QString &name, QObject *parent = nullptr);
	RpgInventory(const RpgPickableObject::PickableType &type, QObject *parent = nullptr) :
		RpgInventory(type, QStringLiteral(""), parent) {}
	virtual ~RpgInventory() {}

	const RpgPickableObject::PickableType &pickableType() const { return m_pickableType; }

	QString name() const;
	void setName(const QString &newName);

	QString icon() const;
	QColor iconColor() const;

signals:
	void nameChanged();
	void iconChanged();
	void iconColorChanged();

private:
	const RpgPickableObject::PickableType m_pickableType;
	QString m_name;
};





#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"

using RpgInventoryList = qolm::QOlm<RpgInventory>;
Q_DECLARE_METATYPE(RpgInventoryList*)


#endif // RPGPICKABLEOBJECT_H
