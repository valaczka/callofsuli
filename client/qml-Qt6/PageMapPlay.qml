import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

QPageGradient {
	id: control

	progressBarEnabled: true

	appBar.backButtonVisible: StackView.index > 1

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


				QIconLabel {
					topPadding: 20 * Qaterial.Style.pixelSizeRatio
					bottomPadding: 20 * Qaterial.Style.pixelSizeRatio
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
					icon.width: 1.5 * font.pixelSize
					icon.height: 1.5 * font.pixelSize
					wrapMode: Text.Wrap
				}



				ListView {
					id: view
					model: mission.missionLevelList
					orientation: ListView.Horizontal

					implicitHeight: 110*Qaterial.Style.pixelSizeRatio

					width: control.width
					spacing: 5 * Qaterial.Style.pixelSizeRatio

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

						readOnly: map && map.readOnly

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

	StackView.onActivated: {
		if (map)
			map.gameState = MapPlay.StateSelect
	}

}
