/*
 * ---- Call of Suli ----
 *
 * modulepair.h
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

#ifndef MODULEORDER_H
#define MODULEORDER_H

#include "../interfaces.h"
#include <QObject>
#include <QtPlugin>


class ModuleOrder : public QObject, public ModuleInterface
{
	Q_OBJECT

	Q_PLUGIN_METADATA(IID "org.callofsuli.Modules.order")
	Q_INTERFACES(ModuleInterface)

public:
	explicit ModuleOrder(QObject *parent = nullptr);

	inline QString name() const override { return "order"; }
	inline bool isStorageModule() const override { return false; }
	inline QString readableName() const override { return tr("Sorbarendezés"); }
	inline QString icon() const override { return "image://font/AcademicI/\uf145"; }

	inline QString qmlEditor() const override { return "ME_order.qml"; }
	inline QString qmlQuestion() const override { return "GQ_order.qml"; }

	inline QStringList storageModules() const override { return { "sequence", "numbers" }; }

	QVariantMap details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const override;

	QVariantList generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const override;

	qreal xpFactor() const override { return 1.5; };

	QVariantMap preview(const QVariantList &) const override { return QVariantMap(); };

	inline bool canImport() const override { return false; }
	AbstractObjectiveImporter* newImporter(QXlsx::Worksheet *) const override { return nullptr; }

	void registerQmlTypes() const override {};

	QVariantList generateItems(const QStringList &list, const int &count) const;
	QVariantList generateItems(const QVariantList &list, const int &count) const;

signals:

};


#endif // MODULEPAIR_H
