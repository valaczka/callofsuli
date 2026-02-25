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

#ifndef MODULEDOUBLECHOICE_H
#define MODULEDOUBLECHOICE_H

#include "../interfaces.h"
#include <QObject>
#include <QtPlugin>


class ModuleDoublechoice : public QObject, public ModuleInterface
{
	Q_OBJECT

	Q_PLUGIN_METADATA(IID "org.callofsuli.Modules.doublechoice")
	Q_INTERFACES(ModuleInterface)

public:
	explicit ModuleDoublechoice(QObject *parent = nullptr);

	inline QString name() const override { return QStringLiteral("doublechoice"); }
	inline Types types() const override { return Online|PaperAuto; }
	inline QString readableName() const override { return tr("Dupla választás"); }
	inline QString icon() const override { return QStringLiteral("image://font/Academic/\uf21c"); }

	inline QString qmlEditor() const override { return QStringLiteral("ME_doublechoice.qml"); }
	inline QString qmlQuestion() const override { return QStringLiteral("GQ_doublechoice.qml"); }

	QString testResult(const QVariantMap &data, const QVariantMap &answer, const bool &success) const override;

	inline QStringList storageModules() const override {
		static const QStringList l = {
			QStringLiteral("block"),
			QStringLiteral("mergeblock"),
			QStringLiteral("binding"),
			QStringLiteral("mergebinding"),
		};
		return l;
	}

	QVariantMap details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const override;

	QVariantList generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData,
							 QVariantMap *commonDataPtr, StorageSeed *seed) const override;

	qreal xpFactor() const override { return 2.5; };

	QVariantMap preview(const QVariantList &generatedList, const QVariantMap &commonData) const override;

	void registerQmlTypes() const override {};

	QVariantMap generateOne(const QVariantMap &data) const;

	QVariantList generateBinding(const QVariantMap &data, const QVariantMap &storageData, StorageSeed *seed) const;
	QVariantList generateMergeBinding(const QVariantMap &data, const QVariantMap &storageData, StorageSeed *seed) const;
	QVariantList generateBlock(const bool &isMerge, const QVariantMap &data, const QVariantMap &storageData, StorageSeed *seed) const;

	QList<int> images(const QVariantMap &) const override { return QList<int>(); }

	static std::optional<quint8> getOptionValue(const quint8 &value, const bool &isA);
	static QString getOptionString(const quint8 &value, const bool &isA);

	struct DoubleData
	{
		DoubleData(const QStringList &aList, const QString &correct, const QString &separator);

		QString correctA;
		QString correctB;
		QSet<QString> oListA;
		QSet<QString> oListB;
	};


	static QVariantMap _generate(const DoubleData &data, int maxOptionsA, int maxOptionsB);

private:
	static const QList<quint8> m_optionsA;
	static const QList<quint8> m_optionsB;

signals:

};

#endif // MODULEMULTICHOICE_H
