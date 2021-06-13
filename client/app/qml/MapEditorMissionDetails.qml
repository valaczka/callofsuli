import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QSwipeContainer {
	id: container

	title: qsTr("Adatok")
	icon: CosStyle.iconSetup

	QAccordion {
		QCollapsible {
			title: qsTr("Alapadatok")

			QGridLayout {
				width: parent.width
				watchModification: false

				QGridImageTextField {
					id: imageTextName
					fieldName: qsTr("Küldetés neve")

					textfield.font.pixelSize: CosStyle.pixelSize*1.3
					textfield.textColor: CosStyle.colorAccentLighter
					textfield.onTextModified: mapEditor.missionModify({name: textfield.text})

					property string _cv: ""

					image: _cv.length ? "qrc:/internal/medals/"+_cv : ""

					mousearea.onClicked:  {
						var d = JS.dialogCreateQml("ImageGrid", {
													   model: cosClient.medalIcons(),
													   icon: CosStyle.iconMedal,
													   title: qsTr("Küldetés medálképe"),
													   modelImagePattern: "qrc:/internal/medals/%1",
													   clearEnabled: false,
													   currentValue: _cv
												   })

						d.accepted.connect(function(data) {
							mapEditor.missionModify({medalImage: data})
						})

						d.open()
					}
				}

				QGridLabel {
					field: areaDetails
				}

				QGridTextArea {
					id: areaDetails
					fieldName: qsTr("Leírás")
					placeholderText: qsTr("Rövid ismertető leírás a küldetésről")
					background: Item {
						implicitWidth: 50
						implicitHeight: CosStyle.baseHeight*2
					}

					onTextModified: mapEditor.missionModify({description: text})
				}
			}
		}

		QCollapsible {
			title: qsTr("Zárolások")
		}

		QToolButtonFooter {
			anchors.horizontalCenter: parent.horizontalCenter
			color: CosStyle.colorErrorLighter
			icon.source: CosStyle.iconDelete
			text: qsTr("Küldetés törlése")

			onClicked: mapEditor.missionRemove()
		}

	}


	Connections {
		target: mapEditor

		function onCurrentMissionDataChanged(data) {
			imageTextName._cv = data.medalImage ? data.medalImage : ""
			imageTextName.textfield.text = data.name ? data.name : ""
			areaDetails.text = data.description ? data.description: ""
		}
	}
}
