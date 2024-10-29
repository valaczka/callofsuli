/*
 * ---- Call of Suli ----
 *
 * rpgcontrolgroupstate.h
 *
 * Created on: 2024. 10. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlGroupState
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

#ifndef RPGCONTROLGROUPSTATE_H
#define RPGCONTROLGROUPSTATE_H

#include "rpgcontrolgroup.h"

class RpgControlGroupState : public RpgControlGroup
{
public:
	RpgControlGroupState(const RpgControlGroup::Type &type, RpgGame *game, TiledScene *scene, Tiled::GroupLayer *group);

	TiledVisualItem *visualItem() const;
	void setVisualItem(TiledVisualItem *newVisualItem);

	TiledVisualItem *createVisualItem(Tiled::GroupLayer *layer);

	int currentState() const;
	void setCurrentState(int newCurrentState);

	QPointF basePosition() const;
	void setBasePosition(QPointF newBasePosition);

protected:
	struct State {
		int id = -1;
		QUrl image;
		QPointF relativePosition;
	};

	const QVector<State> &states() const;
	QVector<State>::const_iterator cstate(const int &id = -1) const;
	QVector<State>::iterator state(const int &id = -1);

	bool stateAdd(const State &state);

	void refreshVisualItem() const;

	QPointF m_basePosition;
	QVector<State> m_states;
	QPointer<TiledVisualItem> m_visualItem;
	int m_currentState = -1;
};



#endif // RPGCONTROLGROUPSTATE_H
