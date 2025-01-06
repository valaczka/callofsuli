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

#include "rpgconfig.h"
#include <QVector2D>

class RpgGameDataInterface
{
public:
	RpgGameDataInterface() {}

	virtual std::unique_ptr<RpgGameData::Body> serialize() const = 0;

	template <typename T, typename = std::enable_if<std::is_base_of<RpgGameData::Body, T>::value>::type>
	std::optional<T> serialize() const {
		const auto &ptr = serialize();

		T* p = dynamic_cast<T*>(ptr.get());

		if (!p)
			return std::nullopt;

		return *p;
	}

	virtual bool deserialize(const RpgGameData::Body *from) const = 0;

	bool keyFrameRequired() const;
	void setKeyFrameRequired(bool newKeyFrameRequired);

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
