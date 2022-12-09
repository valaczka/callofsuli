/*
 * ---- Call of Suli ----
 *
 * fillouthighlighter.h
 *
 * Created on: 2021. 11. 07.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * FilloutHighlighter
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

#ifndef FILLOUTHIGHLIGHTER_H
#define FILLOUTHIGHLIGHTER_H

#include <QObject>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QQuickTextDocument>

class FilloutSyntaxHighlighter;

class FilloutHighlighter : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QQuickTextDocument * document READ document WRITE setDocument NOTIFY documentChanged)

	Q_PROPERTY(QColor wordForeground READ wordForeground WRITE setWordForeground NOTIFY wordForegroundChanged)
	Q_PROPERTY(QColor wordBackground READ wordBackground WRITE setWordBackground NOTIFY wordBackgroundChanged)

public:
	explicit FilloutHighlighter(QObject *parent = nullptr);

	const QColor &wordForeground() const;
	void setWordForeground(const QColor &newWordForeground);

	const QColor &wordBackground() const;
	void setWordBackground(const QColor &newWordBackground);

	QQuickTextDocument *document() const;
	void setDocument(QQuickTextDocument *newDocument);

signals:
	void wordForegroundChanged();
	void wordBackgroundChanged();

	void documentChanged();

private:
	friend class FilloutSyntaxHighlighter;

	FilloutSyntaxHighlighter *m_syntaxHighlighter;

	QTextCharFormat m_fmtWord;
	QQuickTextDocument *m_document;
};



/**
 * @brief The FilloutHighlighterPrivate class
 */

class FilloutSyntaxHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT

public:
	explicit FilloutSyntaxHighlighter(FilloutHighlighter *parent = nullptr);

protected:
	void highlightBlock(const QString &text) override;

private:
	FilloutHighlighter *m_highlighter;
};

#endif // FILLOUTHIGHLIGHTER_H
