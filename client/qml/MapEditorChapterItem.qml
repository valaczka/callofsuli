import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import SortFilterProxyModel 0.2
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

Qaterial.Expandable {
	id: root

	property MapEditorChapter chapter: null
	property bool separatorVisible: true

	header: Item {
		width: root.width
		height: _headerRow.height

		RowLayout {
			id: _headerRow
			width: parent.width
			spacing: 5

			Qaterial.RoundButton {
				icon.source: root.expanded ? Qaterial.Icons.plus : Qaterial.Icons.minus
				Layout.alignment: Qt.AlignCenter
				onClicked: root.expanded = !root.expanded
			}

			Qaterial.LabelHeadline6 {
				text: chapter ? chapter.name : ""
				elide: Text.ElideRight
				color: Qaterial.Style.accentColor
				Layout.fillWidth: true
				Layout.fillHeight: true
				Layout.alignment: Qt.AlignCenter

				verticalAlignment: Text.AlignVCenter

				MouseArea {
					anchors.fill: parent
					acceptedButtons: Qt.LeftButton
					onClicked: root.expanded = !root.expanded
				}
			}

			QBanner {
				num: chapter ? chapter.objectiveCount : 0
				visible: num > 0
				color: Qaterial.Colors.cyan900
				Layout.alignment: Qt.AlignCenter
			}

			Qaterial.RoundButton {
				icon.source: Qaterial.Icons.dotsVertical
				Layout.alignment: Qt.AlignCenter
			}
		}

		Qaterial.HorizontalLineSeparator {
			width: parent.width
			anchors.bottom: parent.bottom
			visible: root.separatorVisible
		}

	}


	delegate: QIndentedItem {
		width: root.width
		QListView {
			width: parent.width
			height: contentHeight
			boundsBehavior: Flickable.StopAtBounds

			autoSelectChange: true

			model: chapter ? chapter.objectiveList : null

			delegate: MapEditorObjectiveItem {
				objective: model.qtObject
				width: ListView.view.width
			}
		}
	}
}
