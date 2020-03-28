import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: page

	property string fileName: ""
	property bool mapLoaded: false

	Map {
		id: map

		client: cosClient
		mapType: Map.MapEditor

		onMapBackupExists: {
			var d = JS.dialogCreate(dlgBackupExists)
			d.item.fname = originalFile
			d.open()
		}

		onMapLoaded: page.mapLoaded = true

		onMapSaved: console.debug(data)
	}

	header: QToolBar {
		id: toolbar

		backButton.visible: true
		backButton.onClicked: mainStack.back()
	}

	Image {
		id: bgImage
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/img/villa.png"
	}

	Column {
		QButton {
			label: "NEW"
			onClicked: map.create()
			disabled: mapLoaded
		}

		QButton {
			label: "OPEN"
			onClicked: {
				var d = JS.dialogCreate(dlgFileName)
				d.item.mode = 0
				d.open()
			}
			disabled: mapLoaded
		}


		QButton {
			label: "OPEN JSON"
			onClicked: {
				var d = JS.dialogCreate(dlgFileName)
				d.item.mode = 1
				d.open()
			}
			disabled: mapLoaded
		}


		QButton {
			label: "SAVE"
			onClicked: map.save(true)
			disabled: !mapLoaded
		}

		QButton {
			label: "Save json"
			onClicked: map.save(false)
			disabled: !mapLoaded
		}


		QButton {
			label: "Save as"
			onClicked: {
				var d = JS.dialogCreate(dlgFileName)
				d.item.mode = 4
				d.open()
			}
			disabled: !mapLoaded
		}


		QButton {
			label: "Save as JSON"
			onClicked: {
				var d = JS.dialogCreate(dlgFileName)
				d.item.mode = 5
				d.open()
			}
			disabled: !mapLoaded
		}

	}


	/* CONTENT */
	BusyIndicator {
		id: busy
		anchors.centerIn: parent
		running: false
	}
	/* CONTENT */


	Component {
		id: dlgBackupExists

		QDialogYesNo {
			id: dlgYesNo

			property alias fname: dlgYesNo.text

			title: qsTr("Helyreállítsam automatikusan a fájlt?")

			onDlgAccept: {
				map.loadFromBackup()
			}

		}
	}



	Component {
		id: dlgFileName

		QDialogTextField {
			id: dlgYesNo

			property int mode: 0

			title: switch (mode) {
				   case 0: qsTr("Megnyitás fájlból"); break
				   case 1: qsTr("Megnyitás JSON fájlból"); break
				   case 2: qsTr("Mentés"); break
				   case 3: qsTr("Mentés JSON"); break
				   case 4: qsTr("Mentés másként"); break
				   case 5: qsTr("Mentés másként JSON"); break
				   }

			onDlgAccept: {
				switch (mode) {
				case 0: map.loadFromFile(data, true); break
				case 1: map.loadFromFile(data, false); break
				case 2: map.save(true); break
				case 3: map.save(false); break
				case 4: map.saveAs(data, true); break
				case 5: map.saveAs(data, false); break
				}
			}

		}
	}

	StackView.onRemoved: destroy()

	StackView.onActivated: {
		toolbar.title = qsTr("Pályaszerkesztő")
		/* LOAD */
	}

	StackView.onDeactivated: {
		/* UNLOAD */
	}


	Component.onCompleted: {
		map.databaseCheck()
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
