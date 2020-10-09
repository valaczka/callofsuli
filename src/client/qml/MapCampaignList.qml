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

	title: qsTr("Hadjáratok")
	icon: CosStyle.iconUsers
	maximumWidth: 600


	ListModel {
		id: baseCampaignModel
	}

	SortFilterProxyModel {
		id: campaignProxyModel
		sourceModel: baseCampaignModel

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
		id: campaignList

		anchors.fill: parent

		model: campaignProxyModel
		isProxyModel: true
		modelTitleRole: "name"
		//modelSubtitleRole: "details"
		modelTitleColorRole: "textColor"
		modelSubtitleColorRole: "textColor"
		modelBackgroundRole: "bgColor"

		//delegateHeight: CosStyle.twoLineHeight

		onClicked: {
			var m = model.get(index)
			pageStudentMap.campaignSelected(m.id)
		}

		refreshEnabled: true
		onRefreshRequest: reloadModel()

	}

	onPanelActivated: {
		reloadModel()
		campaignList.forceActiveFocus()
	}


	Connections {
		target: studentMap
		onMapResultUpdated: reloadModel()
		onCampaignListChanged: JS.setModel(baseCampaignModel, studentMap.campaignList)
	}

	Connections {
		target: pageStudentMap
		onPageActivated: {
			reloadModel()
			campaignList.forceActiveFocus()
		}
	}


	function reloadModel() {
		studentMap.campaignListUpdate()
	}
}




