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


class ModuleTruefalse : public QObject, public ModuleInterface
{
	Q_OBJECT

	Q_PLUGIN_METADATA(IID "org.callofsuli.Modules.truefalse")
	Q_INTERFACES(ModuleInterface)

public:
	explicit ModuleTruefalse(QObject *parent = nullptr);

	inline QString name() const override { return QStringLiteral("truefalse"); }
	inline Types types() const override { return Online|PaperAuto; }
	inline QString readableName() const override { return tr("Igaz/hamis"); }
	inline QString icon() const override { return QStringLiteral("qrc:/Qaterial/Icons/list-status.svg"); }

	inline QString qmlEditor() const override { return QStringLiteral("ME_singlechoice.qml"); }
	inline QString qmlQuestion() const override { return QStringLiteral("GQ_singlechoice.qml"); }
	QString testResult(const QVariantMap &data, const QVariantMap &answer, const bool &success) const override;

	inline QStringList storageModules() const override {
		static const QStringList l = {
			QStringLiteral("numbers"), QStringLiteral("binding"), QStringLiteral("block")
		};
		return l;
	}

	QVariantMap details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const override;

	QVariantList generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData, QVariantMap *commonDataPtr) const override;

	QVariantList generateBinding(const QVariantMap &data, const QVariantMap &storageData) const;
	QVariantList generateBlock(const QVariantMap &data, const QVariantMap &storageData) const;

	qreal xpFactor() const override { return 1.0; };

	QVariantMap preview(const QVariantList &generatedList, const QVariantMap &commonData) const override;

	void registerQmlTypes() const override {};

	QList<int> images(const QVariantMap &) const override { return QList<int>(); }

signals:

};




#endif // MODULETRUEFALSE_H
