import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
	id: control

	title: qsTr("Osztályok")
	icon: CosStyle.iconGroups

	property real buttonSize: CosStyle.twoLineHeight*3
	readonly property real _contentWidth: Math.min(width-10, 600)

	Component {
		id: componentUsers
		AdminUserList { }
	}

	Flickable {
		id: flickable
		width: Math.min(parent.width, contentWidth)
		height: Math.min(parent.height, contentHeight)
		anchors.centerIn: parent

		contentWidth: col.width
		contentHeight: col.height

		clip: true

		flickableDirection: Flickable.VerticalFlick
		boundsBehavior: Flickable.StopAtBounds

		ScrollIndicator.vertical: ScrollIndicator { }

		Column {
			id: col

			QTabHeader {
				width: grid.width
				tabContainer: control
				isPlaceholder: true
				visible: grid.height > control.height-2*height
			}

			Grid {
				id: grid
				columns: Math.floor(_contentWidth/(buttonSize+spacing))
				spacing: buttonSize*0.2

				bottomPadding: 20

				QCard {
					id: adminItem
					height: buttonSize
					width: buttonSize

					backgroundColor: "#940000"

					onClicked: {
						serverSettings.modelUserList.resetJsonArray()
						tabPage.pushContent(componentUsers, {
												contentTitle: qsTr("Adminok"),
												queryParameters: {
													isAdmin: true
												}
											})

					}

					Column {
						anchors.centerIn: parent

						QFontImage {
							anchors.horizontalCenter: parent.horizontalCenter
							size: addItem.width*0.5
							color: "white"
							icon: "image://font/AcademicI/\uf1ec"
						}

						QLabel {
							color: "white"
							font.weight: Font.DemiBold
							width: addItem.width*0.8
							horizontalAlignment: Text.AlignHCenter
							wrapMode: Text.Wrap
							maximumLineCount: 2
							elide: Text.ElideRight
							font.capitalization: Font.AllUppercase
							text: qsTr("Adminok")
						}

					}
				}

				QCard {
					id: teacherItem
					height: buttonSize
					width: buttonSize

					backgroundColor: "#996202"

					onClicked: {
						serverSettings.modelUserList.resetJsonArray()
						tabPage.pushContent(componentUsers, {
												contentTitle: qsTr("Tanárok"),
												queryParameters: {
													isTeacher: true
												}
											})
					}

					Column {
						anchors.centerIn: parent

						QFontImage {
							anchors.horizontalCenter: parent.horizontalCenter
							size: addItem.width*0.5
							color: "white"
							icon: "image://font/Academic/\uf213"
						}

						QLabel {
							color: "white"
							font.weight: Font.DemiBold
							width: addItem.width*0.8
							horizontalAlignment: Text.AlignHCenter
							wrapMode: Text.Wrap
							maximumLineCount: 2
							elide: Text.ElideRight
							font.capitalization: Font.AllUppercase
							text: qsTr("Tanárok")
						}

					}
				}


				QCard {
					id: emptyItem
					height: buttonSize
					width: buttonSize

					backgroundColor: "#708090"

					onClicked: {
						serverSettings.modelUserList.resetJsonArray()
						tabPage.pushContent(componentUsers, {
												contentTitle: qsTr("Osztály nélküli diákok"),
												queryParameters: {
													classid: -1
												}
											})
					}

					Column {
						anchors.centerIn: parent

						QFontImage {
							anchors.horizontalCenter: parent.horizontalCenter
							size: addItem.width*0.5
							color: "white"
							icon: "image://font/AcademicI/\uf1b4"
						}

						QLabel {
							color: "white"
							font.weight: Font.DemiBold
							width: addItem.width*0.8
							horizontalAlignment: Text.AlignHCenter
							wrapMode: Text.Wrap
							maximumLineCount: 2
							elide: Text.ElideRight
							font.capitalization: Font.AllUppercase
							text: qsTr("Osztály nélkül")
						}

					}
				}

				Repeater {
					id: groupRepeater

					QCard {
						id: groupItem
						height: buttonSize
						width: buttonSize

						backgroundColor: "#BA55D3"

						onClicked: {
							serverSettings.modelUserList.resetJsonArray()
							tabPage.pushContent(componentUsers, {
													contentTitle: modelData.name,
													queryParameters: {
														classid: modelData.id
													}
												})
						}

						Column {
							anchors.centerIn: parent

							QFontImage {
								anchors.horizontalCenter: parent.horizontalCenter
								size: groupItem.width*0.5
								color: "white"
								icon: CosStyle.iconGroups
							}

							QLabel {
								color: "white"
								font.weight: Font.DemiBold
								width: groupItem.width*0.8
								horizontalAlignment: Text.AlignHCenter
								wrapMode: Text.Wrap
								maximumLineCount: 2
								elide: Text.ElideRight
								font.capitalization: Font.AllUppercase
								text: modelData.name
							}

						}
					}
				}

				QCard {
					id: addItem
					height: buttonSize
					width: buttonSize

					backgroundColor: CosStyle.colorOKDarker

					onClicked: {
						var d = JS.dialogCreateQml("TextField", { title: qsTr("Új osztály létrehozása"), text: qsTr("Új osztály neve:") })

						d.accepted.connect(function(data) {
							if (data.length && profile)
								serverSettings.send("classCreate", {name: data})
						})
						d.open()
					}

					Column {
						anchors.centerIn: parent

						QFontImage {
							anchors.horizontalCenter: parent.horizontalCenter
							size: addItem.width*0.5
							color: "white"
							icon: CosStyle.iconAdd
						}

						QLabel {
							color: "white"
							font.weight: Font.DemiBold
							width: addItem.width*0.8
							horizontalAlignment: Text.AlignHCenter
							wrapMode: Text.Wrap
							maximumLineCount: 2
							elide: Text.ElideRight
							font.capitalization: Font.AllUppercase
							text: qsTr("Új osztály létrehozása")
						}

					}
				}

			}

		}
	}



	onPopulated: {
		serverSettings.send("getAllClass")
	}


	Connections {
		target: serverSettings

		function onGetAllClass(jsonData, binaryData) {
			groupRepeater.model = jsonData.list
		}

		function onClassCreate(jsonData, binaryData) {
			if (jsonData.created) {
				serverSettings.send("getAllClass")
			}
		}
	}


}
