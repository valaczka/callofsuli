/*
 * ---- Call of Suli ----
 *
 * interfaces.h
 *
 * Created on: 2021. 08. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
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

#ifndef INTERFACES_H
#define INTERFACES_H

#include <QtPlugin>

class ModuleInterface
{
public:
	virtual ~ModuleInterface() = default;

	// A modul neve (ezt használja az adatbázis)
	virtual QString name() const = 0;

	// Objective vagy storage modul
	virtual bool isStorageModule() const = 0;

	// A modul ovlasható neve (ami megjelenik)
	virtual QString readableName() const = 0;

	// A modul ikonja
	virtual QString icon() const = 0;

	// Ha objective modul, akkor a használható storage modulok listája
	virtual QStringList storageModules() const = 0;


	// A szerkesztőfelület (QML)
	virtual QString qmlEditor() const = 0;

	// A kérdező felület a játékban (QML)
	virtual QString qmlQuestion() const = 0;

	// A GameMode::Test és GameMode::Exam típusú játékokban az eredmények megtekintése (HTML)
	virtual QString testResult(const QVariantMap &data, const QVariantMap &answer, const bool &success) const = 0;

	// Információk a szerkesztőfelülethez
	virtual QVariantMap details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const = 0;

	// Az összes kérdés/feladat elkészítése
	virtual QVariantList generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const = 0;

	// XP faktor
	virtual qreal xpFactor() const = 0;

	// Előnézet készítése
	virtual QVariantMap preview(const QVariantList &generatedList) const = 0;

	// QML-type register
	virtual void registerQmlTypes() const = 0;

	// Képek azonosítói, amiket tartalmaz
	virtual QList<int> images(const QVariantMap &data) const = 0;
};

QT_BEGIN_NAMESPACE

#define ModuleInterface_iid "org.callofsuli.Modules.ModuleInterface/1.0"
Q_DECLARE_INTERFACE(ModuleInterface, ModuleInterface_iid)

QT_END_NAMESPACE

#endif // INTERFACES_H
