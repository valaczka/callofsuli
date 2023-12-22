import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import "JScript.js" as JS

QItemGradient {
	id: root

	property StudentMapHandler studentMapHandler: null
	property StudentGroupList groupList: Client.cache("studentGroupList")

	QScrollable {
		anchors.fill: parent
		spacing: 15

		refreshEnabled: true

		onRefreshRequest: reload()

		Qaterial.LabelHeadline5 {
			width: parent.width
			topPadding: 50+root.paddingTop
			leftPadding: 50
			rightPadding: 50
			horizontalAlignment: Qt.AlignHCenter
			text: qsTr("Csoportjaim")
		}


		QDashboardGrid {
			id: groupsStudentGrid
			anchors.horizontalCenter: parent.horizontalCenter

			visible: groupList.count
			contentItems: groupList.count

			Repeater {
				model: SortFilterProxyModel {
					sourceModel: groupList
					sorters: [
						StringSorter {
							roleName: "name"
							sortOrder: Qt.AscendingOrder
						}
					]
				}

				QDashboardButton {
					property StudentGroup group: model.qtObject
					text: group ? group.name : ""
					icon.source: Qaterial.Icons.accountGroupOutline
					highlighted: true

					onClicked: Client.stackPushPage("PageStudentGroup.qml", {
														//user: root.user,
														group: group,
														mapHandler: root.studentMapHandler
													})

				}
			}
		}

	}


	StackView.onActivated: reload()

	function reload() {
		Client.reloadCache("studentGroupList")
	}

}


