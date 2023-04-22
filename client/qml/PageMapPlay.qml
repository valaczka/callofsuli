import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

QPageGradient {
	id: control

	property MapPlay map: null

	property SortFilterProxyModel mList: SortFilterProxyModel {
		sourceModel: map ? map.missionList : []

		filters: ValueFilter {
			roleName: "lockDepth"
			value: -1
			inverted: true
		}


		sorters: [
			RoleSorter {
				roleName: "lockDepth"
				priority: 1
				sortOrder: Qt.AscendingOrder
			},

			StringSorter {
				roleName: "name"
				priority: 0
				sortOrder: Qt.AscendingOrder
			}
		]
	}

	QScrollable {
		anchors.fill: parent
		horizontalPadding: 0
		verticalPadding: 0
		leftPadding: 0
		rightPadding: 0

		refreshEnabled: true
		onRefreshRequest: if (map)
							  map.updateSolver()


		Item {
			width: parent.width
			height: control.paddingTop
		}

		Repeater {
			model: mList

			Column {
				id: missionColumn

				property MapPlayMission mission: model.qtObject

				Item {
					width: parent.width
					height: 20
				}

				Qaterial.IconLabel {
					//topPadding: 20
					//bottomPadding: 10
					anchors.left: parent.left
					anchors.leftMargin: Math.max(Client.safeMarginLeft, Qaterial.Style.card.horizontalPadding)
					width: parent.width
						   -Math.max(Client.safeMarginLeft, Qaterial.Style.card.horizontalPadding)
						   -Math.max(Client.safeMarginRight, Qaterial.Style.card.horizontalPadding)
					horizontalAlignment: Qt.AlignLeft
					font: Qaterial.Style.textTheme.headline5
					text: mission.name
					icon.source: mission.medalImage
					icon.color: "transparent"
					icon.width: 32 * Qaterial.Style.pixelSizeRatio
					icon.height: 32 * Qaterial.Style.pixelSizeRatio
					wrapMode: Text.Wrap
				}

				Item {
					width: parent.width
					height: 10
				}

				ListView {
					id: view
					model: mission.missionLevelList
					orientation: ListView.Horizontal

					implicitHeight: 110*Qaterial.Style.pixelSizeRatio

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

						onClicked: Client.stackPushPage("PageMapPlayMissionLevel.qml", {
															mission: missionColumn.mission,
															map: control.map,
															missionLevel: item
														})
					}

				}

			}
		}

	}

}
