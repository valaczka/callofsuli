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
#include "../mergeblock/modulemergeblock.h"
#include <QObject>
#include <QtPlugin>


class ModuleWriter : public QObject, public ModuleInterface
{
	Q_OBJECT

	Q_PLUGIN_METADATA(IID "org.callofsuli.Modules.writer")
	Q_INTERFACES(ModuleInterface)

public:
	explicit ModuleWriter(QObject *parent = nullptr);

	inline QString name() const override { return QStringLiteral("writer"); }
	inline Types types() const override { return Online|PaperManual; }
	inline QString readableName() const override { return tr("Szöveges válasz"); }
	inline QString icon() const override { return QStringLiteral("qrc:/Qaterial/Icons/receipt-text-edit.svg"); }

	inline QString qmlEditor() const override { return QStringLiteral("ME_writer.qml"); }
	inline QString qmlQuestion() const override { return QStringLiteral("GQ_writer.qml"); }
	QString testResult(const QVariantMap &data, const QVariantMap &answer, const bool &success) const override;

	inline QStringList storageModules() const override {
		static const QStringList l = {
			QStringLiteral("text"),
			QStringLiteral("binding"),
			QStringLiteral("sequence"),
			QStringLiteral("images"),
			QStringLiteral("block"),
			QStringLiteral("mergebinding"),
			QStringLiteral("mergeblock"),
		};
		return l;
	}

	QVariantMap details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const override;

	QVariantList generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData,
							 QVariantMap *commonDataPtr, StorageSeed *seed) const override;

	qreal xpFactor() const override { return 1.8; };

	QVariantMap preview(const QVariantList &generatedList, const QVariantMap &commonData) const override;

	void registerQmlTypes() const override;

	QVariantList generateBinding(const QVariantMap &data, const QVariantMap &storageData, StorageSeed *seed) const;
	QVariantList generateImages(const QVariantMap &data, const QVariantMap &storageData, StorageSeed *seed) const;
	QVariantList generateSequence(const QVariantMap &data, const QVariantMap &storageData) const;
	QVariantList generateText(const QVariantMap &data, const QVariantMap &storageData, StorageSeed *seed) const;
	QVariantList generateBlockContains(const QVariantMap &data, const ModuleMergeblock::BlockUnion &blocks, StorageSeed *seed) const;
	QVariantList generateMergeBinding(const QVariantMap &data, const QVariantMap &storageData, StorageSeed *seed) const;

	QList<int> images(const QVariantMap &) const override { return QList<int>(); };

private:
	static const QRegularExpression m_expressionWord;
	static const QString m_punctation;
	static const QString m_placeholder;

	friend class ModuleFillout;
	friend class FilloutSyntaxHighlighter;
};

#endif // MODULEFILLOUT_H
