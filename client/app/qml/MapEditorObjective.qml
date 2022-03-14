import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Item {
	id: control

	required property GameMapEditorChapter chapter
	property GameMapEditorObjective objective: null
	property bool duplicate: false

	property string objectiveModule: ""
	property int storageId: -1
	property string storageModule: ""
	property int storageCount: 0
	property var objectiveData: null
	property var storageData: null


	property Drawer drawer: parent.drawer
	property MapEditor mapEditor: parent.mapEditor

	property ListModel availableObjectiveModel: ListModel {}
	property ListModel availableStorageModel: ListModel {}

	property var _availableStorageModules: []

	StackView {
		id: stack
		anchors.top: parent.top
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: buttonRow.top
		anchors.margins: 1
	}


	Row {
		id: buttonRow
		spacing: 10

		anchors.bottom: parent.bottom
		anchors.horizontalCenter: parent.horizontalCenter

		QButton {
			id: buttonNo
			anchors.verticalCenter: parent.verticalCenter
			text: qsTr("Mégsem")
			icon.source: CosStyle.iconCancel
			themeColors: CosStyle.buttonThemeRed

			onClicked: drawer.close()
		}

		QButton {
			id: buttonYes

			anchors.verticalCenter: parent.verticalCenter

			visible: false

			text: qsTr("OK")
			icon.source: CosStyle.iconOK
			themeColors: CosStyle.buttonThemeGreen

			onClicked: {
				save()
				drawer.close()
			}
		}
	}



	Component {
		id: cmpObjectiveModules

		QObjectListView {
			id: list

			model: SortFilterProxyModel {
				sourceModel: availableObjectiveModel

				sorters: StringSorter {
					roleName: "name"
				}
			}

			modelTitleRole: "name"

			autoSelectorChange: false

			leftComponent: QFontImage {
				width: list.delegateHeight+10
				height: list.delegateHeight
				size: height*0.85
				icon: model.icon
			}

			onClicked: {
				var d = model.get(index)

				objectiveModule = d.module
				_availableStorageModules = []

				var i=0

				while (true) {
					if (!d.storageModules[i])
						break

					_availableStorageModules.push(d.storageModules[i])

					i++
				}

				if (_availableStorageModules.length) {
					stack.replace(cmpStorages)
				} else {
					storageId = -1
					stack.replace(cmpEdit)
				}
			}
		}

	}



	Component {
		id: cmpStorages

		QListView {
			id: slist

			model: SortFilterProxyModel {
				sourceModel: availableStorageModel

				filters: ExpressionFilter {
					expression: _availableStorageModules.includes(model.module)
				}

				proxyRoles: [
					SwitchRole {
						name: "isNew"
						defaultValue: false
						filters: ExpressionFilter {
							expression: model.id < 0
							SwitchRole.value: true
						}
					}
				]

				sorters: [
					FilterSorter {
						filters: ValueFilter { roleName: "isNew"; value: true }
						priority: 2
					},
					StringSorter {
						roleName: "name"
						priority: 1
					},
					RoleSorter {
						roleName: "id"
					}
				]

			}

			delegate: Item {
				id: item
				width: slist.width
				height: CosStyle.twoLineHeight*1.7

				required property int id
				required property string module
				required property string name
				required property string icon
				required property string title
				required property string details
				required property int objectiveCount
				required property var storageData
				required property bool isNew


				QRectangleBg {
					anchors.fill: parent
					acceptedButtons: Qt.LeftButton

					Item {
						id: rect
						anchors.fill: parent
						anchors.leftMargin: 5
						anchors.rightMargin: 5


						QLabel {
							id: labelName
							text: item.name
							color: CosStyle.colorPrimary
							font.weight: Font.DemiBold
							font.pixelSize: CosStyle.pixelSize*0.6
							font.capitalization: Font.AllUppercase
							anchors.left: parent.left
							anchors.top: parent.top
							anchors.leftMargin: 3
							anchors.topMargin: 1
						}

						Row {
							anchors.verticalCenter: parent.verticalCenter

							spacing: 0

							QFontImage {
								id: imgModule
								width: Math.max(rect.height*0.5, size*1.1)
								size: CosStyle.pixelSize*1.5
								anchors.verticalCenter: parent.verticalCenter
								icon: item.icon
								color: item.isNew ? CosStyle.colorAccent : CosStyle.colorPrimary
							}


							Column {
								anchors.verticalCenter: parent.verticalCenter
								anchors.verticalCenterOffset: (labelName.height/2)*(subtitle.lineCount-1)/3

								QLabel {
									id: title
									anchors.left: parent.left
									width: rect.width-imgModule.width
										   -(badge.visible ? badge.width : 0)
									text: item.title
									color: item.isNew ? CosStyle.colorAccentLighter : CosStyle.colorPrimaryLighter
									font.pixelSize: CosStyle.pixelSize*1.1
									font.weight: Font.Normal
									maximumLineCount: 1
									lineHeight: 0.9
									elide: Text.ElideRight
								}
								QLabel {
									id: subtitle
									anchors.left: parent.left
									width: title.width
									text: item.details
									color: CosStyle.colorPrimary
									font.pixelSize: CosStyle.pixelSize*0.75
									font.weight: Font.Light
									maximumLineCount: 3
									lineHeight: 0.8
									wrapMode: Text.Wrap
									elide: Text.ElideRight
								}
							}

							QBadge {
								id: badge
								text: item.objectiveCount
								color: CosStyle.colorWarningDark
								anchors.verticalCenter: parent.verticalCenter
								visible: item.objectiveCount > 0
							}

						}
					}


					mouseArea.onClicked: {
						storageId = item.id > 0 ? item.id : -1
						control.storageData = item.storageData
						storageModule = item.module
						stack.replace(cmpEdit)
					}
				}

				Rectangle {
					anchors.top: parent.top
					anchors.left: parent.left
					width: parent.width
					height: 1
					color: CosStyle.colorPrimaryDark
				}
			}

			header: QToolButtonFooter {
				icon.source: CosStyle.iconAdd
				text: qsTr("Előállító nélkül")
				width: slist.width
				height: CosStyle.twoLineHeight*1.7

				onClicked: {
					storageId = -1
					control.storageData = null
					storageModule = ""
					stack.replace(cmpEdit)
				}
			}
		}
	}


	Component {
		id: cmpEdit

		QAccordion {
			anchors.fill: undefined

			Loader {
				id: storageLoader
				width: parent.width
			}

			Loader {
				id: objectiveLoader
				width: parent.width
			}

			StackView.onActivated: {
				buttonYes.visible = true
			}

			Component.onCompleted: {
				if (storageModule != "") {
					var q = mapEditor.storageQml(storageModule)
					storageLoader.setSource(q, {
												moduleData: control.storageData
											})

				}

				if (objectiveModule != "") {
					var q2 = mapEditor.objectiveQml(objectiveModule)
					objectiveLoader.setSource(q2, {
												  moduleData: objectiveData,
												  storageData: control.storageData,
												  storageModule: storageModule,
												  storageCount: storageCount
											  })
				}
			}


			Connections {
				target: storageLoader.item

				function onModuleDataChanged(d) {
					control.storageData = d
				}
			}

			Connections {
				target: control

				function onStorageDataChanged(sd) {
					if (objectiveLoader.status == Loader.Ready)
						objectiveLoader.item.setStorageData(control.storageData)
				}
			}



			function getObjectiveData() {
				if (objectiveLoader.status == Loader.Ready)
					return objectiveLoader.item.getData()

				return ""
			}

			function getStorageData() {
				if (storageLoader.status == Loader.Ready)
					return storageLoader.item.getData()

				return ""
			}

			function getStorageCount() {
				if (objectiveLoader.status == Loader.Ready)
					return objectiveLoader.item.storageCount

				return 0
			}
		}
	}



	Component.onCompleted: {
		var l = mapEditor.availableObjectives

		for (var i=0; i<l.length; i++) {
			availableObjectiveModel.append(l[i])
		}


		var sl = mapEditor.getStorages()

		for (var j=0; j<sl.length; j++) {
			availableStorageModel.append(sl[j])
		}

		if (objective) {
			objectiveModule = objective.module
			storageId = objective.storageId
			storageModule = objective.storageModule
			storageCount = objective.storageCount
			objectiveData = objective.data
			control.storageData = objective.storageData

			stack.replace(cmpEdit)
		} else {
			stack.replace(cmpObjectiveModules)
		}
	}


	function save() {
		var odata = stack.currentItem.getObjectiveData()
		var sdata = stack.currentItem.getStorageData()
		var sc = stack.currentItem.getStorageCount()

		if (objective && !duplicate) {
			mapEditor.objectiveModify(chapter, objective,
									  {
										  data: odata,
										  storageCount: sc
									  },
									  sdata)
		} else {
			mapEditor.objectiveAdd(chapter,
								   {
									   module: objectiveModule,
									   data: odata,
									   storageCount: sc
								   },
								   {
									   id: storageId,
									   module: storageModule,
									   data: sdata
								   })

		}
	}

}
