import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

Qaterial.LoaderItemDelegate {
	id: control

	property bool setCurrentIndexEnabled: true
	property url iconSource: ""									// Ezt használjuk az icon.source helyett
	property color iconColorBase: Qaterial.Style.iconColor()	// Ezt használjuk az iconColor helyett
	property SelectableObject selectableObject: null
	property bool highlightedIcon: false

	checkable: false

	readonly property QListView _view: (ListView.view instanceof QListView) ? ListView.view : null


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

	Keys.onPressed: event => {
						if (event.key == Qt.Key_Enter || event.key == Qt.Key_Return) {
							down = true
							event.accepted = true
						}
					}

	Keys.onReleased: event => {
						 if (event.key == Qt.Key_Enter || event.key == Qt.Key_Return) {
							 down = false
							 clicked()
							 event.accepted = true
						 }
					 }

	leftSourceComponent: Qaterial.RoundColorIcon
	{
		source: _view && _view.selectEnabled ?
					selectableObject && selectableObject.selected ? Qaterial.Icons.checkCircle : Qaterial.Icons.checkBold
		: iconSource
		color: highlightedIcon || (_view && _view.selectEnabled) ? Qaterial.Style.accentColor : iconColorBase
		iconSize: Qaterial.Style.delegate.iconWidth

		fill: true
		width: roundIcon ? roundSize : iconSize
		height: roundIcon ? roundSize : iconSize
	}


	MouseArea {
		id: selectArea
		anchors.fill: parent
		acceptedButtons: Qt.LeftButton
		enabled: _view && _view.selectEnabled
		onClicked: mouse => {
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

		onPressAndHold: mouse => {
							var r = mapToItem(_view, mouse.x, mouse.y)
							_view.rightClickOrPressAndHold(index, r.x, r.y)
						}
	}

}
