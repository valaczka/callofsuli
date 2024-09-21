import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Item {
	id: root

	property double indent: Qaterial.Style.delegate.iconWidth

	default property alias _contentData: _content.data

	implicitWidth: _content.childrenRect.x+_content.childrenRect.width
	implicitHeight: _content.childrenRect.y+_content.childrenRect.height

	Item {
		id: _content
		x: root.indent
		y: 0
		width: parent.width-root.indent
		height: parent.height
	}
}
