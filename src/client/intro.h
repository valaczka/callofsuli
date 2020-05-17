/*
 * ---- Call of Suli ----
 *
 * intro.h
 *
 * Created on: 2020. 05. 11.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Intro
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef INTRO_H
#define INTRO_H

#include <QObject>

class Intro : public QObject
{
	Q_OBJECT

	Q_PROPERTY(IntroType type READ type WRITE setType NOTIFY typeChanged)
	Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
	Q_PROPERTY(QString image READ image WRITE setImage NOTIFY imageChanged)
	Q_PROPERTY(QString media READ media WRITE setMedia NOTIFY mediaChanged)
	Q_PROPERTY(int seconds READ seconds WRITE setSeconds NOTIFY secondsChanged)

public:
	enum IntroType {
		IntroIntro,
		IntroOutro
	};
	Q_ENUM(IntroType)

	Intro(QObject *parent = nullptr);

	IntroType type() const { return m_type; }
	QString text() const { return m_text; }
	QString image() const { return m_image; }
	QString media() const { return m_media; }
	int seconds() const { return m_seconds; }

public slots:
	void setType(IntroType type);
	void setText(QString text);
	void setImage(QString image);
	void setMedia(QString media);
	void setSeconds(int seconds);

signals:
	void typeChanged(IntroType type);
	void textChanged(QString text);
	void imageChanged(QString image);
	void mediaChanged(QString media);
	void secondsChanged(int seconds);

private:
	IntroType m_type;
	QString m_text;
	QString m_image;
	QString m_media;
	int m_seconds;
};

#endif // INTRO_H
