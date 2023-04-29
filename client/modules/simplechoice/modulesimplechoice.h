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

#include "abstractobjectiveimporter.h"
#include "xlsxworksheet.h"
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
	inline QString qmlTestResult() const override { return QLatin1String(""); }

	inline QStringList storageModules() const override { return {"binding", "numbers", "images"}; }

	QVariantMap details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const override;

	QVariantList generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const override;

	qreal xpFactor() const override { return 1.1; };

	QVariantList generateBinding(const QVariantMap &data, const QVariantMap &storageData) const;
	QVariantList generateImages(const QVariantMap &data, const QVariantMap &storageData) const;
	QVariantMap generateOne(const QString &correctAnswer, QStringList optionsList) const;

	QVariantMap preview(const QVariantList &generatedList) const override;

	inline bool canImport() const override { return true; }
	AbstractObjectiveImporter* newImporter(QXlsx::Worksheet *worksheet) const override { return new ObjectiveImporterSimplechoice(worksheet); }

	void registerQmlTypes() const override {};

	QList<int> images(const QVariantMap &) const override { return QList<int>(); }

signals:

};


#endif // MODULESIMPLECHOICE_H
