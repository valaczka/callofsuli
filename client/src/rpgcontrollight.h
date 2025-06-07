/*
 * ---- Call of Suli ----
 *
 * rpgcontrollight.h
 *
 * Created on: 2025. 05. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlLight
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

#ifndef RPGCONTROLLIGHT_H
#define RPGCONTROLLIGHT_H

#include "rpgcontrol.h"

class RpgControlLight : public RpgControl<RpgGameData::ControlLight, RpgGameData::ControlBaseData>
{
public:
	RpgControlLight(const RpgGameData::ControlBaseData &data,
					const RpgGameData::ControlLight::State state = RpgGameData::ControlLight::LightOn);

	virtual void updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::ControlLight> &snapshot) override;
	virtual void updateFromSnapshot(const RpgGameData::ControlLight &snap) override;

	const RpgGameData::ControlLight::State &state() const;
	void setState(const RpgGameData::ControlLight::State &newState);

	QQuickItem *visualItem() const;
	void setVisualItem(QQuickItem *newVisualItem);

protected:
	virtual RpgGameData::ControlLight serializeThis() const override;

private:
	void updateState();

	RpgGameData::ControlLight::State m_state = RpgGameData::ControlLight::LightOff;

	QPointer<QQuickItem> m_visualItem;
};

#endif // RPGCONTROLLIGHT_H
