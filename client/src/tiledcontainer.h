/*
 * ---- Call of Suli ----
 *
 * tiledcontainer.h
 *
 * Created on: 2024. 04. 26.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledContainer
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

#ifndef TILEDCONTAINER_H
#define TILEDCONTAINER_H

#include "tiledscene.h"
#include <QObject>

class TiledContainer : public QObject
{
	Q_OBJECT

	Q_PROPERTY(ContainerType type READ type WRITE setType NOTIFY typeChanged FINAL)
	Q_PROPERTY(bool isActive READ isActive WRITE setIsActive NOTIFY isActiveChanged FINAL)
	Q_PROPERTY(TiledScene *scene READ scene WRITE setScene NOTIFY sceneChanged FINAL)

public:
	explicit TiledContainer(QObject *parent = nullptr);

	enum ContainerType {
		ContainerInvalid = 0,
		ContainerBase
	};

	Q_ENUM(ContainerType);

	ContainerType type() const;
	void setType(const ContainerType &newType);

	bool isActive() const;
	void setIsActive(bool newIsActive);

	TiledScene *scene() const;
	void setScene(TiledScene *newScene);

signals:
	void isActiveChanged();
	void typeChanged();
	void sceneChanged();

protected:
	virtual void onActivated() {};
	virtual void onDeactivated() {};

	ContainerType m_type = ContainerInvalid;
	bool m_isActive = true;
	QPointer<TiledScene> m_scene;
};

#endif // TILEDCONTAINER_H
