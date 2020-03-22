/*
 * ---- Call of Suli ----
 *
 * moduletruefalse.h
 *
 * Created on: 2021. 08. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModuleTruefalse
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

#ifndef MODULETRUEFALSE_H
#define MODULETRUEFALSE_H

#include "../interfaces.h"
#include <QObject>
#include <QtPlugin>

#include "../../abstractobjectiveimporter.h"
#include "../../../QtXlsxWriter/xlsxworksheet.h"
#include "objectiveimportertruefalse.h"


class ModuleTruefalse : public QObject, public ModuleInterface
{
	Q_OBJECT

	Q_PLUGIN_METADATA(IID "org.callofsuli.Modules.truefalse")
	Q_INTERFACES(ModuleInterface)

public:
	explicit ModuleTruefalse(QObject *parent = nullptr);

	inline QString name() const override { return "truefalse"; }
	inline bool isStorageModule() const override { return false; }
	inline QString readableName() const override { return tr("Igaz/hamis"); }
	inline QString icon() const override { return "image://font/School/\uf124"; }

	inline QString qmlEditor() const override { return "ME_truefalse.qml"; }
	inline QString qmlQuestion() const override { return "GQ_truefalse.qml"; }

	inline QStringList storageModules() const override { return QStringList(); }

	QVariantMap details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const override;

	QVariantMap generate(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData, QVariantMap *answer) const override;

	inline bool canImport() const override { return true; }
	AbstractObjectiveImporter* newImporter(QXlsx::Worksheet *worksheet) const override { return new ObjectiveImporterTruefalse(worksheet); }

	void registerQmlTypes() const override {};
signals:

};




#endif // MODULETRUEFALSE_H
