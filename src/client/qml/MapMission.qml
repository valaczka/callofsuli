import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPagePanel {
	id: panel

	property StudentMap studentMap: null
	property var missionData: null

	title: missionData ? missionData.name : ""
	icon: CosStyle.iconUsers

	QLabel {
		id: noLabel

		anchors.centerIn: parent
		opacity: missionData == null
		visible: opacity != 0

		text: qsTr("Válassz küldetést")

		Behavior on opacity { NumberAnimation { duration: 125 } }
	}


	Column {
		anchors.centerIn: parent
		opacity: missionData != null
		visible: opacity != 0

		Behavior on opacity { NumberAnimation { duration: 125 } }

		Repeater {
			id: btnRepeater

			delegate: QButton {
				text: "Level "+level

				onClicked: pageStudentMap.playGame(missionData.id, missionData.type === "summary", level)
			}
		}
	}


	Connections {
		target: studentMap
		onCampaignListChanged: missionData = null
	}

	Connections {
		target: pageStudentMap
		onMissionDataSelected: missionData = mdata
	}


	onMissionDataChanged: if (missionData)
							  btnRepeater.model = missionData.levels
						  else
							  btnRepeater.model = null
}




