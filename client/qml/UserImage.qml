import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

Item {
	id: control

	property User user: null
	property var userData: null
	property real size: Math.min(control.width, control.height)
	property color iconColor: Qaterial.Style.primaryColor
	property color sublevelColor: Qaterial.Style.iconColor()
	property bool pictureEnabled: true
	property bool sublevelEnabled: true

	implicitWidth: Math.max(_icon.implicitWidth, _rank.implicitWidth)
	implicitHeight: Math.max(_icon.implicitHeight, _rank.implicitHeight)

	readonly property int _u_rankLevel: user ? user.rank.level : userData ? userData.rank.level : -1
	readonly property int _u_rankSubLevel: user ? user.rank.sublevel : userData ? userData.rank.sublevel : -1
	readonly property string _u_picture: user ? user.picture : userData && userData.picture !== undefined ? userData.picture : ""

	Qaterial.Icon {
		id: _icon
		visible: _u_rankLevel < 0
		anchors.centerIn: parent
		icon: Qaterial.Icons.account
		color: control.iconColor
		width: control.size
		height: control.size
	}

	Rectangle {
		id: _picture

		visible: _u_picture.length && pictureEnabled

		property real contentSize: control.size-(2*roundBorderWidth)
		property int roundBorderWidth: 1

		anchors.centerIn: parent
		width: img.width+2*roundBorderWidth
		height: width
		radius: width/2
		color: Qaterial.Style.primaryColor
		Qaterial.RoundImage {
			id: img
			anchors.centerIn: parent
			width: _picture.contentSize
			height: _picture.contentSize
			source: _u_picture
			asynchronous: true
		}

		Image {
			id: _rankSmall
			anchors.right: img.right
			anchors.bottom: img.bottom
			visible: _u_rankLevel >= 0
			width: control.size*0.5
			height: control.size*0.5
			source: _u_rankLevel >= 0 ? "qrc:/internal/rank/"+_u_rankLevel+".svg" : ""
			asynchronous: true
		}
	}


	Image {
		id: _rank
		anchors.centerIn: parent
		visible: _u_rankLevel >= 0 && !_picture.visible
		width: control.size
		height: control.size
		source: _u_rankLevel >= 0 ? "qrc:/internal/rank/"+_u_rankLevel+".svg" : ""
		fillMode: Image.PreserveAspectFit
		asynchronous: true
	}

	Label {
		visible: _u_rankSubLevel > 0 && !_picture.visible && control.sublevelEnabled
		font.family: Qaterial.Style.textTheme.overline.family
		font.pixelSize: control.size*0.5
		font.weight: Font.Bold
		color: control.sublevelColor
		style: Text.Outline
		styleColor: Qaterial.Colors.black
		anchors.right: _rank.right
		anchors.bottom: _rank.bottom
		height: font.pixelSize
		text: _u_rankSubLevel
	}

}
