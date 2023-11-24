/*
 * ---- Call of Suli ----
 *
 * fillouthighlighter.cpp
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

#include "fillouthighlighter.h"
#include <QRegularExpressionMatchIterator>
#include "modulefillout.h"


FilloutHighlighter::FilloutHighlighter(QObject *parent)
	: QObject(parent)
	, m_syntaxHighlighter(new FilloutSyntaxHighlighter(this))
{

}



/**
 * @brief FilloutSyntaxHighlighter::FilloutSyntaxHighlighter
 * @param parent
 */

FilloutSyntaxHighlighter::FilloutSyntaxHighlighter(FilloutHighlighter *parent)
	: QSyntaxHighlighter(parent)
	, m_highlighter(parent)
{

}



/**
 * @brief FilloutSyntaxHighlighter::highlightBlock
 * @param text
 */

void FilloutSyntaxHighlighter::highlightBlock(const QString &text)
{
	QRegularExpressionMatchIterator i = ModuleFillout::expressionWord().globalMatch(text);

	while (i.hasNext())
	{
		QRegularExpressionMatch match = i.next();
		setFormat(match.capturedStart(1)-1, match.capturedLength(1)+2, m_highlighter->m_fmtWord);
	}

}


/**
 * @brief FilloutHighlighter::wordForeground
 * @return
 */

const QColor &FilloutHighlighter::wordForeground() const
{
	return m_fmtWord.foreground().color();
}


/**
 * @brief FilloutHighlighter::setWordForeground
 * @param newWordForeground
 */

void FilloutHighlighter::setWordForeground(const QColor &newWordForeground)
{
	m_fmtWord.setForeground(newWordForeground);
	emit wordForegroundChanged();
}


/**
 * @brief FilloutHighlighter::wordBackground
 * @return
 */

const QColor &FilloutHighlighter::wordBackground() const
{
	return m_fmtWord.background().color();
}


/**
 * @brief FilloutHighlighter::setWordBackground
 * @param newWordBackground
 */

void FilloutHighlighter::setWordBackground(const QColor &newWordBackground)
{
	m_fmtWord.setBackground(newWordBackground);
	emit wordBackgroundChanged();
}

/**
 * @brief FilloutHighlighter::document
 * @return
 */

QQuickTextDocument *FilloutHighlighter::document() const
{
	return m_document;
}


/**
 * @brief FilloutHighlighter::setDocument
 * @param newDocument
 */

void FilloutHighlighter::setDocument(QQuickTextDocument *newDocument)
{
	if (m_document == newDocument)
		return;
	m_document = newDocument;
	if (m_document)
		m_syntaxHighlighter->setDocument(m_document->textDocument());
	emit documentChanged();
}
