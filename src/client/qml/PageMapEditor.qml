import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: page

	//requiredPanelWidth: 900

	title: qsTr("Pályszerkesztő")
	subtitle: mapEditor.mapName

	property string loadFileName: ""

	//mainToolBarComponent: QToolBusyIndicator { running: true }


	activity: MapEditor {
		id: mapEditor

		onBackupReady: {
			var ld = false
			var d = JS.dialogCreateQml("YesNo", {
										   title: qsTr("Biztonsági másolat található"),
										   text: qsTr("Megpróbáljuk visszaállítani a biztonsági másolatot?")
									   })
			d.accepted.connect(function () {
				ld = true
				loadFromBackup()
			})
			d.closedAndDestroyed.connect(function() {
				if (!ld) {
					var d2 = JS.dialogCreateQml("YesNo", {
													title: qsTr("Biztonsági másolat törlése"),
													text: qsTr("Töröljük a biztonsági másolatot?")
												})
					d2.accepted.connect(function () {
						removeBackup()
					})
					d2.open()
				}
			})

			d.open()
		}

		onBackupUnavailable: {
			if (page.loadFileName.length) {
				loadFromFile(page.loadFileName)
			}
		}

		onDatabaseLoaded: console.debug("---------------")
	}


	Label {
		anchors.centerIn: parent
		text: "***** "+mapEditor.loadProgress
	}

	//mainMenuFunc: function(m) {}
	//contextMenuFunc: function(m) {}

	/*panelComponents: [
		Component { QPagePanel {
				panelVisible: true
				layoutFillWidth: true
			} }
	]*/

	onPageActivated: {
		mapEditor.checkBackup()
	}


	function windowClose() {
		mapEditor.removeDatabase()
		return true
	}

	function pageStackBack() {
		mapEditor.removeDatabase()
		return false
	}

}
