/*
 * ---- Call of Suli ----
 *
 * mapeditormatriximporter.cpp
 *
 * Created on: 2025. 10. 18.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapEditorMatrixImporter
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

#include <QUuid>
#include "mapeditormatriximporter.h"
#include "application.h"
#include "utils_.h"
#include "xlsxcellformula.h"
#include "xlsxconditionalformatting.h"
#include "xlsxdatavalidation.h"
#include "xlsxdocument.h"


#define LEVEL_COUNT	20


static const int startRow = 6;
static const int startCol = 7;
static const int dataCol = 2;
static const int lastCol = startCol+LEVEL_COUNT;

static const std::vector<QByteArray> columnName = {
	"",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"AA",
	"AB",
	"AC",
	"AD",
	"AE",
	"AF",
	"AG",
	"AH",
	"AI",
	"AJ",
	"AK",
	"AL",
	"AM",
	"AN",
	"AO",
	"AP",
	"AR",
	"AS",
	"AT",
	"AU",
};

MapEditorMatrixImporter::MapEditorMatrixImporter(QObject *parent)
	: QObject{parent}
	, m_hash(QUuid::createUuid().toString(QUuid::WithoutBraces))
{


}

MapEditor *MapEditorMatrixImporter::mapEditor() const
{
	return m_mapEditor;
}

void MapEditorMatrixImporter::setMapEditor(MapEditor *newMapEditor)
{
	if (m_mapEditor == newMapEditor)
		return;
	m_mapEditor = newMapEditor;
	emit mapEditorChanged();
}




/**
 * @brief MapEditorMatrixImporter::downloadTemplate
 * @param file
 */

void MapEditorMatrixImporter::downloadTemplate(const QUrl &file)
{
	if (!m_mapEditor) {
		Application::instance()->messageInfo(tr("MapEditor hiányzik"), tr("Belső hiba"));
		return;
	}

	const QString &fileName = file.toLocalFile();

	QFile f(fileName);

	if (!f.open(QIODevice::WriteOnly)) {
		LOG_CWARNING("client") << "Can't write file:" << qPrintable(fileName);
		return;
	}

	f.write(createContent());

	f.close();

	setDownloaded(true);

	Application::instance()->messageInfo(tr("Sablon letöltve:\n").append(fileName), tr("Letöltés"));
}



/**
 * @brief MapEditorMatrixImporter::upload
 * @param file
 */

void MapEditorMatrixImporter::upload(const QUrl &file)
{
	if (!m_mapEditor) {
		Application::instance()->messageInfo(tr("MapEditor hiányzik"), tr("Belső hiba"));
		return;
	}

	const QString &fileName = file.toLocalFile();
	const auto &b = Utils::fileContent(fileName);

	if (!b)
		return Application::instance()->messageWarning(tr("Nem lehet megnyitni a fájlt:\n")+fileName, tr("Importálás"));

	LOG_CDEBUG("client") << "Import file:" << qPrintable(fileName);

	if (!loadContent(b.value()))
		return Application::instance()->messageWarning(tr("Nem lehet feldolgozni a fájlt:\n")+fileName, tr("Importálás"));

	emit imported();
}



/**
 * @brief MapEditorMatrixImporter::createContent
 * @return
 */

QByteArray MapEditorMatrixImporter::createContent() const
{
	Q_ASSERT(m_mapEditor);

	QXlsx::Document doc;

	QXlsx::Format fmtHeader;
	fmtHeader.setFontBold(true);
	fmtHeader.setHorizontalAlignment(QXlsx::Format::AlignHCenter);

	QXlsx::Format fmtCenter;
	fmtCenter.setHorizontalAlignment(QXlsx::Format::AlignHCenter);

	doc.write(1, dataCol, m_hash);			// B1 = hash

	doc.write(2, 5, QStringLiteral("Level"), fmtHeader);		// E2 = "Level"
	doc.write(4, 5, QStringLiteral("Count"), fmtCenter);		// E5 = "Count"

	doc.write(1, 3, tr("-- Új küldetés neve --"), fmtHeader);		// E2 = "Level"

	// Create sorted chapters

	std::vector<MapEditorChapter*> chapters;

	for (MapEditorChapter *ch : *m_mapEditor->map()->chapterList())
		chapters.push_back(ch);

	std::sort(chapters.begin(),
			  chapters.end(),
			  [](MapEditorChapter *ch1, MapEditorChapter *ch2) {
		if (ch1->name().isEmpty() && !ch2->name().isEmpty())
			return true;
		else if (!ch1->name().isEmpty() && ch2->name().isEmpty())
			return false;
		else if (ch1->name().isEmpty() && ch2->name().isEmpty())
			return true;
		else
			return ch1->name().localeAwareCompare(ch2->name()) < 0;

	});


	const int lastRow = startRow+chapters.size()-1;

	for (int i=0; i<LEVEL_COUNT; ++i) {				// G2:Z2 = "1", "2", "3",...
		doc.write(2, startCol+i, i+1, fmtHeader);

		QXlsx::CellFormula form(
					QStringLiteral("=SUMIF(%1$%2:%3$%4,\"X\",$%5$%6:$%7$%8)")
					.arg(columnName[startCol+i])
					.arg(startRow)
					.arg(columnName[startCol+i])
					.arg(lastRow)

					.arg(columnName[4])
					.arg(startRow)
					.arg(columnName[4])
					.arg(lastRow)
					);

		doc.currentWorksheet()->writeFormula(4, startCol+i, form, fmtCenter);
	}



	int i = 0;
	for (MapEditorChapter *ch : chapters) {
		doc.write(startRow+i, dataCol, ch->id());
		doc.write(startRow+i, 3, ch->name().isEmpty() ? tr("Chapter #%1").arg(ch->id()) : ch->name());
		doc.write(startRow+i, 4, ch->objectiveCount(), fmtCenter);

		QXlsx::CellFormula form(
					QStringLiteral("=COUNTIF(%1$%2:%3$%4,\"X\")")
					.arg(columnName[startCol])
					.arg(startRow+i)
					.arg(columnName[lastCol])
					.arg(startRow+i)
					);

		doc.currentWorksheet()->writeFormula(startRow+i, 5, form, fmtCenter);

		++i;
	}


	QXlsx::ConditionalFormatting cFmtCount;

	QXlsx::Format fmtUsed;
	fmtUsed.setFontBold(true);
	fmtUsed.setFontColor(QColorConstants::Svg::seashell);
	fmtUsed.setPatternBackgroundColor(QColorConstants::Svg::limegreen);


	cFmtCount.addHighlightCellsRule(QXlsx::ConditionalFormatting::Highlight_GreaterThan, QStringLiteral("0"), fmtUsed);
	cFmtCount.addRange(4, startCol, 4, lastCol);
	cFmtCount.addRange(startRow, 5, lastRow, 5);

	doc.addConditionalFormatting(cFmtCount);



	QXlsx::DataValidation validation(QXlsx::DataValidation::List, QXlsx::DataValidation::Between,
									 QStringLiteral("X"), QString(), true);

	validation.setErrorMessageVisible(true);
	validation.addRange(startRow, startCol, lastRow, lastCol);

	doc.addDataValidation(validation);



	QXlsx::ConditionalFormatting cFmtX;

	QXlsx::Format fmtX;
	fmtX.setFontBold(true);
	fmtX.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
	fmtX.setFontColor(QColorConstants::Svg::maroon);
	fmtX.setPatternBackgroundColor(QColorConstants::Svg::navajowhite);

	cFmtX.addHighlightCellsRule(QXlsx::ConditionalFormatting::Highlight_ContainsText, QStringLiteral("X"), fmtX);
	cFmtX.addRange(startRow, startCol, lastRow, lastCol);

	doc.addConditionalFormatting(cFmtX);



	const double baseWidth = 12.;

	doc.setColumnHidden(dataCol, true);

	doc.setColumnWidth(1, baseWidth*0.1);
	doc.setColumnWidth(startCol-1, baseWidth*0.1);
	doc.setColumnWidth(3, baseWidth*3);
	doc.setColumnWidth(4, baseWidth*0.6);
	doc.setColumnWidth(startCol, lastCol, baseWidth*0.4);


	QBuffer buf;
	doc.saveAs(&buf);

	return buf.data();
}





/**
 * @brief MapEditorMatrixImporter::loadContent
 * @param data
 * @return
 */

bool MapEditorMatrixImporter::loadContent(const QByteArray &data)
{
	Q_ASSERT(m_mapEditor);

	QBuffer buf;
	buf.setData(data);
	buf.open(QIODevice::ReadOnly);

	QXlsx::Document doc(&buf);

	buf.close();


	const QString hash = doc.read(1, 2).toString();
	const QString missionName = doc.read(1, 3).toString();

	if (hash != m_hash) {
		LOG_CERROR("client") << "Hash mismatch";
		return false;
	}


	const QXlsx::CellRange &range = doc.dimension();


	// Load chapters

	QHash<int, MapEditorChapter*> chapterHash;

	for (int i=startRow; i<=range.lastRow(); ++i) {
		const int id = doc.read(i, dataCol).toInt();

		MapEditorChapter *chapter = m_mapEditor->map()->chapter(id);

		if (!chapter) {
			LOG_CWARNING("client") << "Invalid chapter id" << id;
			continue;
		}

		chapterHash.insert(i, chapter);
	}


	if (chapterHash.isEmpty()) {
		LOG_CWARNING("client") << "Missing chapters";
		return false;
	}

	// Load levels

	QList<QSet<MapEditorChapter*>> levels;

	for (int i=startCol; i<=range.lastColumn(); ++i) {
		QSet<MapEditorChapter*> chlist;

		for (const auto &[id, chapter] : chapterHash.asKeyValueRange()) {
			const QString data = doc.read(id, i).toString();

			if (data == 'X' || data == 'x') {
				chlist.insert(chapter);
			}
		}

		if (!chlist.empty()) {
			levels.append(chlist);
		} else {
			break;
		}
	}

	if (levels.isEmpty()) {
		LOG_CWARNING("client") << "Missing levels";
		return false;
	}

	LOG_CDEBUG("client") << "Loaded" << levels.size() << "levels";

	m_mapEditor->missionAddWithLevels(levels, missionName);

	return true;
}






bool MapEditorMatrixImporter::downloaded() const
{
	return m_downloaded;
}

void MapEditorMatrixImporter::setDownloaded(bool newDownloaded)
{
	if (m_downloaded == newDownloaded)
		return;
	m_downloaded = newDownloaded;
	emit downloadedChanged();
}
