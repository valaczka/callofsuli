import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: control

	required property MapEditor mapEditor

	/*stackPopFunction: function() {
		if (_stack._step > 0) {
			_stack._step--
			return false
		}

		return true
	}*/

	title: qsTr("Mátrix szerkesztés")

	appBar.backButtonVisible: true


	MapEditorMatrixImporter {
		id: _importer

		mapEditor: control.mapEditor

		onImported: {
			Client.stackPop(control)
		}
	}


	QScrollable {
		anchors.fill: parent

		contentCentered: true
		refreshEnabled: false

		Column {
			spacing: 20

			width: parent.width

			QLabelInformative {
				text: qsTr("(1) Töltsd le a sablont a kitöltéshez")
			}


			Qaterial.OutlineButton {
				anchors.horizontalCenter: parent.horizontalCenter
				highlighted: false
				text: qsTr("Sablon letöltése")
				icon.source: Qaterial.Icons.fileDocumentOutline
				onClicked: {
					/*if (Qt.platform.os == "wasm")
						_importer.wasmDownloadTemplate()
					else*/
						Qaterial.DialogManager.openFromComponent(_cmpFileSave)
				}
			}


			QLabelInformative {
				visible: _importer.downloaded
				text: qsTr("(2) Töltsd fel a kitöltött táblázatot")
			}

			QButton {
				anchors.horizontalCenter: parent.horizontalCenter
				visible: _importer.downloaded

				highlighted: true
				text: qsTr("Feltöltés")
				icon.source: Qaterial.Icons.upload
				onClicked: {
					/*if (Qt.platform.os == "wasm")
						_importer.wasmUpload()
					else*/
						Qaterial.DialogManager.openFromComponent(_cmpFileOpen)

				}
			}

		}
	}


	Component {
		id: _cmpFileOpen

		QFileDialog {
			title: qsTr("Adatok feltöltése")
			filters: [ "*.xlsx" ]
			onFileSelected: file => {
								_importer.upload(file)
								Client.Utils.settingsSet("folder/excelImport", modelFolder.toString())
							}

			folder: Client.Utils.settingsGet("folder/excelImport", "")
		}

	}



	Component {
		id: _cmpFileSave

		QFileDialog {
			title: qsTr("Sablon letöltése")
			filters: [ "*.xlsx" ]

			folder: Client.Utils.settingsGet("folder/excelImport", "")

			isSave: true
			suffix: ".xlsx"
			onFileSelected: file => {
								if (Client.Utils.fileExists(file))
								overrideQuestion(file)
								else
								_importer.downloadTemplate(file)

								Client.Utils.settingsSet("folder/excelImport", modelFolder.toString())
							}

		}
	}

	function overrideQuestion(file) {
		JS.questionDialog({
							  onAccepted: function()
							  {
								  _importer.downloadTemplate(file)
							  },
							  text: qsTr("A fájl létezik. Felülírjuk?\n%1").arg(file),
							  title: qsTr("Sablon letöltése"),
							  iconSource: Qaterial.Icons.fileAlert
						  })
	}


	StackView.onActivated: {
		//Client.reloadCache("classList", control, _preparedClassList.reload)
	}

}
