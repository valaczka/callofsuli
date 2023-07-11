/*
 * ---- Call of Suli ----
 *
 * modulefillout.h
 *
 * Created on: 2021. 11. 07.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModuleFillout
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

#ifndef MODULEFILLOUT_H
#define MODULEFILLOUT_H

#include "../interfaces.h"
#include <QObject>
#include <QtPlugin>


class ModuleFillout : public QObject, public ModuleInterface
{
	Q_OBJECT

	Q_PLUGIN_METADATA(IID "org.callofsuli.Modules.fillout")
	Q_INTERFACES(ModuleInterface)

public:
	explicit ModuleFillout(QObject *parent = nullptr);

	inline QString name() const override { return "fillout"; }
	inline bool isStorageModule() const override { return false; }
	inline QString readableName() const override { return tr("Szövegkitöltés"); }
	inline QString icon() const override { return "image://font/Books/\uf120"; }

	inline QString qmlEditor() const override { return "ME_fillout.qml"; }
	inline QString qmlQuestion() const override { return "GQ_fillout.qml"; }
	QString testResult(const QVariantMap &data, const QVariantMap &answer, const bool &) const override;

	inline QStringList storageModules() const override { return QStringList(); }

	QVariantMap details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const override;

	QVariantList generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const override;

	qreal xpFactor() const override { return 1.2; };

	QVariantMap preview(const QVariantList &) const override { return QVariantMap(); };

	void registerQmlTypes() const override;

	static const QRegularExpression &expressionWord();

	QVariantMap generateOne(const QVariantMap &data) const;

	QList<int> images(const QVariantMap &) const override { return QList<int>(); };

signals:

private:
	static const QRegularExpression m_expressionWord;

};

#endif // MODULEFILLOUT_H
