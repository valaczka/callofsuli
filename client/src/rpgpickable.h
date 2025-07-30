/*
 * ---- Call of Suli ----
 *
 * rpgpickable.h
 *
 * Created on: 2025. 06. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgPickable
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

#ifndef RPGPICKABLE_H
#define RPGPICKABLE_H

#include "rpgcontrol.h"



/**
 * @brief The RpgPickable class
 */

class RpgPickable : public RpgActiveControl<RpgGameData::Pickable,
		RpgGameData::PickableBaseData,
		RpgActiveIface::DefaultEnum>
{
public:
	RpgPickable(RpgGame *game, TiledScene *scene, const RpgGameData::PickableBaseData &base);
	virtual ~RpgPickable() = default;

	virtual void updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::Pickable> &snapshot) override;
	virtual void updateFromSnapshot(const RpgGameData::Pickable &snap) override;

	static QHash<RpgGameData::PickableBaseData::PickableType, QString> typeHash() { return m_typeHash; }

	void playSfx(const QString &sound = QStringLiteral(":/sound/sfx/pick.mp3")) const;

protected:
	virtual RpgGameData::Pickable serializeThis() const override;

	virtual void onShapeContactBegin(cpShape *self, cpShape *other) override;
	virtual void onShapeContactEnd(cpShape *self, cpShape *other) override;

private:
	static const QHash<RpgGameData::PickableBaseData::PickableType, QString> m_typeHash;

	void _updateGlow();
};




/**
 * @brief The RpgControlCollectionObject class
 */

class RpgPickableControlObject : public RpgActiveControlObject
{
	Q_OBJECT

public:
	explicit RpgPickableControlObject(RpgPickable *control,
									  const QPointF &center, const qreal &radius,
									  TiledGame *game);

	RpgPickable *pickable() const { return m_control; }

protected:
	virtual void synchronize() override;

private:
	RpgPickable *const m_control;

};


#endif // RPGPICKABLE_H
