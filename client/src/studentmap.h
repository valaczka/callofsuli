/*
 * ---- Call of Suli ----
 *
 * studentmap.h
 *
 * Created on: 2023. 04. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * StudentMap
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

#ifndef STUDENTMAP_H
#define STUDENTMAP_H

#include <QObject>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"
#include "qjsonobject.h"
#include "basemap.h"

class StudentMap;
using StudentMapList = qolm::QOlm<StudentMap>;
Q_DECLARE_METATYPE(StudentMapList*)

/**
 * @brief The TeacherMap class
 */

class StudentMap : public BaseMap
{
	Q_OBJECT

public:
	explicit StudentMap(QObject *parent = nullptr);

	void loadFromJson(const QJsonObject &object, const bool &allField = true);
};

#endif // STUDENTMAP_H
