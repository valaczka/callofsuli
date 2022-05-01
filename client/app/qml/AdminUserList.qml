import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
	id: control

	title: qsTr("Felhasználók")
	icon: CosStyle.iconUsers

	property var queryParameters: ({})

	menu: QMenu {
		MenuItem { action: actionRename }
		MenuItem { action: actionDelete }
	}

	SortFilterProxyModel {
		id: userProxyModel
		sourceModel: serverSettings.modelUserList

		sorters: [
			StringSorter { roleName: "firstname"; sortOrder: Qt.AscendingOrder; priority: 2 },
			StringSorter { roleName: "lastname"; sortOrder: Qt.AscendingOrder; priority: 1 }
		]

		proxyRoles: [
			ExpressionRole {
				name: "name"
				expression: model.firstname+" "+model.lastname
			},
			SwitchRole {
				name: "background"
				filters: [
					ValueFilter {
						roleName: "isAdmin"
						value: true
						SwitchRole.value: JS.setColorAlpha(CosStyle.colorErrorDark, 0.4)
					},
					ValueFilter {
						roleName: "isTeacher"
						value: true
						SwitchRole.value: JS.setColorAlpha(CosStyle.colorAccentDark, 0.4)
					}
				]
				defaultValue: "transparent"
			},
			SwitchRole {
				name: "titlecolor"
				filters: ValueFilter {
					roleName: "username"
					value: cosClient.userName
					SwitchRole.value: CosStyle.colorAccentLight
				}
				defaultValue: CosStyle.colorPrimaryLighter
			}

		]
	}

	QIconEmpty {
		visible: userProxyModel.count == 0
		anchors.centerIn: parent
		textWidth: parent.width*0.75
		tabContainer: control
		text: qsTr("Egyetlen felhasználó sem tartozik ide")
	}


	QObjectListView {
		id: userList
		anchors.fill: parent

		refreshEnabled: true
		delegateHeight: CosStyle.twoLineHeight
		autoSelectorChange: true

		header: QTabHeader {
			tabContainer: control
			isPlaceholder: true
		}

		leftComponent: QProfileImage {
			source: model && model.picture ? model.picture : ""
			rankId: model ? model.rankid : -1
			rankImage: model ? model.rankimage : ""
			width: userList.delegateHeight+10
			height: userList.delegateHeight*0.8
		}

		rightComponent: QLabel {
			text: model ? Number(model.xp).toLocaleString()+" XP" : ""
			font.weight: Font.Normal
			font.pixelSize: userList.delegateHeight*0.5
			color: CosStyle.colorAccent
			leftPadding: 5
		}

		model: userProxyModel
		modelTitleRole: "name"
		modelSubtitleRole: "username"
		modelBackgroundRole: "background"
		modelTitleColorRole: "titlecolor"
		colorSubtitle: CosStyle.colorPrimaryDarker

		highlightCurrentItem: false

		onRefreshRequest: serverSettings.send("userListGet", queryParameters)

		onClicked: {
			//userSelected(userList.modelObject(index).username)
		}
	}



	onPopulated: {
		serverSettings.send("userListGet", queryParameters)
	}


	Connections {
		target: serverSettings

		function onClassUpdate(jsonData, binaryData) {
			if (queryParameters.classid !== undefined && queryParameters.classid !== -1 && jsonData.updated === queryParameters.classid) {
				contentTitle = jsonData.name
				tabPage.contentTitle = contentTitle
			}
		}
	}

	Action {
		id: actionRename
		text: qsTr("Átnevezés")
		icon.source: CosStyle.iconRename
		enabled: queryParameters.classid !== undefined && queryParameters.classid > 0
		onTriggered: {
			var d = JS.dialogCreateQml("TextField", { title: qsTr("Osztály neve"), value: control.contentTitle })

			d.accepted.connect(function(data) {
				if (data.length)
					serverSettings.send("classUpdate", {id: queryParameters.classid, name: data})
			})
			d.open()
		}
	}

	Action {
		id: actionDelete
		text: qsTr("Törlés")
		icon.source: CosStyle.iconDelete
		enabled: queryParameters.classid !== undefined && queryParameters.classid > 0
		onTriggered: {
			var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan törlöd az osztályt?\n%1").arg(control.contentTitle)})
			d.accepted.connect(function() {
				serverSettings.send("classRemove", {id: queryParameters.classid})
				mainStack.back()
			})
			d.open()
		}
	}
}
