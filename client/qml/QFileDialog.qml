import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.folderlistmodel 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.ModalDialog
{
	id: control

	property bool isSave: false
	property alias text: tfFile.text
	property alias placeholderText: tfFile.placeholderText
	property alias textTitle: tfFile.title
	property url folder: ""

	property string suffix: ""

	property alias filters: folderListModel.nameFilters
	property alias modelFolder: folderListModel.folder

	property bool isDirectorySelect: false


	horizontalPadding: 0

	dialogImplicitWidth: 600

	signal fileSelected(url file)
	signal directorySelected(url dir)


	property url _selectedFile: ""


	contentItem: ColumnLayout
	{
		id: col

		spacing: 5

		Item {
			id: itemNoPermissions
			Layout.fillWidth: true
			Layout.fillHeight: true

			Qaterial.IconLabelWithCaption {
				anchors.centerIn: parent
				icon.source: Qaterial.Icons.connection
				icon.color: Qaterial.Colors.orange500
				textColor: Qaterial.Colors.orange500
				text: qsTr("Fájlrendszer nem elérhető")
				caption: qsTr("Nem lehet hozzáférni a fájlrendszerhez")
			}
		}


		Item {
			id: toolbar

			Layout.fillWidth: true
			Layout.preferredHeight: Math.max(button.height, filePath.height)

			property int leftPadding: Qaterial.Style.delegate.leftPadding(Qaterial.Style.DelegateType.Icon, 1)
			property int rightPadding: Qaterial.Style.delegate.rightPadding(Qaterial.Style.DelegateType.Icon, 1)

			visible: !itemNoPermissions.visible

			QButton {
				id: button
				text: ".."
				highlighted: true
				anchors.right: parent.right
				anchors.rightMargin: parent.rightPadding
				anchors.verticalCenter: parent.verticalCenter

				enabled: folderListModel.folder.toString() !== "file:///"

				onClicked: {
					folderListModel.folder = folderListModel.parentFolder
				}
			}

			Flow {
				id: filePath
				spacing: 5
				anchors.verticalCenter: parent.verticalCenter
				anchors.left: parent.left
				anchors.leftMargin: parent.leftPadding
				width: parent.width-button.width-parent.leftPadding-parent.rightPadding

				Repeater {
					model: ListModel { id: filePathModel }

					Qaterial.LabelCaption {
						text: _text
						color: (url && url.length && index != 0) ? Qaterial.Style.primaryTextColor() : Qaterial.Style.accentColor

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

		Qaterial.HorizontalLineSeparator {
			Layout.fillWidth: true
			visible: !itemNoPermissions.visible
		}

		ListView {
			id: view
			Layout.fillWidth: true
			Layout.fillHeight: true

			implicitHeight: 800
			implicitWidth: 200

			clip: true

			visible: !itemNoPermissions.visible

			model: FolderListModel {
				id:  folderListModel

				showDirsFirst: true
				//showFiles: !control.isDirectorySelect

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

			delegate: Qaterial.ItemDelegate {
				id: item
				width: view.width

				required property string fileName
				required property url fileUrl
				required property bool fileIsDir

				text: fileName
				icon.source: fileIsDir ? Qaterial.Icons.folderOpen : Qaterial.Icons.fileOutline
				textColor: fileIsDir ? Qaterial.Style.accentColor :
									   control.isDirectorySelect ? Qaterial.Style.disabledTextColor() : Qaterial.Style.primaryTextColor()

				MouseArea {
					anchors.fill: parent
					acceptedButtons: Qt.LeftButton

					onClicked: {
						if (item.fileIsDir) {
							folderListModel.folder = fileUrl
						} else if (control.isDirectorySelect) {
							return
						} else if (!isSave) {
							_selectedFile = fileUrl
							control.accept()
						} else if (isSave) {
							tfFile.text = fileName
						}
					}
				}
			}
		}

		Qaterial.TextField {
			id: tfFile

			Layout.fillWidth: true
			Layout.leftMargin: Qaterial.Style.delegate.leftPadding(Qaterial.Style.DelegateType.Icon, 1)
			Layout.rightMargin: Qaterial.Style.delegate.rightPadding(Qaterial.Style.DelegateType.Icon, 1)
			Layout.alignment: Qt.AlignHCenter

			title: qsTr("Fájlnév")
			placeholderText: qsTr("A fájl neve")

			leadingIconSource: Qaterial.Icons.file
			leadingIconInline: true

			visible: isSave && !itemNoPermissions.visible && !control.isDirectorySelect

			suffixText: control.suffix != "" ? (text.endsWith(control.suffix) ? "" : control.suffix) : ""

			onAccepted: {
				control.accept()
			}
		}

	}

	standardButtons: itemNoPermissions.visible ? Dialog.Close :
												 control.isDirectorySelect ? Dialog.Ok | Dialog.Cancel :
												 isSave ? Dialog.Save | Dialog.Cancel : Dialog.Cancel

	Component.onCompleted: {
		Client.Utils.checkStoragePermissions()
		open()
	}


	onOpened: {
		if (isSave && !isDirectorySelect)
			tfFile.forceActiveFocus()
		else
			view.forceActiveFocus()
	}

	onAccepted: {
		if (isDirectorySelect) {
			directorySelected(folderListModel.folder)
			return
		}

		if (isSave && tfFile.text == "")
			return

		if (isSave) {
			let t = tfFile.text

			if (control.suffix != "" && !t.endsWith(control.suffix))
				t += control.suffix

			_selectedFile = folderListModel.folder+"/"+t
		}

		if (_selectedFile == "")
			return

		fileSelected(_selectedFile)
	}

	Connections {
		target: Client.Utils

		function onStoragePermissionsGranted() {
			itemNoPermissions.visible = false

			if (Qt.platform.os == "android" || Qt.platform.os === "ios")
				folderListModel.folder = Client.Utils.genericDataPath()
			else if (folder != "")
				folderListModel.folder = folder
		}

		/*function onStoragePermissionsDenied() {
			labelNoPermissions.text = qsTr("Írási/olvasási jogosultság hiányzik")
		}*/
	}

}

