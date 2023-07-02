import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

Qaterial.LoaderItemDelegate {
	id: control

	property bool setCurrentIndexEnabled: true
	property url iconSource: ""						// Ezt használjuk az icon.source helyett
	property color iconColor: Qaterial.Style.iconColor()
	property SelectableObject selectableObject: null
	property bool highlightedIcon: false

	checkable: false

	readonly property QListView _view: (ListView.view instanceof QListView) ? ListView.view : null
	readonly property int _index: index

	leftSourceComponent: Qaterial.RoundColorIcon
	{
		id: _icon
		visible: source != ""
		iconSize: Qaterial.Style.delegate.iconWidth
		highlighted: true
		enabled: control.enabled

		fill: true
		width: roundIcon ? roundSize : iconSize
		height: roundIcon ? roundSize : iconSize

		source: _view && _view.selectEnabled ?
						 selectableObject && selectableObject.selected ?
							 Qaterial.Icons.checkCircle : Qaterial.Icons.checkBold : iconSource

		color: highlightedIcon || (_view && _view.selectEnabled) ? Qaterial.Style.accentColor : iconColor
	}

	width: ListView.view.width

	leftPadding: Math.max(!mirrored ? Qaterial.Style.delegate.leftPadding(control.type, control.lines) : Qaterial.Style.delegate
						  .rightPadding(control.type, control.lines), Client.safeMarginLeft)
	rightPadding: Math.max(mirrored ? Qaterial.Style.delegate.leftPadding(control.type, control.lines) : Qaterial.Style.delegate
						   .rightPadding(control.type, control.lines), Client.safeMarginRight)

	onClicked: if (selectArea.enabled) {
				   if (selectableObject) {
					   _view.currentIndex = index
					   selectableObject.selected = !selectableObject.selected
					   control.ListView.view.checkSelected()
				   }
			   } else if (setCurrentIndexEnabled && ListView.view)
				   ListView.view.currentIndex = index

	onPressAndHold: {
		if (_view && !_view.selectEnabled && _view.autoSelectChange) {
			_view.selectEnabled = true
			_view.currentIndex = index
			if (selectableObject)
				selectableObject.selected = true

			if (Qt.platform.os == "ios" || Qt.platform.os == "android") {
				var rr = mapToItem(_view, width/2, height/2)
				_view.rightClickOrPressAndHold(index, rr.x, rr.y)
			}

			return
		}

		if (_view) {
			_view.currentIndex = index
			var r = mapToItem(_view, width/2, height/2)
			_view.rightClickOrPressAndHold(index, r.x, r.y)
		}
	}

	Keys.onPressed: {
		if (event.key == Qt.Key_Enter || event.key == Qt.Key_Return) {
			down = true
			event.accepted = true
		}
	}

	Keys.onReleased: {
		if (event.key == Qt.Key_Enter || event.key == Qt.Key_Return) {
			down = false
			clicked()
			event.accepted = true
		}
	}

	MouseArea {
		id: selectArea
		anchors.fill: parent
		acceptedButtons: Qt.LeftButton
		enabled: _view && _view.selectEnabled
		onClicked: {
			if (mouse.modifiers & Qt.ShiftModifier) {
				var i1 = Math.min(Math.max(_view.currentIndex,0), index)
				var i2 = Math.max(Math.max(_view.currentIndex,0), index)
				for (var i=i1; i<=i2; ++i) {
					_view.modelGet(i).selected = true
				}
				_view.currentIndex = index
			} else if (selectableObject) {
				_view.currentIndex = index
				selectableObject.selected = !selectableObject.selected
				control.ListView.view.checkSelected()
			}
		}

		onPressAndHold: {
			var r = mapToItem(_view, mouse.x, mouse.y)
			_view.rightClickOrPressAndHold(index, r.x, r.y)
		}
	}
}
