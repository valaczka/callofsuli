import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPagePanel {
	id: panel

	maximumWidth: 600

	property StudentMap studentMap: null
	property int campaignId: -1

	title: qsTr("Küldetések")
	icon: CosStyle.iconUsers

	QLabel {
		id: noLabel

		anchors.centerIn: parent
		opacity: campaignId == -1
		visible: opacity != 0

		text: qsTr("Válassz hadjáratot")

		Behavior on opacity { NumberAnimation { duration: 125 } }
	}

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

		opacity: campaignId != -1
		visible: opacity != 0

		Behavior on opacity { NumberAnimation { duration: 125 } }

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
					   console.debug(m.type, m.locked, m.levels, m.uuid)
					   pageStudentMap.missionDataSelected(m)
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
		onCampaignListChanged: campaignId = -1
	}

	Connections {
		target: pageStudentMap
		onPageActivated: {
			reloadModel()
			missionList.forceActiveFocus()
		}
		onCampaignSelected: campaignId = cid
	}


	onCampaignIdChanged: reloadModel()


	function reloadModel() {
		JS.setModel(baseMissionModel, studentMap.missionListGet(campaignId))
	}
}




