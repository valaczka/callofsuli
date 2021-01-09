import QtQuick 2.15
import QtQuick.Controls 2.15
import "."
import "Style"
import "JScript.js" as JS


QListView {
	id: view

	property int delegateHeight: CosStyle.baseHeight
	property color backgroundColor: "transparent"
	property color removeButtonBackgroundColor: "transparent"
	property color removeButtonColor: CosStyle.colorError

	property string textRole: "text"
	property string placeholderRole: ""
	property string removeRole: ""
	property string addRole: ""

	property string defaultPlaceholderText: ""

	signal addRequest(int index)
	signal removeRequest(int index)
	signal itemModified(int index, string text)
	signal itemAccepted(int index, string text)

	delegate: Rectangle {
		id: delegateItem
		height: Math.max(view.delegateHeight, textField.height, removeButton.height, addButton.height)
		width: view.width
		color: backgroundColor

		readonly property bool _addMode: addRole.length ? model[addRole] : false

		QTextField {
			id: textField
			visible: !_addMode
			text: textRole.length ? model[textRole] : ""
			placeholderText: placeholderRole.length ? model[placeholderRole] : defaultPlaceholderText
			anchors.left: parent.left
			anchors.verticalCenter: parent.verticalCenter
			anchors.right: removeButton.left

			onTextModified: itemModified(index, text)
			onAccepted: itemAccepted(index, text)
		}

		QButton {
			id: removeButton
			visible: !_addMode && (removeRole.length ? model[removeRole] : true)
			anchors.verticalCenter: parent.verticalCenter
			anchors.right: parent.right
			icon.source: CosStyle.iconDelete
			animationEnabled: false
			themeColors: [removeButtonColor, removeButtonBackgroundColor, "transparent", "transparent"]

			onClicked: removeRequest(index)
		}

		QButton {
			id: addButton
			visible: _addMode
			anchors.centerIn: parent
			icon.source: CosStyle.iconAdd
			display: AbstractButton.TextBesideIcon
			text: qsTr("Hozzáadás")

			onClicked: addRequest(index)
		}

		ListView.onAdd:  forceFocus()



		function forceFocus() {
			if (_addMode)
				addButton.forceActiveFocus()
			else
				textField.forceActiveFocus()
		}
	}


	function forceFocus(index) {
		var i = itemAtIndex(index)
		if (i)
			i.forceFocus()
	}
}
