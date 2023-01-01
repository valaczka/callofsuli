import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

QScrollablePage {
	id: control

	contentBottomPadding: 20

	property MapPlay map: null

	property SortFilterProxyModel mList: SortFilterProxyModel {
		sourceModel: map ? map.missionList : []

		sorters: StringSorter { roleName: "name"; priority: 0 ; sortOrder: Qt.AscendingOrder }
	}

	Column {
		id: col
		width: parent.width

		Repeater {
			model: mList

			Column {
				id: missionColumn

				property QtObject mission: model.qtObject

				Qaterial.LabelHeadline5 {
					topPadding: 20
					bottomPadding: 10
					leftPadding: Math.max(Client.safeMarginLeft, Qaterial.Style.card.horizontalPadding)
					rightPadding: Math.max(Client.safeMarginRight, Qaterial.Style.card.horizontalPadding)
					text: mission.name
				}

				ListView {
					id: view
					model: mission.missionLevelList
					orientation: ListView.Horizontal

					implicitHeight: 150*Qaterial.Style.pixelSizeRatio

					width: control.width
					spacing: 5

					clip: true

					header: Item {
						width: Math.max(Client.safeMarginLeft, Qaterial.Style.card.horizontalPadding)
						height: view.height
					}

					footer: Item {
						width: Math.max(Client.safeMarginRight, Qaterial.Style.card.horizontalPadding)
						height: view.height
					}

					delegate: MapPlayMissionLevelCard {
						height: view.implicitHeight
						width: view.implicitHeight

						missionLevel: model.qtObject

						onClicked: map.play(item)
					}

				}

			}
		}


	}



}
