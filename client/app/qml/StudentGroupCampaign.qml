import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
	id: control

	title: qsTr("Hadjáratok")
	icon: "qrc:/internal/img/battle.png"
	menu: QMenu {
		MenuItem { action: actionList }
	}

	ListModel {
		id: modelCampaign
	}

	ListModel {
		id: modelCampaignList
	}

	SortFilterProxyModel {
		id: userProxyModel
		sourceModel: modelCampaign
		sorters: [
			StringSorter { roleName: "starttime"; sortOrder: Qt.DescendingOrder; priority: 1 },
			StringSorter { roleName: "endtime"; priority: 0 }
		]

		proxyRoles: [
			ExpressionRole {
				name: "title"
				expression: model.description === "" ? qsTr("Hadjárat •%1").arg(model.id) : model.description
			}
		]
	}

	SortFilterProxyModel {
		id: listProxyModel
		sourceModel: modelCampaignList
		sorters: [
			StringSorter { roleName: "starttime"; sortOrder: Qt.DescendingOrder; priority: 1 },
			StringSorter { roleName: "endtime"; priority: 0 }
		]

		proxyRoles: [
			ExpressionRole {
				name: "title"
				expression: model.description === "" ? qsTr("Hadjárat •%1").arg(model.id) : model.description
			}
		]
	}



	QIconEmpty {
		visible: userProxyModel.count == 0
		anchors.centerIn: parent
		textWidth: parent.width*0.75
		text: qsTr("Jelenleg egyetlen hadjárat sincs ebben a csoportban")
		tabContainer: control
	}


	QAccordion {
		QTabHeader {
			tabContainer: control
			isPlaceholder: true
		}

		Repeater {
			model: userProxyModel

			Campaign {
				width: parent.width
			}
		}

	}

	Connections {
		target: studentMaps

		function onCampaignGetReady(list) {
			modelCampaign.clear()
			for (var i=0; i<list.length; i++) {
				var o = list[i]
				o.subtitle = JS.readableInterval(o.starttime, o.endtime)
				modelCampaign.append(o)
			}
		}

		function onCampaignListGet(jsonData, binaryData) {
			modelCampaignList.clear()

			for (var i=0; i<jsonData.list.length; i++) {
				var o = jsonData.list[i]
				o.subtitle = JS.readableInterval(o.starttime, o.endtime)
				modelCampaignList.append(o)
			}

			var d = JS.dialogCreateQml("List", {
										   icon: control.icon,
										   title: qsTr("Hadjárat kiválasztása"),
										   selectorSet: false,
										   modelTitleRole: "title",
										   modelSubtitleRole: "subtitle",
										   delegateHeight: CosStyle.twoLineHeight,
										   imageWidth: 0,
										   model: listProxyModel
									   })


			d.accepted.connect(function(data) {
				if (!data)
					return

				studentMaps.send("campaignGet", {groupid: studentMaps.selectedGroupId, id: data.id})
			})
			d.open()
		}
	}


	Action {
		id: actionList

		text: qsTr("Korábbi hadjáratok")

		onTriggered: {
			studentMaps.send("campaignListGet", {groupid: studentMaps.selectedGroupId})
		}
	}

	onPopulated: {
		if (studentMaps.selectedGroupId > -1)
			studentMaps.send("campaignGet", {groupid: studentMaps.selectedGroupId})
	}

}



