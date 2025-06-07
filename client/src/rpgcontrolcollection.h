/*
 * ---- Call of Suli ----
 *
 * rpgcontrolcollection.h
 *
 * Created on: 2025. 05. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlCollection
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

#ifndef RPGCONTROLCOLLECTION_H
#define RPGCONTROLCOLLECTION_H

#include "rpgcontrol.h"


/**
 * @brief The RpgControlCollection class
 */

class RpgControlCollection : public RpgActiveControl<RpgGameData::ControlCollection,
		RpgGameData::ControlCollectionBaseData,
		RpgActiveIface::DefaultEnum>
{
public:
	RpgControlCollection(RpgGame *game, TiledScene *scene, const RpgGameData::ControlCollectionBaseData &base, const QPointF &pos);

	virtual void updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::ControlCollection> &snapshot) override;
	virtual void updateFromSnapshot(const RpgGameData::ControlCollection &snap) override;

protected:
	virtual RpgGameData::ControlCollection serializeThis() const override;

	virtual void onShapeContactBegin(cpShape *self, cpShape *other) override;
	virtual void onShapeContactEnd(cpShape *self, cpShape *other) override;

private:
	void _updateGlow();
};




/**
 * @brief The RpgControlCollectionObject class
 */

class RpgControlCollectionObject : public RpgActiveControlObject
{
	Q_OBJECT

public:
	explicit RpgControlCollectionObject(RpgControlCollection *control,
										const QPointF &center, const qreal &radius,
										TiledGame *game);

protected:
	virtual void synchronize() override;

private:
	RpgControlCollection *const m_control;

};

#endif // RPGCONTROLCOLLECTION_H
