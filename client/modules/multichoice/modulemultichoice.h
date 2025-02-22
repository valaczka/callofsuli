/*
 * ---- Call of Suli ----
 *
 * modulemultichoice.h
 *
 * Created on: 2021. 11. 13.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModuleMultichoice
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

#ifndef MODULEMULTICHOICE_H
#define MODULEMULTICHOICE_H

#include "../interfaces.h"
#include <QObject>
#include <QtPlugin>


class ModuleMultichoice : public QObject, public ModuleInterface
{
	Q_OBJECT

	Q_PLUGIN_METADATA(IID "org.callofsuli.Modules.multichoice")
	Q_INTERFACES(ModuleInterface)

public:
	explicit ModuleMultichoice(QObject *parent = nullptr);

	inline QString name() const override { return QStringLiteral("multichoice"); }
	inline Types types() const override { return Online|PaperAuto; }
	inline QString readableName() const override { return tr("Többszörös választás"); }
	inline QString icon() const override { return QStringLiteral("image://font/Academic/\uf155"); }

	inline QString qmlEditor() const override { return QStringLiteral("ME_multichoice.qml"); }
	inline QString qmlQuestion() const override { return QStringLiteral("GQ_multichoice.qml"); }

	QString testResult(const QVariantMap &data, const QVariantMap &answer, const bool &success) const override;

	inline QStringList storageModules() const override {
		static const QStringList l = {QStringLiteral("block")};
		return l;
	}

	QVariantMap details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const override;

	QVariantList generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData, QVariantMap *commonDataPtr) const override;

	qreal xpFactor() const override { return 1.5; };

	QVariantMap preview(const QVariantList &generatedList, const QVariantMap &commonData) const override;

	void registerQmlTypes() const override {};

	QVariantList generateBlock(const QVariantMap &data, const QVariantMap &storageData) const;
	QVariantMap generateOne(const QVariantMap &data) const;

	QList<int> images(const QVariantMap &) const override { return QList<int>(); }

private:
	QVariantMap _generate(QStringList correctList, QStringList optionsList, int minCorrect, int maxOptions, int maxCorrect) const;

signals:

};

#endif // MODULEMULTICHOICE_H
