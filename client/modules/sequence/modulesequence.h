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

#ifndef MODULESEQUENCE_H
#define MODULESEQUENCE_H

#include "../interfaces.h"
#include <QObject>
#include <QtPlugin>


class ModuleSequence : public QObject, public ModuleInterface
{
	Q_OBJECT

	Q_PLUGIN_METADATA(IID "org.callofsuli.Modules.sequence")
	Q_INTERFACES(ModuleInterface)

public:
	explicit ModuleSequence(QObject *parent = nullptr);

	inline QString name() const override { return QStringLiteral("sequence"); }
	inline Types types() const override { return Storage; }
	inline QString readableName() const override { return tr("Sorozat"); }
	inline QString icon() const override { return QStringLiteral("image://font/AcademicI/\uf100"); }

	inline QString qmlEditor() const override { return QStringLiteral("ME_sequence.qml"); }
	inline QString qmlQuestion() const override { return QString(); }
	inline QString testResult(const QVariantMap &, const QVariantMap &, const bool &) const override { return QString(); }

	inline QStringList storageModules() const override { return QStringList(); }

	QVariantMap details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const override;

	QVariantList generateAll(const QVariantMap &, ModuleInterface *, const QVariantMap &,
							 QVariantMap *, StorageSeed *) const override { return QVariantList(); }

	qreal xpFactor() const override { return 0; };

	QVariantMap preview(const QVariantList &, const QVariantMap &) const override { return QVariantMap(); };

	void registerQmlTypes() const override {};

	QList<int> images(const QVariantMap &) const override { return QList<int>(); }

signals:

};


#endif // MODULEPAIR_H
