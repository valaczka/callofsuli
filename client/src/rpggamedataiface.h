/*
 * ---- Call of Suli ----
 *
 * rpggamedataiface.h
 *
 * Created on: 2025. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
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

#ifndef RPGGAMEDATAIFACE_H
#define RPGGAMEDATAIFACE_H

#include <QVector2D>
#include <QCborMap>

#define ADD_SERIALIZATION_OVERRIDE		\
	virtual QCborMap serialize(const qint64 &tick = -1) const override { \
		auto p = serializeThis(); \
		p.f = tick; \
		return p.toCborMap(true); \
	} \
	virtual QCborMap serialize(const QCborMap &other, const qint64 &tick = -1) const override { \
		auto p = serializeThis(); \
		p.f = other.value(QStringLiteral("f")).toInteger(-1); \
		if (p.toCborMap(other, true).isEmpty()) \
			return {}; \
		p.f = tick; \
		return p.toCborMap(true); \
	}




/**
 * @brief The RpgGameDataInterface class
 */

class RpgGameDataInterface
{
public:
	RpgGameDataInterface() {}

	bool keyFrameRequired() const;
	void setKeyFrameRequired(bool newKeyFrameRequired);

	virtual QCborMap serialize(const qint64 &tick = -1) const { Q_UNUSED(tick); return QCborMap(); }
	virtual QCborMap serialize(const QCborMap &other, const qint64 &tick = -1) const { Q_UNUSED(other); Q_UNUSED(tick); return QCborMap(); }

protected:
	static QList<float> toPosList(const QVector2D &pos) { return { pos.x(), pos.y() }; }
	static QList<float> toPosList(const QPointF &pos) { return { (float) pos.x(), (float) pos.y() }; }

	bool m_keyFrameRequired = false;
};


inline bool RpgGameDataInterface::keyFrameRequired() const
{
	return m_keyFrameRequired;
}

inline void RpgGameDataInterface::setKeyFrameRequired(bool newKeyFrameRequired)
{
	m_keyFrameRequired = newKeyFrameRequired;
}

#endif // RPGGAMEDATAIFACE_H
