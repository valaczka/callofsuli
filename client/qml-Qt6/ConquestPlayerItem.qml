import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


Rectangle {
	id: root

	implicitHeight: 100
	implicitWidth: 200

	property int playerId: -1
	property string username: ""
	property color theme: Qaterial.Colors.white
	property int xp: 0

	property alias playerItem: _playerItem
	property alias placeholder: _placeholder

	color: "black"

	Item {
		id: _placeholder
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		width: height

		Rectangle {
			id: _playerItem
			color: root.theme
			width: _placeholder.width*0.7
			height: _placeholder.height*0.7
			x: (parent.width-width)/2
			y: (parent.height-height)/2
		}

	}

	Column {
		anchors.left: _placeholder.right
		anchors.right: parent.right
		anchors.top: parent.top
		anchors.bottom: parent.bottom

		Qaterial.LabelCaption {
			id: _labelName

			width: parent.width
			wrapMode: Text.Wrap
			text: username
		}

		Qaterial.LabelCaption {
			id: _labelXP

			width: parent.width

			text: "%1 XP".arg(xp)
		}
	}

}
