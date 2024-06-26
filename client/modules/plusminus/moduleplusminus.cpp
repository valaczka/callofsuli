/*
 * ---- Call of Suli ----
 *
 * moduleplusminus.cpp
 *
 * Created on: 2021. 08. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModulePlusminus
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

#include "moduleplusminus.h"

ModulePlusminus::ModulePlusminus(QObject *parent) : QObject(parent)
{

}

/**
 * @brief ModulePlusminus::details
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantMap ModulePlusminus::details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	Q_UNUSED(data)
	Q_UNUSED(storage)
	Q_UNUSED(storageData)

	QVariantMap m;
	m[QStringLiteral("title")] = tr("Egész számok összeadása-kivonása");
	m[QStringLiteral("details")] = QStringLiteral("");
	m[QStringLiteral("image")] = QStringLiteral("");

	return m;
}

