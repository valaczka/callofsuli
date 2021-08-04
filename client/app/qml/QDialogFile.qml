import QtQuick 2.15
import QtQuick.Controls 2.15
import Qt.labs.folderlistmodel 2.15
import "Style"
import "JScript.js" as JS
import "."

QDialogPanel {
	id: control

	title: isSave ? qsTr("Mentés") : qsTr("Megnyitás")
	icon: isSave ? CosStyle.iconSave : CosStyle.iconBooks

	property alias text: tfFile.text
	property url folder: ""

	property bool isSave: false
	property alias filters: folderListModel.nameFilters


	maximumWidth: 750


	titleColor: CosStyle.colorPrimary


	Item {
		id: toolbar
		anchors.top: parent.top
		anchors.left: parent.left
		anchors.right: parent.right
		height: Math.max(CosStyle.halfLineHeight, button.height, filePath.height)

		QButton {
			id: button
			text: ".."
			anchors.right: parent.right
			anchors.verticalCenter: parent.verticalCenter

			animationEnabled: false

			enabled: folderListModel.folder.toString() !== "file:///"

			onClicked: {
				folderListModel.folder = folderListModel.parentFolder
			}
		}

		QLabel {
			id: filePath
			elide: Text.ElideMiddle
			wrapMode: Text.Wrap
			verticalAlignment: Text.AlignVCenter

			anchors.verticalCenter: parent.verticalCenter

			width: parent.width-button.width-5
		}
	}


	QListView {
		id: view
		anchors.top: toolbar.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: tfFile.top

		model: FolderListModel {
			id:  folderListModel

			showDirsFirst: true

			onFolderChanged: filePath.text = folder.toString().replace("file:///", "► ").replace(new RegExp("/",'g'), " ► ")

			/*showDotAndDotDot: picker.showDotAndDotDot
			showHidden: picker.showHidden
			showDirsFirst: picker.showDirsFirst
			folder: picker.folder
			nameFilters: picker.nameFilters*/
		}

		delegate: Item {
			id: item
			width: view.width
			height: CosStyle.baseHeight

			required property string fileName
			required property url fileUrl
			required property bool fileIsDir

			QFontImage {
				id: img

				width: height
				height: item.height

				icon: fileIsDir ? CosStyle.iconBooks : CosStyle.iconPlay
			}

			QLabel {
				x: img.width+5
				width: parent.width-x
				anchors.verticalCenter: parent.verticalCenter
				text: fileName

				color: fileIsDir ? CosStyle.colorAccent : CosStyle.colorPrimaryLighter
			}

			MouseArea {
				anchors.fill: parent
				acceptedButtons: Qt.LeftButton

				onClicked: {
					if (item.fileIsDir) {
						folderListModel.folder = fileUrl
					} else if (!isSave) {
						acceptedData = fileUrl
						dlgClose()
					} else if (isSave) {
						tfFile.text = fileName
					}
				}
			}


		}
	}

	QTextField {
		id: tfFile

		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		lineVisible: true
		placeholderText: qsTr("fájl neve")

		visible: isSave

		onAccepted: {
			acceptedData = folderListModel.folder+"/"+text
			dlgClose()
		}
	}


	buttons: Row {
		id: buttonRow
		spacing: 10

		anchors.horizontalCenter: parent.horizontalCenter

		QButton {
			id: buttonNo
			anchors.verticalCenter: parent.verticalCenter
			text: qsTr("Mégsem")
			icon.source: CosStyle.iconCancel
			themeColors: CosStyle.buttonThemeRed

			onClicked: dlgClose()
		}

		QButton {
			id: buttonYes

			anchors.verticalCenter: parent.verticalCenter

			text: qsTr("Mentés")
			icon.source: CosStyle.iconSave
			themeColors: CosStyle.buttonThemeGreen

			visible: isSave

			onClicked: {
				acceptedData = folderListModel.folder+"/"+tfFile.text
				dlgClose()
			}
		}
	}


	function populated() {
		if (folder != "")
			folderListModel.folder = folder
		else if (Qt.platform.os == "android")
			folderListModel.folder = "file:///sdcard"

		if (isSave)
			tfFile.forceActiveFocus()
		else
			view.forceActiveFocus()
	}

}
