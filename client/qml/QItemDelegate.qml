import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

Qaterial.ItemDelegate {
	id: control

	property bool setCurrentIndexEnabled: true
	property url iconSource: ""						// Ezt használjuk az icon.source helyett
	property SelectableObject selectableObject: null

	checkable: false

	readonly property QListView _view: (ListView.view instanceof QListView) ? ListView.view : null


	icon.source: _view && _view.selectEnabled ?
					 selectableObject && selectableObject.selected ? Qaterial.Icons.checkCircle : Qaterial.Icons.checkBold
	: iconSource


	width: ListView.view.width
	iconColor: highlightedIcon ? Qaterial.Style.accentColor : Qaterial.Style.iconColor()

	onClicked: if (selectArea.enabled) {
				   if (selectableObject) {
					   _view.currentIndex = index
					   selectableObject.selected = !selectableObject.selected
					   control.ListView.view.checkSelected()
				   }
			   } else if (setCurrentIndexEnabled)
				   ListView.view.currentIndex = index

	onPressAndHold: {
		if (_view && !_view.selectEnabled && _view.autoSelectChange) {
			_view.selectEnabled = true
			_view.currentIndex = index
			if (selectableObject)
				selectableObject.selected = true
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
					_view.model.get(i).selected = true
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
