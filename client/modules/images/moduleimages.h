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

#ifndef MODULEIMAGES_H
#define MODULEIMAGES_H

#include "../interfaces.h"
#include <QObject>
#include <QtPlugin>


class ModuleImages : public QObject, public ModuleInterface
{
	Q_OBJECT

	Q_PLUGIN_METADATA(IID "org.callofsuli.Modules.images")
	Q_INTERFACES(ModuleInterface)

public:
	explicit ModuleImages(QObject *parent = nullptr);

	inline QString name() const override { return "images"; }
	inline bool isStorageModule() const override { return true; }
	inline QString readableName() const override { return tr("Képválasztás"); }
	inline QString icon() const override { return "qrc:/Qaterial/Icons/camera-image.svg"; }

	inline QString qmlEditor() const override { return "ME_images.qml"; }
	inline QString qmlQuestion() const override { return QLatin1String(""); }
	inline QString testResult(const QVariantMap &, const QVariantMap &, const bool &) const override { return QLatin1String(""); }

	inline QStringList storageModules() const override { return QStringList(); }

	QVariantMap details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const override;

	QVariantList generateAll(const QVariantMap &, ModuleInterface *, const QVariantMap &) const override { return QVariantList(); }

	qreal xpFactor() const override { return 0; };

	QVariantMap preview(const QVariantList &) const override { return QVariantMap(); };

	inline bool canImport() const override { return false; }
	AbstractObjectiveImporter* newImporter(QXlsx::Worksheet *) const override { return nullptr; }

	void registerQmlTypes() const override {};

	QList<int> images(const QVariantMap &data) const override;

signals:

};


#endif // MODULEPAIR_H
