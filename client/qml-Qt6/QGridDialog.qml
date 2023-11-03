import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.ModalDialog
{
	id: _root

	property var model: null
	property int currentIndex: -1

	property var delegate: defaultDelegate
	readonly property var defaultDelegate: _defaultDelegate

	property double maxHeight: Qaterial.Style.dialog.maxHeight

	property int cellWidth: 70 * Qaterial.Style.pixelSizeRatio
	property int cellHeight: 70 * Qaterial.Style.pixelSizeRatio


	dialogImplicitWidth: cellWidth*7

	Component
	{
		id: _defaultDelegate

		ItemDelegate {
			id: _delegate

			width: GridView.view.cellWidth
			height: GridView.view.cellWidth
			highlighted: GridView.isCurrentItem

			contentItem: Image {
				id: _content
				source: model.source ? model.source: ""
				fillMode: Image.PreserveAspectFit
				asynchronous: true
				horizontalAlignment: Image.AlignHCenter
				verticalAlignment: Image.AlignVCenter

				sourceSize.width: width
				sourceSize.height: height
			}

			background: Qaterial.ListDelegateBackground
			{
				id: _background
				type: Qaterial.Style.DelegateType.Icon
				lines: 1
				pressed: _delegate.pressed
				rippleActive: _delegate.down || _delegate.visualFocus || _delegate.hovered
				rippleAnchor: _delegate
				highlighted: _delegate.highlighted

				border.color: Qaterial.Style.accentColor
				border.width: _delegate.GridView.isCurrentItem ? 1 : 0
			}

			onClicked: GridView.view.select(index)
			onDoubleClicked: GridView.view.selectAndAccept(index)

		}
	}

	horizontalPadding: 0
	bottomPadding: 1
	drawSeparator: grid.height < grid.contentHeight

	standardButtons: DialogButtonBox.Ok | DialogButtonBox.Cancel
	contentItem: GridView {
		id: grid

		interactive: contentHeight > height

		implicitHeight: Math.min(_root.maxHeight, grid.contentHeight)

		model: _root.model

		cellHeight: _root.cellHeight
		cellWidth: _root.cellWidth

		delegate: _root.delegate

		highlightFollowsCurrentItem: true
		currentIndex: _root.currentIndex

		ScrollIndicator.vertical: ScrollIndicator { }

		clip: true

		Component.onCompleted: {
			if (currentIndex != -1)
				positionViewAtIndex(currentIndex, GridView.Contain)
		}

		function selectAndAccept(_index) {
			if (_index > -1) {
				currentIndex = _index
				_root.currentIndex = _index
				_root.accept()
			} else {
				_root.reject()
			}
		}

		function select(_index) {
			currentIndex = _index
			_root.currentIndex = _index
		}
	}
}
