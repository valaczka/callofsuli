import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property Teacher teacher: null
	property int mapId: -1
	property string mapName: ""


	title: qsTr("Lehetőségek")
	maximumWidth: 600

	Item {
		id: selectItem
		anchors.fill: parent

		visible: mapId == -1

		QLabel {
			anchors.centerIn: parent
			text: qsTr("Válassz pályát")
		}
	}

	QLabel {
		id: mapNameLabel
		anchors.top: parent.top
		width: parent.width
		padding: 5
		topPadding: 10
		bottomPadding: 10
		elide: Text.ElideRight
		font.pixelSize: CosStyle.pixelSize*1.5
		font.weight: Font.Thin
		horizontalAlignment: Text.AlignHCenter
		verticalAlignment: Text.AlignVCenter

		text: mapName

		visible: text.length
	}

	QListItemDelegate {
		id: list
		anchors.top: mapNameLabel.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		modelTitleRole: "title"

		visible:  mapId != -1

		model: ListModel {
			ListElement {
				title: qsTr("Szerkesztés")
				cmd: "edit"
			}
			ListElement {
				title: qsTr("Members")
				cmd: "members"
			}
			ListElement {
				title: qsTr("Exportálás")
				cmd: "export"
			}
		}

		onClicked: {
			var m = model.get(index)
			if (m.cmd === "edit") {
				teacher.send({"class": "teacherMaps", "func": "getMapData", "id": mapId })
			}
		}
	}


	Connections {
		target: pageTeacherMap
		onMapSelected: {
			mapId = id
			mapName = name
		}
	}

	onPanelActivated: list.forceActiveFocus()
}
