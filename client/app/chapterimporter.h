/*
 * ---- Call of Suli ----
 *
 * chapterimporter.h
 *
 * Created on: 2021. 01. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ChapterImporter
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

#ifndef CHAPTERIMPORTER_H
#define CHAPTERIMPORTER_H

#include <QString>
#include <QVariantList>

class ChapterImporter
{
public:
	ChapterImporter(const QString &filename);

	bool import();

	int chapterId() const;
	void setChapterId(int chapterId);

	QVariantList records() const;

private:
	QString m_filename;
	int m_chapterId;
	QVariantList m_records;
};

#endif // CHAPTERIMPORTER_H
