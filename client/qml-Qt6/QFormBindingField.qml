import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


Column {
	id: root

	property bool watchModification: true
	readonly property QFormColumn _form : (parent instanceof QFormColumn) ? parent : null
	property string field: ""
	property string fieldData: ""
	property var getData: function() { return saveToList() }


	property var defaultLeftData: ({})
	property var defaultRightData: ({})

	property Component leftComponent: null
	property Component rightComponent: null
	property Component separatorComponent: Qaterial.LabelHeadline4 {
		text: qsTr("⁠–⁠")
		leftPadding: 10
		rightPadding: 10
	}

	property double split: 0.5
	property int defaultCount: 5

	property bool readOnly: true


	Component {
		id: _cmpRow
		Row {
			id: _row
			width: parent.width

			enabled: !root.readOnly

			readonly property alias leftItem: _left.item
			readonly property alias rightItem: _right.item

			readonly property double _itemWidth: width
												 -(_separator.item ? _separator.item.implicitWidth : 0)
												 -(_remove.visible ? _remove.implicitWidth : 0)

			readonly property double _rightWidth: Math.max(_right.item ? _right.item.implicitWidth : 0, _itemWidth*(1-root.split))

			Loader {
				id: _left
				sourceComponent: root.leftComponent
				anchors.verticalCenter: parent.verticalCenter
				width: _row._itemWidth-_row._rightWidth
				Connections {
					target: _left.item

					function onGotoNextField() {
						if (!_right.item)
							return

						if (_right.item.performGoto())
							return

						gotoNextField(_left.item)
					}
				}
			}
			Loader {
				id: _separator
				sourceComponent: root.separatorComponent
				anchors.verticalCenter: parent.verticalCenter
			}
			Loader {
				id: _right
				sourceComponent: root.rightComponent
				width: _row._rightWidth
				anchors.verticalCenter: parent.verticalCenter

				Connections {
					target: _right.item

					function onGotoNextField() { gotoNextField(_right.item) }
				}
			}
			Qaterial.RoundButton {
				id: _remove
				icon.source: Qaterial.Icons.delete_
				icon.color: Qaterial.Colors.red400
				visible: !root.readOnly

				anchors.verticalCenter: parent.verticalCenter
				onClicked: {
					performModification()
					_row.destroy()
				}
			}

			function loadData(param1, param2) {
				if (_left.item)
					_left.item.loadData(param1)

				if (_right.item)
					_right.item.loadData(param2)
			}

			function saveData() {
				var d = {}

				if (_left.item)
					d.first = _left.item.saveData()
				else
					d.first = null

				if (_right.item)
					d.second = _right.item.saveData()
				else
					d.second = null

				return d
			}
		}
	}


	Column {
		id: _col
		width: parent.width
		bottomPadding: 10
	}

	QButton {
		id: _addButton
		anchors.horizontalCenter: parent.horizontalCenter
		text: qsTr("Hozzáadás")
		icon.source: Qaterial.Icons.plus
		outlined: true
		flat: true
		visible: !root.readOnly
		foregroundColor: Qaterial.Colors.green400
		onClicked: {
			root._addItem(defaultLeftData, defaultRightData)
			performModification()
		}
	}



	function _addItem(_param1, _param2) {
		if (!leftComponent || !rightComponent) {
			console.error("Missing component")
			return null
		}

		var o = _cmpRow.createObject(_col)
		o.loadData(_param1, _param2)

		if (o.leftItem && o.leftItem.performGoto())
			return o
		else if (o.rightItem && o.rightItem.performGoto())
			return o

		return o

	}


	function loadFromList(list) {
		let first = null

		for (let i=0; i<list.length || i<defaultCount; ++i) {
			let o = null
			if (i<list.length)
				o = _addItem(list[i].first, list[i].second)
			else
				o = _addItem(defaultLeftData, defaultRightData)

			if (first == null)
				first = o
		}

		if (first && first.leftItem)
			first.leftItem.forceActiveFocus()
	}



	function saveToList() {
		let list = []

		for (let i=0; i<_col.visibleChildren.length; ++i) {
			list.push(_col.visibleChildren[i].saveData())
		}

		return list
	}


	function gotoNextField(_item) {
		let found = false

		for (let i=0; i<_col.visibleChildren.length; ++i) {
			let item = _col.visibleChildren[i]

			if (found) {
				if (item.leftItem && item.leftItem.performGoto())
					return
				else if (item.rightItem && item.rightItem.performGoto())
					return
			} else {
				if (item.leftItem === _item || item.rightItem === _item)
					found = true
			}

		}

		_addButton.clicked()
	}

	function performModification() {
		if (_form && watchModification)
			_form.modified = true
	}

}
