import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPagePanel {
	id: panel

	enum EditType {
		Invalid,
		Campaign,
		Mission
	}

	property int editType: MapEditorCampaignEdit.Invalid
	property int campaignId: -1
	property string missionUuid: ""

	layoutFillWidth: true


	title: switch (editType) {
		   case MapEditorCampaignEdit.Campaign:
			   qsTr("Hadjárat")
			   break
		   case MapEditorCampaignEdit.Mission:
			   qsTr("Küldetés")
			   break
		   default:
			   qsTr("Adatok")
		   }



	icon: switch (editType) {
		  case MapEditorCampaignEdit.Campaign:
			  CosStyle.iconUserWhite
			  break
		  case MapEditorCampaignEdit.Mission:
			  CosStyle.iconUser
			  break
		  default:
			  CosStyle.iconUsers
		  }



	property var contextFuncCampaign: function (m) {
		m.addAction(actionRemove)
	}

	property var contextFuncMission: function (m) {
		m.addAction(actionRemove)
	}

	contextMenuFunc: switch (editType) {
					 case MapEditorCampaignEdit.Campaign:
						 contextFuncCampaign
						 break
					 case MapEditorCampaignEdit.Mission:
						 contextFuncMission
						 qsTr("Küldetés")
						 break
					 default:
						 null
					 }




	property alias locksModel: locksProxyModel.sourceModel

	Connections {
		target: mapEditor

		function onCampaignLoaded(data) {
			locksModel.clear()

			if (!Object.keys(data).length) {
				editType = MapEditorCampaignEdit.Invalid
				campaignId = -1
				return
			}

			editType = MapEditorCampaignEdit.Campaign
			campaignId = data.id
			JS.setSqlFields([
								textCampaignName
							], data)

			locksModel.setVariantList(data.locks, "lock")

			if (swipeMode)
				parent.parentPage.swipeToPage(1)
		}



		function onCampaignRemoved() {
			reloadData()
		}

		function onMissionRemoved() {
			reloadData()
		}
	}

	Connections {
		target: mapEditor.db

		function onUndone() {
			reloadData()
		}
	}




	QLabel {
		id: noLabel
		opacity: editType === MapEditorCampaignEdit.Invalid
		visible: opacity != 0

		anchors.centerIn: parent

		text: qsTr("Válassz hadjáratot/küldetést")

		Behavior on opacity { NumberAnimation { duration: 125 } }
	}

	QAccordion {
		anchors.fill: parent

		opacity: editType === MapEditorCampaignEdit.Campaign
		visible: opacity != 0

		Behavior on opacity { NumberAnimation { duration: 125 } }

		QCollapsible {
			title: qsTr("Általános")

			QGridLayout {
				width: parent.width

				watchModification: false

				//onAccepted: buttonSave.press()

				QGridLabel { field: textCampaignName }

				QGridTextField {
					id: textCampaignName
					fieldName: qsTr("Hadjárat neve")
					sqlField: "name"

					onTextModified: {
						mapEditor.run("campaignModify", {id: campaignId, data:{ name: text }})
					}


					validator: RegExpValidator { regExp: /.+/ }
				}

			}
		}


		QCollapsible {
			title: qsTr("Zárolások")



			QVariantMapProxyView {
				id: locksView

				model: SortFilterProxyModel {
					id: locksProxyModel

					sourceModel: mapEditor.newModel(["lock", "name"])

					sorters: [
						StringSorter { roleName: "name" }
					]
				}

				autoSelectorChange: true

				delegateHeight: CosStyle.halfLineHeight

				modelTitleRole: "name"

				width: parent.width-locksCol.width
			}

			Column {
				id: locksCol
				anchors.left: locksView.right

				QToolButton {
					display: AbstractButton.IconOnly
					action: actionLockAdd
				}

				QToolButton {
					display: AbstractButton.IconOnly
					action: actionLockRemove
				}
			}

		}





		/*Action {
		id: actionRemove
		icon.source: CosStyle.iconRemove
		text: qsTr("Törlés")
		onTriggered: {
			var d = JS.dialogCreateQml("YesNo", {
										   title: qsTr("Biztosan törlöd a szervert?"),
										   text: textName.text
									   })
			d.accepted.connect(function () {
				servers.serverDeleteKey(servers.serverKey)
				servers.editing = false
				servers.serverKey = -1
			})
			d.open()
		}
	}

	Connections {
		target: servers

		function onEditingChanged(editing) {
			loadData()
		}

		function onServerKeyChanged(serverKey) {
			loadData()
		}
	}

	//onPanelActivated: textHostname.forceActiveFocus()

	Component.onCompleted: loadData()

	function loadData() {
		if (!servers.editing)
			return

		if (servers.serverKey==-1) {
			JS.setSqlFields([
								textName,
								textHostname,
								textPort,
								checkSsl
							], {name:"", host:"", port:10101, ssl:false})
		} else  {
			JS.setSqlFields([
								textName,
								textHostname,
								textPort,
								checkSsl
							], servers.serversModel.getByKey(servers.serverKey))
		}

		grid.modified = false

		if (swipeMode)
			parent.parentPage.swipeToPage(1)

	}*/

	}


	Action {
		id: actionLockAdd
		text: qsTr("Hozzáadás")
		icon.source: CosStyle.iconAdd
	}

	Action {
		id: actionLockRemove
		text: qsTr("Eltávolítás")
		icon.source: CosStyle.iconRemove
	}


	Action {
		id: actionRemove
		text: qsTr("Törlés")
		icon.source: CosStyle.iconDelete
		enabled: !mapEditor.isBusy && editType !== MapEditorCampaignEdit.Invalid
		onTriggered: {
			var d = JS.dialogCreateQml("YesNo")

			if (editType === MapEditorCampaignEdit.Campaign) {
				d.item.title = textCampaignName.text
				d.item.text = qsTr("Biztosan törlöd a hadjáratot a küldetéseivel együtt?")

				d.accepted.connect(function(data) {
					mapEditor.run("campaignRemove", {"id": campaignId})
				})
			} else if (editType === MapEditorCampaignEdit.Mission) {
				//d.item.title = o.name
				d.item.text = qsTr("Biztosan törlöd a küldetést?")

				d.accepted.connect(function(data) {
					mapEditor.run("missionRemove", {"uuid": missionUuid})
				})
			}

			d.open()
		}
	}



	function reloadData() {
		if (editType === MapEditorCampaignEdit.Campaign)
			mapEditor.run("campaignLoad", {id: campaignId})
		else if (editType === MapEditorCampaignEdit.Mission)
			mapEditor.run("missionLoad", {uuid: missionUuid})
	}
}

