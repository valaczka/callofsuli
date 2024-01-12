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

#ifndef MODULEWRITER_H
#define MODULEWRITER_H

#include "../interfaces.h"
#include <QObject>
#include <QtPlugin>


class ModuleWriter : public QObject, public ModuleInterface
{
	Q_OBJECT

	Q_PLUGIN_METADATA(IID "org.callofsuli.Modules.writer")
	Q_INTERFACES(ModuleInterface)

public:
	explicit ModuleWriter(QObject *parent = nullptr);

	inline QString name() const override { return "writer"; }
	inline bool isStorageModule() const override { return false; }
	inline QString readableName() const override { return tr("Szöveges válasz"); }
	inline QString icon() const override { return "qrc:/Qaterial/Icons/receipt-text-edit.svg"; }

	inline QString qmlEditor() const override {
#if QT_VERSION >= 0x060000
		return "ME_writer_qt6.qml";
#else
		return "ME_writer.qml";
#endif
	}
	inline QString qmlQuestion() const override {
#if QT_VERSION >= 0x060000
		return "GQ_writer_qt6.qml";
#else
		return "GQ_writer.qml";
#endif
	}
	QString testResult(const QVariantMap &data, const QVariantMap &answer, const bool &success) const override;

	inline QStringList storageModules() const override { return {"binding", "images", "sequence", "text"}; }

	QVariantMap details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const override;

	QVariantList generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const override;

	qreal xpFactor() const override { return 2.5; };

	QVariantMap preview(const QVariantList &generatedList) const override;

	void registerQmlTypes() const override;

	QVariantList generateBinding(const QVariantMap &data, const QVariantMap &storageData) const;
	QVariantList generateImages(const QVariantMap &data, const QVariantMap &storageData) const;
	QVariantList generateSequence(const QVariantMap &data, const QVariantMap &storageData) const;
	QVariantList generateText(const QVariantMap &data, const QVariantMap &storageData) const;

	QList<int> images(const QVariantMap &) const override { return QList<int>(); };

private:
	static const QRegularExpression m_expressionWord;
	static const QString m_punctation;
	static const QString m_placeholder;
};

#endif // MODULEFILLOUT_H
