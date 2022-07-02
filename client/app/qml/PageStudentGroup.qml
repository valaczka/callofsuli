import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QTabPage {
	id: control

	property alias groupId: studentMaps.selectedGroupId

	property Profile profile: null

	//buttonColor: CosStyle.colorPrimary
	buttonBackgroundColor: Qt.darker("#006400")

	buttonModel: ListModel {
		id: modelGuest

		ListElement {
			title: qsTr("Hadjáratok")
			icon: "qrc:/internal/img/battle.png"
			iconColor: "firebrick"
			func: function() { replaceContent(cmpStudentCampaign) }
		}
		ListElement {
			title: qsTr("Pályák")
			icon: "image://font/School/\uf19d"
			iconColor: "lime"
			func: function() { replaceContent(cmpStudentMapList) }
			checked: true
		}
		ListElement {
			title: qsTr("Résztvevők")
			icon: "image://font/School/\uf154"
			iconColor: "orchid"
			func: function() { replaceContent(cmpStudentMemberList) }
		}
		ListElement {
			title: qsTr("Eredményeim")
			icon: "image://font/AcademicI/\uf15d"
			iconColor: "tomato"
			func: function() { replaceContent(cmpStudentGroupScore) }
		}
	}


	activity: StudentMaps {
		id: studentMaps

		onMapDownloadRequest: {
			if (studentMaps.downloader.fullSize > cosClient.getSetting("autoDownloadBelow", 500000)) {
				var d = JS.dialogCreateQml("YesNo", {
											   title: qsTr("Letöltés"),
											   text: qsTr("A szerver %1 adatot akar küldeni. Elindítod a letöltést?").arg(formattedDataSize),
											   image: "qrc:/internal/icon/cloud-download-outline.svg"
										   })
				d.accepted.connect(function() {
					var dd = JS.dialogCreateQml("Progress", { title: qsTr("Letöltés"), downloader: studentMaps.downloader })
					dd.closePolicy = Popup.NoAutoClose
					dd.open()
				})

				d.open()
			} else {
				var dd = JS.dialogCreateQml("Progress", { title: qsTr("Letöltés"), downloader: studentMaps.downloader })
				dd.closePolicy = Popup.NoAutoClose
				dd.open()
			}
		}


		onGameMapLoaded: {
			JS.createPage("Map", {
							  studentMaps: studentMaps,
							  title: map.name,
							  mapUuid: map.uuid,
							  readOnly: !map.active
						  })
		}

		onGameMapUnloaded: {
			if (control.StackView.view)
				mainStack.pop(control)
		}

		Component.onCompleted: init(false)


		onCampaignListGet: {
			if (!actionCampaignFilter.waitForList)
				return

			actionCampaignFilter.waitForList = false

			modelCampaignFilterList.clear()

			for (var i=0; i<jsonData.list.length; i++) {
				var o = jsonData.list[i]
				o.subtitle = JS.readableInterval(o.starttime, o.endtime)
				modelCampaignFilterList.append(o)
			}

			modelCampaignFilterList.append({
											   id: -1,
											   description: qsTr("-- Nincs szűrés --"),
											   subtitle: "",
											   starttime: "",
											   endtime: "",
											   finsihed: false
										   })

			var d = JS.dialogCreateQml("List", {
										   icon: "qrc:/internal/icon/filter.svg",
										   title: qsTr("Szűrés hadjáratra"),
										   selectorSet: false,
										   modelTitleRole: "title",
										   modelSubtitleRole: "subtitle",
										   modelIconRole: "icon",
										   modelIconColorRole: "iconcolor",
										   modelTitleColorRole: "iconcolor",
										   modelSubtitleColorRole: "iconcolor",
										   delegateHeight: CosStyle.twoLineHeight,
										   imageWidth: 0,
										   model: listCampaignFilterModel
									   })


			d.accepted.connect(function(data) {
				if (!data)
					return

				actionCampaignFilter.campaign = data.id
			})
			d.open()
		}
	}



	ListModel {
		id: modelCampaignFilterList
	}

	SortFilterProxyModel {
		id: listCampaignFilterModel
		sourceModel: modelCampaignFilterList
		sorters: [
			FilterSorter {
				ValueFilter { roleName: "id"; value: -1 }
				sortOrder: Qt.DescendingOrder; priority: 2
			},
			StringSorter { roleName: "starttime"; sortOrder: Qt.DescendingOrder; priority: 1 },
			StringSorter { roleName: "endtime"; priority: 0 }
		]

		proxyRoles: [
			ExpressionRole {
				name: "title"
				expression: model.description === "" ? qsTr("Hadjárat •%1").arg(model.id) : model.description
			},
			SwitchRole {
				name: "icon"
				filters: [
					ValueFilter {
						roleName: "id"
						value: -1
						SwitchRole.value: "qrc:/internal/icon/filter-off.svg"
					},
					ValueFilter {
						roleName: "finished"
						value: true
						SwitchRole.value: "qrc:/internal/icon/calendar-check.svg"
					}
				]
				defaultValue: "qrc:/internal/icon/calendar-clock.svg"
			},
			SwitchRole {
				name: "iconcolor"
				filters: ValueFilter {
					roleName: "id"
					value: actionCampaignFilter.campaign
					SwitchRole.value: CosStyle.colorAccent
				}
				defaultValue: CosStyle.colorPrimaryLighter
			}


		]
	}


	Component {
		id: cmpStudentMapList
		StudentMapList {  }
	}

	Component {
		id: cmpStudentMemberList
		StudentMemberList { }
	}

	Component {
		id: cmpStudentGroupScore
		StudentGroupScore { }
	}

	Component {
		id: cmpStudentCampaign
		StudentGroupCampaign { }
	}

	Action {
		id: actionCampaignFilter
		text: qsTr("Szűrő")
		icon.source: campaign == -1 ? "qrc:/internal/icon/filter.svg" : "qrc:/internal/icon/filter-check.svg"

		property int campaign: -1
		property color color: campaign == -1 ? CosStyle.colorPrimaryLight : CosStyle.colorAccent
		property bool waitForList: false

		onTriggered: {
			waitForList = true
			studentMaps.send("campaignListGet", {groupid: studentMaps.selectedGroupId})
		}
	}
}

