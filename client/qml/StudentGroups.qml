import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import "JScript.js" as JS

QItemGradient {
	id: root

	//property User user: null
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
					icon.source: Qaterial.Icons.group
					highlighted: true
				}
			}
		}

	}


	StackView.onActivated: reload()

	function reload() {
		Client.reloadCache("studentGroupList")
	}

}


