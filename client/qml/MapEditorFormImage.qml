import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

Row {
	id: root

	property bool watchModification: true
	readonly property QFormColumn _form : (parent instanceof QFormColumn) ? parent : null
	property string field: ""
	property int fieldData: -1
	property var getData: function() { return _spin.value }

	property MapEditorImage image: null
	property MapEditor editor: null
	property bool readOnly: bindingField && bindingField.readOnly

	property QFormBindingField bindingField: null
	property var saveData: function() { return image ? image.imageid : -1 }

	signal gotoNextField()
	signal modified(int id)


	function loadData(d) {
		if (editor && editor.map)
			image = editor.map.image(d)
		else
			image = null
	}


	function performGoto() {
		return false
	}

	spacing: 5

	Item {
		width: Qaterial.Style.pixelSize * 4.5
		height: width

		anchors.verticalCenter: parent.verticalCenter

		visible: image

		Image {
			id: _img
			fillMode: Image.PreserveAspectFit
			width: parent.width-20
			height: parent.height-20
			anchors.centerIn: parent
			source: image ? "image://mapimage/%1".arg(image.imageid) : ""
			visible: false
			cache: false
			//asynchronous: true
			sourceSize.width: width
			sourceSize.height: height
		}

		Glow {
			anchors.fill: _img
			source: _img
			radius: 4
			samples: 9
			color: Qaterial.Style.iconColor()
			spread: 0.5
		}
	}

	Qaterial.RoundButton {
		icon.color: Qaterial.Colors.red500
		icon.source: Qaterial.Icons.imageRemoveOutline

		anchors.verticalCenter: parent.verticalCenter

		visible: image && !readOnly
		enabled: editor

		onClicked: {
			if (_form && watchModification)
				_form.modified = true
			if (bindingField)
				bindingField.performModification()

			root.modified(-1)
		}
	}

	Qaterial.RoundButton {
		icon.color: Qaterial.Colors.green500
		icon.source: Qaterial.Icons.imagePlusOutline

		anchors.verticalCenter: parent.verticalCenter

		visible: !image && !readOnly
		enabled: editor

		onClicked: {
			if (!editor)
				return

			if (Qt.platform.os == "wasm") {
				editor.wasmUploadImage(function(img){
					if (_form && watchModification)
						_form.modified = true
					if (bindingField)
						bindingField.performModification()

					root.modified(img.imageid)
				})

			} else
				Qaterial.DialogManager.openFromComponent(_dialog)
		}
	}

	Component {
		id: _dialog

		QFileDialog {
			title: qsTr("Kép hozzáadása")
			filters: [ "*.jpg", "*.jpeg", "*.png", "*.gif", "*.bmp" ]
			onFileSelected: {
				var img = editor.uploadImage(file)

				if (!img)
					return

				if (_form && watchModification)
					_form.modified = true
				if (bindingField)
					bindingField.performModification()

				root.modified(img.imageid)

				Client.Utils.settingsSet("folder/mapEditor", modelFolder.toString())
			}

			folder: Client.Utils.settingsGet("folder/mapEditor", "")
		}
	}

}

