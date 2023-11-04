/*
 * ---- Call of Suli ----
 *
 * modulepair.cpp
 *
 * Created on: 2021. 11. 13.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModulePair
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

#include "moduleimages.h"
#include <QRandomGenerator>

ModuleImages::ModuleImages(QObject *parent) : QObject(parent)
{

}


/**
 * @brief ModulePair::details
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantMap ModuleImages::details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	Q_UNUSED(storage)
	Q_UNUSED(storageData)

	QStringList list;

	const QVariantList &l = data.value(QStringLiteral("images")).toList();
	QString image = QStringLiteral("");
	foreach (const QVariant &v, l) {
		const QVariantMap &m = v.toMap();
		list.append(m.value(QStringLiteral("first")).toString());
		const int &img = m.value(QStringLiteral("second"), -1).toInt();
		if (image.isEmpty() && img != -1)
			image = QStringLiteral("image://mapimage/%1").arg(img);
	}

	QVariantMap m;
	m[QStringLiteral("title")] = list.join(QStringLiteral(", "));
	m[QStringLiteral("details")] = QStringLiteral("");
	m[QStringLiteral("image")] = image;

	return m;
}


/**
 * @brief ModuleImages::images
 * @return
 */

QList<int> ModuleImages::images(const QVariantMap &data) const
{
	QList<int> list;

	const QVariantList &l = data.value(QStringLiteral("images")).toList();
	foreach (const QVariant &v, l) {
		const QVariantMap &m = v.toMap();
		if (m.contains(QStringLiteral("second")))
			list.append(m.value(QStringLiteral("second"), -1).toInt());
	}
	return list;
}
