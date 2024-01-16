import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

QIconLoaderItemDelegate {
	id: root

	property MapEditorObjective objective: null
	readonly property MapEditor editor: objective && objective.map ? objective.map.mapEditor : null
	property var _info: editor ? editor.objectiveInfo(objective) : {}
	property bool isExam: false

	signal menuRequest(Item button)

	iconSource: _info.icon !== undefined ? _info.icon : ""
	text: _info.title !== undefined ? _info.title: ""
	secondaryText: _info.details !== undefined ? _info.details : ""

	textColor: Qaterial.Style.iconColor()
	iconColor: textColor

	Connections {
		target: objective

		function onDataChanged() {
			if (editor)
				_info = editor.objectiveInfo(objective)
		}
	}

	Connections {
		target: objective ? objective.storage : null

		function onDataChanged() {
			if (editor)
				_info = editor.objectiveInfo(objective)
		}
	}

	selectableObject: objective

	rightSourceComponent: Row {
		spacing: 3

		Qaterial.RoundColorIcon
		{
			anchors.verticalCenter: parent.verticalCenter
			iconSize: Qaterial.Style.delegate.iconWidth

			fill: true
			width: roundIcon ? roundSize : iconSize
			height: roundIcon ? roundSize : iconSize

			visible: source != ""
			source: editor && objective && objective.storage ? editor.storageInfo(objective.storage).icon : ""

			color: Qaterial.Colors.green400
		}

		QBanner {
			num: objective ? objective.storageCount : 0
			visible: num > 0
			color: Qaterial.Colors.cyan900
			anchors.verticalCenter: parent.verticalCenter
		}



		Qaterial.RoundButton {
			visible: root.isExam
			icon.source: Qaterial.Icons.minus
			icon.color: Qaterial.Colors.blue700
			anchors.verticalCenter: parent.verticalCenter
			enabled: objective && objective.examPoint > 0
			onClicked: editor.objectiveModify(objective, null, function() {
				objective.examPoint--
			})
		}

		QBanner {
			visible: root.isExam
			num: objective ? objective.examPoint : 0
			//color: Qaterial.Colors.blue600
			color: "transparent"
			border.color: Qaterial.Colors.blue600
			border.width: 1
			textColor: Qaterial.Colors.blue400
			pixelSize: Qaterial.Style.textTheme.body1.pixelSize
			anchors.verticalCenter: parent.verticalCenter
		}

		Qaterial.RoundButton {
			visible: root.isExam
			icon.source: Qaterial.Icons.plus
			icon.color: Qaterial.Colors.blue700
			anchors.verticalCenter: parent.verticalCenter
			enabled: objective && objective.examPoint < 100
			onClicked: editor.objectiveModify(objective, null, function() {
				objective.examPoint++
			})
		}




		Qaterial.RoundButton {
			id: _btn
			icon.source: Qaterial.Icons.dotsVertical
			icon.color: Qaterial.Style.iconColor()
			anchors.verticalCenter: parent.verticalCenter
			onClicked: menuRequest(_btn)
		}


	}
}
