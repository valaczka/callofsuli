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

#ifndef MODULEPAIR_H
#define MODULEPAIR_H

#include "../interfaces.h"
#include <QObject>
#include <QtPlugin>


class ModulePair : public QObject, public ModuleInterface
{
	Q_OBJECT

	Q_PLUGIN_METADATA(IID "org.callofsuli.Modules.pair")
	Q_INTERFACES(ModuleInterface)

public:
	explicit ModulePair(QObject *parent = nullptr);

	inline QString name() const override { return QStringLiteral("pair"); }
	inline Types types() const override { return Online|PaperAuto; }
	inline QString readableName() const override { return tr("Párosítás"); }
	inline QString icon() const override { return QStringLiteral("qrc:/Qaterial/Icons/vector-link.svg"); }

	inline QString qmlEditor() const override { return QStringLiteral("ME_pair.qml"); }
	inline QString qmlQuestion() const override { return QStringLiteral("GQ_pair.qml"); }

	QString testResult(const QVariantMap &data, const QVariantMap &answer, const bool &) const override;

	inline QStringList storageModules() const override {
		static const QStringList l = {QStringLiteral("numbers"), QStringLiteral("binding"), QStringLiteral("block")};
		return l;
	}

	QVariantMap details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const override;

	QVariantList generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData, QVariantMap *commonDataPtr) const override;

	qreal xpFactor() const override { return 1.5; };

	QVariantMap preview(const QVariantList &, const QVariantMap &) const override { return QVariantMap(); };

	void registerQmlTypes() const override {};

	QVariantList generateBlock(const QVariantMap &storageData) const;
	QVariantMap generateOne(const QVariantMap &data, QVariantList pairList) const;

	enum Mode {
		First = 0x1,
		Second = 0x2
	};
	Q_DECLARE_FLAGS(Modes, Mode)

	QList<int> images(const QVariantMap &) const override { return QList<int>(); }

signals:

};

Q_DECLARE_OPERATORS_FOR_FLAGS(ModulePair::Modes)

#endif // MODULEPAIR_H
