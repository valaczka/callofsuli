import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QTabContainer {
	id: control

	title: qsTr("Részletek")
	icon: "image://font/Academic/\uf118"


	QAccordion {
		id: accordion

		visible: teacherMaps.selectedMapId != ""

		QTabHeader {
			tabContainer: control
			isPlaceholder: true
		}

		Column {
			id: col
			width: parent.width

			spacing: 10

			topPadding: 20
			bottomPadding: 20

			QLabel {
				id: labelName
				font.pixelSize: CosStyle.pixelSize*1.7
				font.weight: Font.Normal
				color: CosStyle.colorAccentLight

				anchors.horizontalCenter: parent.horizontalCenter
				horizontalAlignment: Text.AlignHCenter

				wrapMode: Text.Wrap
				width: Math.min(implicitWidth, col.width)
			}

			QLabel {
				id: labelInfo
				anchors.horizontalCenter: parent.horizontalCenter
				font.pixelSize: CosStyle.pixelSize
				font.weight: Font.Normal
				color: CosStyle.colorPrimaryLighter

				horizontalAlignment: Text.AlignHCenter

				wrapMode: Text.Wrap
				width: Math.min(implicitWidth, col.width)

				bottomPadding: 20

				textFormat: Text.StyledText
			}

			QButton {
				id: btnRename
				text: qsTr("Átnevezés")
				icon.source: CosStyle.iconRename
				display: AbstractButton.TextBesideIcon
				anchors.horizontalCenter: parent.horizontalCenter

				onClicked: {
					var d = JS.dialogCreateQml("TextField", {
												   title: qsTr("Pálya átnevezése"),
												   text: qsTr("Pálya neve:"),
												   value: labelName.text
											   })

					d.accepted.connect(function(data) {
						if (data.length)
							teacherMaps.send("mapModify", {uuid: teacherMaps.selectedMapId, name: data})
					})
					d.open()
				}



			}

			QButton {
				id: btnExport
				text: qsTr("Mentés fájlba")
				icon.source: CosStyle.iconSave
				display: AbstractButton.TextBesideIcon
				visible: !btnDownload.visible
				anchors.horizontalCenter: parent.horizontalCenter

				onClicked:  {
					var d = JS.dialogCreateQml("File", {
												   isSave: true,
												   folder: cosClient.getSetting("mapFolder", ""),
												   text: labelName.text+".map"
											   })
					d.accepted.connect(function(data){
						teacherMaps.mapExport(data)
						cosClient.setSetting("mapFolder", d.item.modelFolder)
					})

					d.open()
				}
			}

			QButton {
				id: btnDownload
				text: qsTr("Letöltés")
				icon.source: CosStyle.iconDownload
				display: AbstractButton.TextBesideIcon
				anchors.horizontalCenter: parent.horizontalCenter

				onClicked: teacherMaps.mapDownload({ uuid: teacherMaps.selectedMapId })
			}

			QButton {
				id: btnOverride
				text: qsTr("Felülírás")
				icon.source: CosStyle.iconRefresh
				display: AbstractButton.TextBesideIcon
				anchors.horizontalCenter: parent.horizontalCenter

				onClicked: {
					var dd = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan felül akarod írni a pályát?\n%1").arg(labelName.text)})
					dd.accepted.connect(function() {
						var d = JS.dialogCreateQml("File", {
													   isSave: false,
													   folder: cosClient.getSetting("mapFolder", ""),
													   title: qsTr("Felülírás")
												   })
						d.accepted.connect(function(data){
							teacherMaps.mapOverride(data)
							cosClient.setSetting("mapFolder", d.item.modelFolder)
						})

						d.open()
					})
					dd.open()


				}
			}

			QButton {
				id: btnDelete
				text: qsTr("Törlés")
				themeColors: CosStyle.buttonThemeRed
				icon.source: CosStyle.iconDelete
				display: AbstractButton.TextBesideIcon
				anchors.horizontalCenter: parent.horizontalCenter

				onClicked: {
					var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan törlöd a pályát a szerverről?\n%1").arg(labelName.text)})
					d.accepted.connect(function() {
						teacherMaps.send("mapRemove", {uuid: teacherMaps.selectedMapId})
					})
					d.open()
				}
			}

		}


		QCollapsible {
			id: colMissions
			title: qsTr("Küldetések")

			Column {
				Repeater {
					id: rptrMission
					model: []

					QLabel {
						width: colMissions.width
						elide: Text.ElideRight
						font.weight: Font.DemiBold
						color: CosStyle.colorAccentLight
						text: modelData
						leftPadding: 15
						rightPadding: 15
					}
				}
			}
		}

		QCollapsible {
			id: colGroups
			title: qsTr("Hozzárendelt csoportok")

			Column {
				Repeater {
					id: rptrGroup
					model: []

					QLabel {
						width: colMissions.width
						elide: Text.ElideRight
						text: "%1 (%2)".arg(modelData.name).arg(modelData.readableClassList)
						leftPadding: 15
						rightPadding: 15
					}
				}
			}
		}
	}

	QIconEmpty {
		visible: !accordion.visible
		text: qsTr("Válassz pályát")
		anchors.centerIn: parent
		textWidth: parent.width*0.75
		tabContainer: control
	}


	Connections {
		target: teacherMaps

		function onMapDataReady(_map, _list, _ready) {
			if (!_map) {
				labelName.text = ""
				labelInfo.text = ""
				rptrGroup.model = []
				rptrMission.model = []
				btnDownload.visible = true
				return
			}

			labelName.text = _map.name
			labelInfo.text = qsTr("Verzió: <b>%1</b><br>Méret: <b>%2</b><br>Utoljára módosítva: <b>%3</b><br>Eddigi játékok száma: <b>%4</b>").arg(_map.version)
			.arg(cosClient.formattedDataSize(Number(_map.dataSize)))
			.arg(JS.readableTimestamp(_map.lastModified))
			.arg(_map.used)

			rptrGroup.model = _map.binded

			btnDownload.visible = !_map.downloaded

			if (_ready) {
				rptrMission.model = _list
			} else {
				rptrMission.model = []
			}
		}
	}

	onPopulated: teacherMaps.getSelectedMapInfo()
}

