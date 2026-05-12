/*
 * ---- Call of Suli ----
 *
 * rpgobject.h
 *
 * Created on: 2026. 05. 10.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgObject
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

#ifndef RPGOBJECT_H
#define RPGOBJECT_H

#include <QQmlEngine>
#include "abstracttiledmotor.h"
#include "isometricobject.h"
#include "rpggame.h"
#include "rpggameitem.h"


class RpgObject;

/**
 * @brief The AbstractRpgMotor class
 */

class AbstractRpgMotor : public AbstractTiledMotor
{
public:
	AbstractRpgMotor(RpgObject *rpgObject);

	virtual bool beforeWorldStep(const qint64 &tick, entt::entity &entity) { Q_UNUSED(tick); Q_UNUSED(entity); return false; }
	virtual bool afterWorldStep(const qint64 &tick, entt::entity &entity) { Q_UNUSED(tick); Q_UNUSED(entity); return false; }

protected:
	virtual void onShapeContactBegin(cpShape *self, cpShape *other) { Q_UNUSED(self); Q_UNUSED(other); }
	virtual void onShapeContactEnd(cpShape *self, cpShape *other) { Q_UNUSED(self); Q_UNUSED(other); }


protected:
	RpgObject *const m_object;
	RpgGameItem *m_gameItem = nullptr;
	RpgGame *m_game = nullptr;

	friend class RpgObject;
};





/**
 * @brief The RpgObject class
 */

class RpgObject : public IsometricObject
{
	Q_OBJECT
	QML_ELEMENT

public:
	RpgObject(RpgGameItem *gameItem, const QPointF &center = {}, const qreal &radius = 10., const cpBodyType &type = CP_BODY_TYPE_DYNAMIC);

	virtual void updateSprite() {}

	AbstractRpgMotor* defaultMotor() const;
	void setDefaultMotor(std::unique_ptr<AbstractRpgMotor> newDefaultMotor);

	AbstractRpgMotor* secondaryMotor() const;
	void setSecondaryMotor(std::unique_ptr<AbstractRpgMotor> newSecondaryMotor);

	AbstractRpgMotor* currentMotor() const { return m_secondaryMotor ? m_secondaryMotor.get() : m_defaultMotor.get(); }

protected:
	void worldStep() override final;
	void onShapeContactBegin(cpShape *self, cpShape *other) override final;
	void onShapeContactEnd(cpShape *self, cpShape *other) override final;

protected:
	RpgGame *m_rpgGame = nullptr;
	std::unique_ptr<AbstractRpgMotor> m_defaultMotor;
	std::unique_ptr<AbstractRpgMotor> m_secondaryMotor;

	friend class AbstractRpgMotor;
};

#endif // RPGOBJECT_H
