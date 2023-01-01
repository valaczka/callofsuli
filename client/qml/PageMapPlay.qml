import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

QScrollablePage {
	id: control

	property MapPlay map: null

	property SortFilterProxyModel mList: SortFilterProxyModel {
		sourceModel: map ? map.missionList : []

		sorters: StringSorter { roleName: "name"; priority: 0 ; sortOrder: Qt.DescendingOrder }
	}


	Column {
		Repeater {
			model: mList

			Column {
				id: missionColumn

				//required property string name
				//required property var levels



				Qaterial.LabelHeadline3 {
					text: name
				}

				ListView {
					id: view
					model: levels
					orientation: ListView.Horizontal

					implicitHeight: 220

					width: control.width
					//height: 200//medalButtonSize
					spacing: 5

					clip: true

					header: Item {
						width: Client.safeMarginLeft
						height: view.height
					}

					footer: Item {
						width: Client.safeMarginRight
						height: view.height
					}

					delegate: Qaterial.Card {
						width: 200
						height: 200

						property QtObject foo: model.qtObject

						pressed: area.pressed

						contentItem: Item {
							width: parent.width
							height: lay.height
							ColumnLayout {
								id: lay
								width: parent.width
								spacing: Qaterial.Style.card.verticalPadding

								Qaterial.LabelHeadline5
								{
									text: foo.level
									elide: Text.ElideRight
									//padding: Qaterial.Style.card.horizontalPadding
									Layout.leftMargin: Qaterial.Style.card.horizontalPadding
									Layout.rightMargin: Qaterial.Style.card.horizontalPadding
									Layout.fillWidth: true
									////Layout.topMargin: Qaterial.Style.card.horizontalPadding
								} // Label


								Qaterial.CardSupportingText
								{
									supportingText: foo.deathmatch
									Layout.leftMargin: Qaterial.Style.card.horizontalPadding
									Layout.rightMargin: Qaterial.Style.card.horizontalPadding
									Layout.topMargin: 2
									Layout.bottomMargin: 2
									Layout.fillWidth: true
								} // CardSupportingText
							}

							MouseArea {
								id: area
								anchors.fill: parent
								acceptedButtons: Qt.LeftButton

								onClicked: map.play(foo)

							}

						} // ColumnLayout

						/*Image
								{
									source: _control.media
									Layout.maximumWidth: Qaterial.Style.dense ? 64 : 80
									Layout.maximumHeight: Qaterial.Style.dense ? 64 : 80
									Layout.rightMargin: Qaterial.Style.card.horizontalPadding
								} // Image*/
					} // RowLayout

				}


			}


		}
	}



	//StackView.onActivated: Client.loadGame()
}
