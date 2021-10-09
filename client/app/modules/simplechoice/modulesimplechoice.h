/*
 * ---- Call of Suli ----
 *
 * modulesimplechoice.h
 *
 * Created on: 2021. 08. 13.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModuleSimplechoice
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

#ifndef MODULESIMPLECHOICE_H
#define MODULESIMPLECHOICE_H


#include "../interfaces.h"
#include <QObject>
#include <QtPlugin>

#include "../../abstractobjectiveimporter.h"
#include "../../../QtXlsxWriter/xlsxworksheet.h"
#include "objectiveimportersimplechoice.h"

class ModuleSimplechoice : public QObject, public ModuleInterface
{
	Q_OBJECT

	Q_PLUGIN_METADATA(IID "org.callofsuli.Modules.simplechoice")
	Q_INTERFACES(ModuleInterface)

public:
	explicit ModuleSimplechoice(QObject *parent = nullptr);

	inline QString name() const override { return "simplechoice"; }
	inline bool isStorageModule() const override { return false; }
	inline QString readableName() const override { return tr("Egyszerű választás"); }
	inline QString icon() const override { return "image://font/Material Icons/\ue1db"; }

	inline QString qmlEditor() const override { return "ME_simplechoice.qml"; }
	inline QString qmlQuestion() const override { return "GQ_simplechoice.qml"; }

	inline QStringList storageModules() const override { return QStringList(); }

	QVariantMap details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const override;

	QVariantMap generate(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData, QVariantMap *answer) const override;

	inline bool canImport() const override { return true; }
	AbstractObjectiveImporter* newImporter(QXlsx::Worksheet *worksheet) const override { return new ObjectiveImporterSimplechoice(worksheet); }

signals:

};


#endif // MODULESIMPLECHOICE_H