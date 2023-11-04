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

#ifndef MODULETEXT_H
#define MODULETEXT_H

#include "../interfaces.h"
#include <QObject>
#include <QtPlugin>


class ModuleText : public QObject, public ModuleInterface
{
	Q_OBJECT

	Q_PLUGIN_METADATA(IID "org.callofsuli.Modules.text")
	Q_INTERFACES(ModuleInterface)

public:
	explicit ModuleText(QObject *parent = nullptr);

	inline QString name() const override { return "text"; }
	inline bool isStorageModule() const override { return true; }
	inline QString readableName() const override { return tr("Szöveg"); }
	inline QString icon() const override { return "image://font/AcademicI/\uf1b2"; }

	inline QString qmlEditor() const override {
#if QT_VERSION >= 0x060000
		return "ME_text_qt6.qml";
#else
		return "ME_text.qml";
#endif
	}
	inline QString qmlQuestion() const override { return QStringLiteral(""); }
	inline QString testResult(const QVariantMap &, const QVariantMap &, const bool &) const override { return QStringLiteral(""); }

	inline QStringList storageModules() const override { return QStringList(); }

	QVariantMap details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const override;

	QVariantList generateAll(const QVariantMap &, ModuleInterface *, const QVariantMap &) const override { return QVariantList(); }

	qreal xpFactor() const override { return 0; };

	QVariantMap preview(const QVariantList &) const override { return QVariantMap(); };

	void registerQmlTypes() const override {};

	QList<int> images(const QVariantMap &) const override { return QList<int>(); }

signals:

};


#endif // MODULEPAIR_H
