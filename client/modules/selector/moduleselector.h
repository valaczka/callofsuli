/*
 * ---- Call of Suli ----
 *
 * moduleselector.h
 *
 * Created on: 2021. 08. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModuleSelector
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

#ifndef MODULESELECTOR_H
#define MODULESELECTOR_H

#include "../interfaces.h"
#include "../mergeblock/modulemergeblock.h"
#include <QObject>
#include <QtPlugin>


class ModuleSelector : public QObject, public ModuleInterface
{
	Q_OBJECT

	Q_PLUGIN_METADATA(IID "org.callofsuli.Modules.selector")
	Q_INTERFACES(ModuleInterface)

public:
	explicit ModuleSelector(QObject *parent = nullptr);

	inline QString name() const override { return QStringLiteral("selector"); }
	inline Types types() const override { return PaperAuto; }
	inline QString readableName() const override { return tr("Karakter-megjelölés"); }
	inline QString icon() const override { return QStringLiteral("qrc:/Qaterial/Icons/spellcheck.svg"); }

	inline QString qmlEditor() const override { return QStringLiteral("ME_selector.qml"); }
	inline QString qmlQuestion() const override { return QString(); }
	QString testResult(const QVariantMap &data, const QVariantMap &answer, const bool &success) const override;

	inline QStringList storageModules() const override {
		static const QStringList l = {
			QStringLiteral("binding"),
			QStringLiteral("block"),
			QStringLiteral("mergebinding"),
			QStringLiteral("mergeblock"),
		};
		return l;
	}

	QVariantMap details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const override;

	QVariantList generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData,
							 QVariantMap *commonDataPtr, StorageSeed *seed) const override;

	QVariantList generateBinding(const QVariantMap &data,
								 const QVariantMap &storageData,
								 QVariantMap *commonDataPtr, StorageSeed *seed) const;

	QVariantList generateMergeBinding(const QVariantMap &data,
									  const QVariantMap &storageData,
									  QVariantMap *commonDataPtr, StorageSeed *seed) const;

	QVariantList generateBlockContains(const QVariantMap &data,
									   const ModuleMergeblock::BlockUnion &blocks,
									   QVariantMap *commonDataPtr, StorageSeed *seed) const;

	qreal xpFactor() const override { return 1.0; };

	QVariantMap preview(const QVariantList &generatedList, const QVariantMap &commonData) const override;

	void registerQmlTypes() const override {};

	QList<int> images(const QVariantMap &) const override { return QList<int>(); }

private:
	QVariantMap generateOne(const QString &answer, const int &maxOptions) const;
};




#endif // MODULESELECTOR_H
