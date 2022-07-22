import QtQuick 2.15
import QtQuick.Controls 2.15
import Qt.labs.folderlistmodel 2.15
import COS.Client 1.0
import "Style"
import "JScript.js" as JS
import "."

QDialogPanel {
	id: control

	title: isSave ? qsTr("Mentés") : qsTr("Megnyitás")
	icon: isSave ? "qrc:/internal/icon/content-save.svg" : "qrc:/internal/icon/folder-open.svg"

	property alias text: tfFile.text
	property url folder: ""

	property bool isSave: false
	property alias filters: folderListModel.nameFilters

	property alias modelFolder: folderListModel.folder


	maximumWidth: 750


	titleColor: CosStyle.colorPrimary


	Item {
		id: toolbar
		anchors.top: parent.top
		anchors.left: parent.left
		anchors.right: parent.right
		height: Math.max(CosStyle.halfLineHeight, button.height, filePath.height)

		visible: !labelNoPermissions.visible

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

		Flow {
			id: filePath
			spacing: 5
			anchors.verticalCenter: parent.verticalCenter
			width: parent.width-button.width-5

			Repeater {
				model: ListModel { id: filePathModel }

				QLabel {
					text: _text
					color: (url && url.length && index != 0) ? CosStyle.colorPrimaryLighter : CosStyle.colorAccent

					MouseArea {
						anchors.fill: parent
						acceptedButtons: Qt.LeftButton
						enabled: url.length
						onClicked: folderListModel.folder = url
					}
				}
			}
		}
	}


	QListView {
		id: view
		anchors.top: toolbar.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: tfFile.top

		visible: !labelNoPermissions.visible

		model: FolderListModel {
			id:  folderListModel

			showDirsFirst: true

			onFolderChanged: setFilePath()

			Component.onCompleted: setFilePath()

			function setFilePath() {
				filePathModel.clear()
				var l = folder.toString().replace("file:///", "").split("/")

				var u = "file:///"
				filePathModel.append({ _text: "►", url: u})

				for (var i=0; i<l.length; i++) {
					if (i>0) {
						filePathModel.append({ _text: "►", url: ""})
						u += "/"
					}

					u += l[i]
					filePathModel.append({ _text: l[i], url: u })
				}

			}

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

				icon: fileIsDir ? "qrc:/internal/icon/folder-open.svg" : "qrc:/internal/icon/file-outline.svg"
				color: fileIsDir ? CosStyle.colorAccent : CosStyle.colorPrimaryLighter
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

		visible: isSave && !labelNoPermissions.visible

		onAccepted: {
			acceptedData = folderListModel.folder+"/"+text
			dlgClose()
		}
	}


	QLabel {
		id: labelNoPermissions
		anchors.centerIn: parent
		text: qsTr("Írási/olvasási jogosultság ellenőrzése")
	}

	buttons: Row {
		id: buttonRow
		spacing: 10

		anchors.horizontalCenter: parent.horizontalCenter

		QButton {
			id: buttonNo
			anchors.verticalCenter: parent.verticalCenter
			text: qsTr("Mégsem")
			icon.source: "qrc:/internal/icon/close-circle.svg"
			themeColors: CosStyle.buttonThemeRed

			onClicked: dlgClose()
		}

		QButton {
			id: buttonYes

			anchors.verticalCenter: parent.verticalCenter

			text: qsTr("Mentés")
			icon.source: "qrc:/internal/icon/content-save.svg"
			themeColors: CosStyle.buttonThemeGreen

			visible: isSave && !labelNoPermissions.visible

			onClicked: {
				acceptedData = folderListModel.folder+"/"+tfFile.text
				dlgClose()
			}
		}
	}


	Connections {
		target: cosClient

		function onStoragePermissionsGranted() {
			labelNoPermissions.visible = false

			if (Qt.platform.os == "android" || Qt.platform.os === "ios")
				folderListModel.folder = cosClient.genericDataPath()
			else if (folder != "")
				folderListModel.folder = folder
		}

		function onStoragePermissionsDenied() {
			labelNoPermissions.text = qsTr("Írási/olvasási jogosultság hiányzik")
		}
	}

	Component.onCompleted: {
		cosClient.checkStoragePermissions()
	}

	function populated() {
		if (isSave)
			tfFile.forceActiveFocus()
		else
			view.forceActiveFocus()
	}

}
