import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

Item {
	id: control

	property User user: null
	readonly property real size: Math.min(control.width, control.height)
	property color iconColor: Qaterial.Style.primaryColor
	property color sublevelColor: Qaterial.Style.iconColor()

	Qaterial.Icon {
		visible: !user || user.rank.level < 0
		anchors.centerIn: parent
		icon: Qaterial.Icons.account
		color: control.iconColor
		size: control.size
	}

	Rectangle {
		id: _picture

		visible: user && user.picture.length

		property real contentSize: control.size-(2*roundBorderWidth)
		property int roundBorderWidth: 1

		anchors.centerIn: parent
		width: img.width+2*roundBorderWidth
		height: width
		radius: width/2
		color: Qaterial.Style.iconColor()
		Qaterial.RoundImage {
			id: img
			anchors.centerIn: parent
			width: _picture.contentSize
			height: _picture.contentSize
			source: user ? user.picture : ""
			asynchronous: true
		}

		Image {
			id: _rankSmall
			anchors.right: img.right
			anchors.bottom: img.bottom
			visible: user && user.rank.level >= 0
			width: control.size*0.5
			height: control.size*0.5
			source: user && user.rank.level >= 0 ? "qrc:/internal/rank/"+user.rank.level+".svg" : ""
		}
	}


	Image {
		id: _rank
		anchors.centerIn: parent
		visible: user && user.rank.level >= 0 && !_picture.visible
		width: control.size
		height: control.size
		source: user && user.rank.level >= 0 ? "qrc:/internal/rank/"+user.rank.level+".svg" : ""
	}

	Label {
		visible: user && user.rank.sublevel > 0 && !_picture.visible
		font.family: Qaterial.Style.textTheme.overline.family
		font.pixelSize: control.size*0.5
		font.weight: Font.Bold
		color: control.sublevelColor
		style: Text.Outline
		styleColor: Qaterial.Colors.black
		anchors.right: _rank.right
		anchors.bottom: _rank.bottom
		height: font.pixelSize
		text: user ? user.rank.sublevel : ""
	}

}
