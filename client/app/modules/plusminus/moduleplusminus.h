/*
 * ---- Call of Suli ----
 *
 * moduleplusminus.h
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

#ifndef MODULEPLUSMINUS_H
#define MODULEPLUSMINUS_H

#include "../interfaces.h"
#include <QObject>
#include <QtPlugin>


class ModulePlusminus : public QObject, public ModuleInterface
{
	Q_OBJECT

	Q_PLUGIN_METADATA(IID "org.callofsuli.Modules.plusminus")
	Q_INTERFACES(ModuleInterface)

public:
	explicit ModulePlusminus(QObject *parent = nullptr);

	inline QString name() const override { return "plusminus"; }
	inline bool isStorageModule() const override { return true; }
	inline QString readableName() const override { return tr("Összeadás-kivonás"); }
	inline QString icon() const override { return "image://font/AcademicI/\uf127"; }

	inline QString qmlEditor() const override { return "ME_plusminus.qml"; }
	inline QString qmlQuestion() const override { return ""; }

	inline bool canImport() const override { return false; }
	inline AbstractObjectiveImporter* newImporter(QXlsx::Worksheet *) const override { return nullptr; }

	QVariantMap details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const override;

	QVariantMap generate(const QVariantMap &, ModuleInterface *, const QVariantMap &, QVariantMap *) const override { return QVariantMap(); }

signals:

};

#endif // MODULEPLUSMINUS_H
