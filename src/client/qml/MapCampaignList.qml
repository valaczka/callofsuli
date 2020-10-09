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

	title: qsTr("Küldetések")
	icon: CosStyle.iconUsers
	maximumWidth: 600


	ListModel {
		id: baseMissionModel
	}

	SortFilterProxyModel {
		id: missionProxyModel
		sourceModel: baseMissionModel

		proxyRoles: [
			/*ExpressionRole {
				name: "details"
				expression: model.levels //join(", ")
			},*/
			SwitchRole {
				name: "textColor"
				filters: ValueFilter {
					roleName: "type"
					value: "summary"
					SwitchRole.value: CosStyle.colorErrorDarker
				}
				defaultValue: "black"
			},

			SwitchRole {
				name: "bgColor"
				filters: [
					ValueFilter {
						roleName: "locked"
						value: "true"
						SwitchRole.value: "transparent"
					},
					ValueFilter {
						roleName: "type"
						value: "campaign"
						SwitchRole.value: CosStyle.colorAccent
					}
				]
				defaultValue: CosStyle.colorPrimary
			}
		]
	}


	QListItemDelegate {
		id: missionList

		anchors.fill: parent

		model: missionProxyModel
		isProxyModel: true
		modelTitleRole: "name"
		//modelSubtitleRole: "details"
		modelTitleColorRole: "textColor"
		modelSubtitleColorRole: "textColor"
		modelBackgroundRole: "bgColor"

		//delegateHeight: CosStyle.twoLineHeight

		onClicked: {
					   var m = model.get(index)
					   console.debug(m.type, m.locked, m.levels)
				   }

		refreshEnabled: true
		onRefreshRequest: reloadModel()

	}

	onPanelActivated: {
		reloadModel()
		missionList.forceActiveFocus()
	}


	Connections {
		target: studentMap
		onMapResultUpdated: reloadModel()
	}

	Connections {
		target: pageStudentMap
		onPageActivated: {
			reloadModel()
			missionList.forceActiveFocus()
		}
	}


	function reloadModel() {
		JS.setModel(baseMissionModel, studentMap.missionList())
	}
}




