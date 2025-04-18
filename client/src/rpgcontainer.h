/*
 * ---- Call of Suli ----
 *
 * rpgcontainer.h
 *
 * Created on: 2024. 12. 30.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgContainer
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

#ifndef RPGCONTAINER_H
#define RPGCONTAINER_H

#include <QObject>
#include "rpgcontrolgroupstate.h"
#include "tiledscene.h"

class RpgContainer : public QObject, public RpgControlGroupStateBody
{
	Q_OBJECT

	Q_PROPERTY(bool isActive READ isActive WRITE setIsActive NOTIFY isActiveChanged FINAL)
	Q_PROPERTY(TiledScene *scene READ scene CONSTANT FINAL)

public:
	explicit RpgContainer(TiledGame *game);

	bool isActive() const;
	void setIsActive(bool newIsActive);

signals:
	void isActiveChanged();
	void typeChanged();

protected:
	virtual void onActivated() {};
	virtual void onDeactivated() {};

	bool m_isActive = true;
};

#endif // RPGCONTAINER_H
