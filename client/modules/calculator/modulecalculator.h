/*
 * ---- Call of Suli ----
 *
 * modulecalculator.h
 *
 * Created on: 2021. 08. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModuleCalculator
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

#ifndef MODULECALCULATOR_H
#define MODULECALCULATOR_H

#include "../interfaces.h"
#include <QObject>
#include <QtPlugin>


class ModuleCalculator : public QObject, public ModuleInterface
{
	Q_OBJECT

	Q_PLUGIN_METADATA(IID "org.callofsuli.Modules.calculator")
	Q_INTERFACES(ModuleInterface)

public:
	explicit ModuleCalculator(QObject *parent = nullptr);

	inline QString name() const override { return QStringLiteral("calculator"); }
	inline Types types() const override { return Online|PaperManual; }
	inline QString readableName() const override { return tr("Numerikus válasz"); }
	inline QString icon() const override { return QStringLiteral("image://font/AcademicI/\uf127"); }

	inline QString qmlEditor() const override { return QStringLiteral("ME_calculator.qml"); }
	inline QString qmlQuestion() const override { return QStringLiteral("GQ_calculator.qml"); }

	QString testResult(const QVariantMap &, const QVariantMap &answer, const bool &success) const override;

	inline QStringList storageModules() const override {
		static const QStringList l = {QStringLiteral("plusminus"), QStringLiteral("numbers")};
		return l;
	}

	QVariantMap details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const override;

	QVariantList generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData, QVariantMap *commonDataPtr) const override;

	qreal xpFactor() const override { return 1.2; };

	QVariantMap preview(const QVariantList &generatedList, const QVariantMap &commonData) const override;


	QVariantMap generatePlusminus(const QVariantMap &data) const;
	QVariantList generateNumbers(const QVariantMap &data, const QVariantMap &storageData) const;

	void registerQmlTypes() const override {};

	QList<int> images(const QVariantMap &) const override { return QList<int>(); };

signals:

};


#endif // MODULECALCULATOR_H
