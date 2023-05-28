import QtQuick 2.15
import QtQuick.Controls 2.15


Item {
	id: root

	property int cellWidth: 50
	property int cellHeight: 30
	property int firstRowHeight: 150
	property int firstColumnWidth: 150
	property int columnSpacing: 3
	property int rowSpacing: 2

	property alias model: _mainView.model
	property alias topHeaderDelegate: _topHeader.delegate
	property alias leftHeaderDelegate: _leftHeader.delegate
	property alias delegate: _mainView.delegate

	property alias cornerSource: _cornerLoader.source
	property alias cornerSourceComponent: _cornerLoader.sourceComponent

	property var columnWidthFunc: null
	property var rowHeightFunc: null

	implicitWidth: firstColumnWidth+columnSpacing
	implicitHeight: firstRowHeight+rowSpacing


	onCellHeightChanged: forceLayout()
	onCellWidthChanged: forceLayout()
	onFirstColumnWidthChanged: forceLayout()
	onFirstRowHeightChanged: forceLayout()
	onColumnSpacingChanged: forceLayout()
	onRowSpacingChanged: forceLayout()


	Item {
		id: _corner

		anchors.top: parent.top
		anchors.left: parent.left
		width: firstColumnWidth
		height: firstRowHeight

		Loader {
			id: _cornerLoader
			anchors.fill: parent
		}
	}

	TableView {
		id: _topHeader

		anchors.top: parent.top
		anchors.left: _corner.right
		anchors.leftMargin: root.columnSpacing
		anchors.right: parent.right
		height: firstRowHeight

		columnSpacing: root.columnSpacing
		rowSpacing: root.rowSpacing

		boundsBehavior: Flickable.StopAtBounds

		clip: true

		syncDirection: Qt.Horizontal
		syncView: _mainView

		model: _mainView.model

		flickableDirection: Flickable.HorizontalFlick

		columnWidthProvider: function (column) {
			if (column > 0)
				return (columnWidthFunc ? columnWidthFunc(column) : cellWidth)
			else
				return 0
		}

		rowHeightProvider: function (row) {
			if (row > 0)
				return 0
			else
				return firstRowHeight
		}

	}

	TableView {
		id: _leftHeader

		anchors.top: _corner.bottom
		anchors.topMargin: root.rowSpacing
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		width: firstColumnWidth

		boundsBehavior: Flickable.StopAtBounds

		flickableDirection: Flickable.VerticalFlick

		columnSpacing: root.columnSpacing
		rowSpacing: root.rowSpacing

		clip: true

		model: _mainView.model

		syncDirection: Qt.Vertical
		syncView: _mainView

		columnWidthProvider: function (column) {
			if (column > 0)
				return 0
			else
				return firstColumnWidth
		}

		rowHeightProvider: function (row) {
			if (row > 0)
				return (rowHeightFunc ? rowHeightFunc(row) : cellHeight)
			else
				return 0
		}
	}



	TableView {
		id: _mainView

		anchors.top: _topHeader.bottom
		anchors.topMargin: root.rowSpacing
		anchors.bottom: parent.bottom
		anchors.right: parent.right
		anchors.left: _leftHeader.right
		anchors.leftMargin: root.columnSpacing

		columnSpacing: root.columnSpacing
		rowSpacing: root.rowSpacing

		clip: true

		boundsBehavior: Flickable.StopAtBounds
		ScrollIndicator.vertical: ScrollIndicator {
			active: _mainView.flickingVertically || _leftHeader.movingVertically
		}
		ScrollIndicator.horizontal: ScrollIndicator {
			active: _mainView.flickingHorizontally || _topHeader.movingHorizontally
		}

		columnWidthProvider: function (column) {
			if (column > 0)
				return (columnWidthFunc ? columnWidthFunc(column) : cellWidth)
			else
				return 0
		}

		rowHeightProvider: function (row) {
			if (row > 0)
				return (rowHeightFunc ? rowHeightFunc(row) : cellHeight)
			else
				return 0
		}
	}


	function forceLayout() {
		_topHeader.forceLayout()
		_leftHeader.forceLayout()
		_mainView.forceLayout()
	}


}
