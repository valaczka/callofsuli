import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


Column {
	id: control

	property alias title: _title.text
	property bool modified: false

	width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
	anchors.horizontalCenter: parent.horizontalCenter

	Qaterial.LabelHeadline5
	{
		id: _title
		width: parent.width
		wrapMode: Label.Wrap
		topPadding: 8
		bottomPadding: 20
		visible: text.length
	}

	function setItems(_items, _data) {
		for (var i=0; i<_items.length; i++) {
			if (Object.keys(_data).includes(_items[i].field))
				_items[i].fieldData = _data[_items[i].field]
		}
	}

	function getItems(_items) {
		var o = {}

		for (var i=0; i<_items.length; i++) {
			o[_items[i].field] = _items[i].getData()
		}

		return o
	}
}
