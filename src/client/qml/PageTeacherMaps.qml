import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: page

	TeacherMaps {
		id: teacherMaps
		client: cosClient

		onMapListLoaded: setModel(list)
		onMapCreated: listReload()
		onMapUpdated: {
			if (data.error) {
				client.sendMessageError("ADATBÁZIS", "HIBA", data.error)
			}
		}

		onMapReceived: {
			var o = JS.createPage("MapEditor",
								  {
									  mapId: jsonData["id"],
									  mapName: jsonData["name"],
									  mapBinaryFormat: true
								  },
								  page)
			o.map.loadFromJson(mapData)

			o.map.mapSaved.connect(page.onMapSaved)
		}
	}

	header: QToolBar {
		id: toolbar

		backButton.visible: true
		backButton.onClicked: mainStack.back()

		rightLoader.sourceComponent: Row {
			QToolBusyIndicator { running: teacherMaps.isBusy }
			QMenuButton {
				MenuItem {
					text: qsTr("Új pálya")
					onClicked:  {
						var d = JS.dialogCreate(dlgMapName)
						d.item.mode = 0
						d.open()
					}
				}
			}
		}
	}


	Image {
		id: bgImage
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/img/villa.png"
	}



	QPagePanel {
		id: p

		maximumWidth: 800

		blurSource: bgImage

		QListItemDelegate {
			id: listMaps
			anchors.fill: parent

			modelTitleSet: true
			modelSubtitleSet: true
			modelRightSet: true
			modelToolTipSet: false

			onClicked: teacherMaps.send({"class": "teacherMaps", "func": "getMap", "id": model.get(index).id })

			onLongPressed: {
				listRigthMenu.modelIndex = index
				listRigthMenu.popup()
			}

			onRightClicked: {
				listRigthMenu.modelIndex = index
				listRigthMenu.popup()
			}

			Keys.onPressed: {
				if (event.key === Qt.Key_Insert) {
					editServer(-1)
				} else if (event.key === Qt.Key_F4 && listMenu.currentIndex !== -1) {
					editServer(listMenu.model.get(listMenu.currentIndex).id)
				} else if (event.key === Qt.Key_Delete && listMenu.currentIndex !== -1) {
					var d = JS.dialogCreate(dlgDelete)
					d.open()
				}
			}
		}

		QMenu {
			id: listRigthMenu

			property int modelIndex: -1


			MenuItem {
				text: qsTr("Szerkesztés")
				//onClicked: editServer(listMenu.model.get(listRigthMenu.modelIndex).id)
			}

			MenuItem {
				text: qsTr("Törlés")


			}

			MenuSeparator {}

			MenuItem {
				text: qsTr("Új pálya")
			}
		}

	}



	Component {
		id: dlgMapName

		QDialogTextField {
			id: dlgYesNo

			property int mode: 0

			title: switch (mode) {
				   case 0: qsTr("Új pálya neve"); break
				   }

			onDlgAccept: {
				switch (mode) {
				case 0: teacherMaps.send({"class": "teacherMaps", "func": "createMap", "name": data }); break
				}
			}

		}
	}


	function onMapSaved(data, uuid, mapid) {
		teacherMaps.send({"class": "teacherMaps", "func": "updateMap", "id": mapid }, data)
	}


	StackView.onRemoved: destroy()

	StackView.onActivated: {
		toolbar.title = qsTr("Pályák")
		listReload()
	}

	StackView.onDeactivated: {
		/* UNLOAD */
	}


	function listReload() {
		teacherMaps.send({"class": "teacherMaps", "func": "getAllMap"})
	}


	function setModel(list) {
		listMaps.model.clear()
		for (var i=0; i<list.length; i++) {
			var o = list[i]
			o.labelTitle = o.name
			o.labelSubtitle = o.timeCreated+" "+o.timeModified+" v"+o.version
			o.labelRight = o.id
			o.toolTip = "Objectives "+o.objectives
			listMaps.model.append(o)
		}

	}

	function stackBack() {
		if (mainStack.depth > page.StackView.index+1) {
			if (!mainStack.get(page.StackView.index+1).stackBack()) {
				if (mainStack.depth > page.StackView.index+1) {
					mainStack.pop(page)
				}
			}
			return true
		}

		/* BACK */

		return false
	}
}
