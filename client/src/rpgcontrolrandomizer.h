/*
 * ---- Call of Suli ----
 *
 * rpgcontrolrandomizer.h
 *
 * Created on: 2025. 06. 07.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlRandomizer
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

#ifndef RPGCONTROLRANDOMIZER_H
#define RPGCONTROLRANDOMIZER_H

#include "rpgcontrol.h"




/**
 * @brief The RpgControlRandomizerContent class
 */

class RpgControlRandomizerContent
{
public:
	RpgControlRandomizerContent(const int &id);

	void objectAdd(TiledObjectBody *object, const bool &invert);
	void layerAdd(QQuickItem *item, const bool &invert);

	void setActive(const bool &active = true);

	int id() const;

	bool isEmpty() const { return m_objects.isEmpty() && m_layers.isEmpty() && m_invertObjects.isEmpty() && m_invertLayers.isEmpty(); }

private:
	const int m_id;
	QList<TiledObjectBody *> m_objects;
	QList<QPointer<QQuickItem>> m_layers;
	QList<TiledObjectBody *> m_invertObjects;
	QList<QPointer<QQuickItem>> m_invertLayers;
};





/**
 * @brief The RpgControlRandomizer class
 */

class RpgControlRandomizer: public RpgControlBase
{
public:
	RpgControlRandomizer(RpgGame *game, TiledScene *scene,
						 Tiled::GroupLayer *group, Tiled::MapRenderer *renderer = nullptr);

	const RpgGameData::ControlBaseData &baseData() const;

	void addGroupLayer(TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer = nullptr);

	int activeId() const;
	bool setActiveId(int newActiveId);

	QString name() const;
	void setName(const QString &newName);

	static RpgControlRandomizer *find(const std::vector<std::unique_ptr<RpgControlBase>> &list, Tiled::GroupLayer *group, const int &sceneId);

	RpgGameData::RandomizerGroup toRandomizerGroup() const;
	bool fromRandomizerGroup(const RpgGameData::RandomizerGroup &group);

private:
	QString m_name;								// unique name
	RpgGameData::ControlBaseData m_baseData;
	std::vector<RpgControlRandomizerContent> m_content;
	int m_activeId = -1;


};

#endif // RPGCONTROLRANDOMIZER_H
