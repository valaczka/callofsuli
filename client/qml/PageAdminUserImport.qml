import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import SortFilterProxyModel 0.2
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: control

	stackPopFunction: function() {
		if (_stack._step > 0) {
			_stack._step--
			return false
		}

		return true
	}

	title: qsTr("Felhasználók importálása")
	subtitle: Client.server ? Client.server.serverName : ""

	appBar.backButtonVisible: true


	property int classId: -1

	property ClassList _classList: Client.cache("classList")

	property list<Component> _components


	UserImporter {
		id: _importer

		property bool hasData: false

		onLoadStarted: {
			hasData = false
			_stack._step = 1
		}

		onLoadFinished: hasData = true

		onEmptyDocument: {
			hasData = true
			Client.messageWarning(qsTr("A dokumentum tartalma nem importálható"), qsTr("Érvénytelen dokumentum"))
		}
	}


	StackView {
		id: _stack

		property int _step: 0

		anchors.fill: parent

		on_StepChanged: {
			replace(_components[_step])
		}
	}


	Component {
		id: _cmpStart

		QScrollable {
			contentCentered: true
			refreshEnabled: false

			Column {
				spacing: 20

				width: parent.width


				QLabelInformative {
					text: qsTr("(1) Válaszd ki, melyik osztályba kerüljenek az importált felhasználók")
				}

				QFormComboBox {
					id: _map
					text: qsTr("Osztály:")

					anchors.horizontalCenter: parent.horizontalCenter

					combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 300*Qaterial.Style.pixelSizeRatio))

					valueRole: "classid"
					textRole: "name"

					model: sortedClassList

					combo.onActivated: control.classId = combo.currentValue

					Component.onCompleted: {
						_preparedClassList.reloaded.connect(selectCurrent)
					}

					function selectCurrent() {
						let i = combo.indexOfValue(control.classId)

						if (i !== -1)
							currentIndex = i
						else
							currentIndex = 0
					}
				}

				QLabelInformative {
					text: qsTr("(2/a) Tölts le egy üres sablont a kitöltéshez")
				}


				Qaterial.OutlineButton {
					anchors.horizontalCenter: parent.horizontalCenter
					highlighted: false
					text: qsTr("Sablon letöltése")
					icon.source: Qaterial.Icons.fileDocumentOutline
					onClicked: {
						if (Qt.platform.os == "wasm")
							_importer.wasmDownloadTemplate()
						else
							Qaterial.DialogManager.openFromComponent(_cmpFileSave)
					}
				}


				QLabelInformative {
					text: qsTr("(2/b) Töltsd fel az importálandó táblázatot")
				}

				QButton {
					anchors.horizontalCenter: parent.horizontalCenter
					highlighted: true
					text: qsTr("Feltöltés")
					icon.source: Qaterial.Icons.upload
					onClicked: {
						if (Qt.platform.os == "wasm")
							_importer.wasmUpload()
						else
							Qaterial.DialogManager.openFromComponent(_cmpFileOpen)

					}
				}

			}
		}
	}


	Component {
		id: _cmpSelect

		QScrollable {
			contentCentered: true
			refreshEnabled: false

			Row {
				visible: !_importer.hasData
				spacing: 20 * Qaterial.Style.pixelSizeRatio

				anchors.horizontalCenter: parent.horizontalCenter

				Qaterial.BusyIndicator {
					anchors.verticalCenter: parent.verticalCenter
				}

				Qaterial.LabelBody1 {
					anchors.verticalCenter: parent.verticalCenter
					text: qsTr("Feldolgozás...")
				}
			}

			Column {
				visible: _importer.hasData

				width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
				anchors.horizontalCenter: parent.horizontalCenter

				readonly property bool _hasErrors: _importer.errorRecords.length
				readonly property bool _hasRecords: _importer.records.length

				Qaterial.LabelHeadline4 {
					anchors.horizontalCenter: parent.horizontalCenter
					visible: parent._hasErrors
					text: qsTr("Hibás rekordok (%1)").arg(_importer.errorRecords.length)
					bottomPadding: 3 * Qaterial.Style.pixelSizeRatio
				}


				Repeater {
					model: _importer.errorRecords

					delegate: Column {
						width: parent.width

						bottomPadding: 3 * Qaterial.Style.pixelSizeRatio

						Qaterial.LabelHeadline6 {
							width: parent.width
							wrapMode: Text.Wrap
							text: modelData.username
							color: Qaterial.Style.accentColor
						}

						Qaterial.LabelCaption {
							width: parent.width
							leftPadding: 30 * Qaterial.Style.pixelSizeRatio
							wrapMode: Text.Wrap
							color: Qaterial.Colors.red400
							text: qsTr("Hiba: %1 (%2. sor)").arg(modelData.error.join(", ")).arg(modelData.errorRow)
						}

						Qaterial.LabelBody1 {
							width: parent.width
							leftPadding: 30 * Qaterial.Style.pixelSizeRatio
							wrapMode: Text.Wrap
							text: {
								let k = Object.keys(modelData)
								let t = ""
								for (let i=0; i<k.length; ++i) {
									let key = k[i]
									if (key === "username" || key === "error" || key === "errorRow")
										continue

									if (t != "") t += "<br>"

									t += key+": <b>"+modelData[key]+"</b>"
								}

								return t
							}
						}

					}
				}


				Qaterial.LabelHeadline4 {
					anchors.horizontalCenter: parent.horizontalCenter
					visible: parent._hasRecords
					text: qsTr("Importálható rekordok (%1)").arg(_importer.records.length)
					bottomPadding: 3 * Qaterial.Style.pixelSizeRatio
				}


				Repeater {
					model: _importer.records

					delegate: Column {
						width: parent.width

						bottomPadding: 3 * Qaterial.Style.pixelSizeRatio

						Qaterial.LabelHeadline6 {
							width: parent.width
							wrapMode: Text.Wrap
							text: modelData.username
							color: Qaterial.Colors.green400
						}

						Qaterial.LabelBody1 {
							width: parent.width
							leftPadding: 30 * Qaterial.Style.pixelSizeRatio
							wrapMode: Text.Wrap
							text: {
								let k = Object.keys(modelData)
								let t = ""
								for (let i=0; i<k.length; ++i) {
									let key = k[i]
									if (key === "username")
										continue

									if (t != "") t += "<br>"
									t += key+": <b>"+modelData[key]+"</b>"
								}

								return t
							}

						}

					}
				}


				Qaterial.LabelHeadline6 {
					visible: !parent._hasRecords
					anchors.horizontalCenter: parent.horizontalCenter
					text: qsTr("Nincs egyetlen importálható rekord!")
					color: Qaterial.Style.accentColor
					topPadding: 20
					bottomPadding: 20
				}

				QLabelInformative {
					visible: parent._hasRecords
					text: qsTr("Ha a fenti adatok megfelelőek, akkor importáld a felhasználókat")
					topPadding: 20
					bottomPadding: 20
				}

				QButton {
					id: _btnImport
					visible: parent._hasRecords
					anchors.horizontalCenter: parent.horizontalCenter
					highlighted: true
					text: qsTr("Importálás")
					icon.source: Qaterial.Icons.import_
					onClicked: {
						_stack.replace(_cmpResult)
					}
				}
			}
		}
	}






	Component {
		id: _cmpResult

		QScrollable {
			id: _scroll
			contentCentered: true
			refreshEnabled: false

			property bool _loading: true

			Row {
				visible: _scroll._loading
				spacing: 20 * Qaterial.Style.pixelSizeRatio

				anchors.horizontalCenter: parent.horizontalCenter

				Qaterial.BusyIndicator {
					anchors.verticalCenter: parent.verticalCenter
				}

				Qaterial.LabelBody1 {
					anchors.verticalCenter: parent.verticalCenter
					text: qsTr("Importálás...")
				}
			}

			Column {
				visible: !_scroll._loading

				width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
				anchors.horizontalCenter: parent.horizontalCenter

				Qaterial.LabelHeadline4 {
					visible: _labelResult.visible
					anchors.horizontalCenter: parent.horizontalCenter
					text: qsTr("Az importálás eredménye")
					bottomPadding: 3 * Qaterial.Style.pixelSizeRatio
				}

				Qaterial.LabelBody1 {
					id: _labelResult

					visible: text != ""

					width: parent.width
					wrapMode: Text.Wrap
				}

				Qaterial.LabelHeadline6 {
					id: _labelError

					visible: text != ""

					anchors.horizontalCenter: parent.horizontalCenter
					horizontalAlignment: Label.AlignHCenter
					color: Qaterial.Colors.red400
					topPadding: 20
					bottomPadding: 20
				}
			}


			Component.onCompleted: {
				Client.send(HttpConnection.ApiAdmin, "user/import", {
								list: _importer.records,
								classid: control.classId
							})
				.done(control, (r) => {
						  _loading = false

						  let t = ""

						  for (let i=0; i<r.list.length; ++i) {
							  let u = r.list[i]

							  if (t != "") {
								  t += "<br>"
							  }

							  t += "<b>"+u.username+"</b> - "

							  if (u.status === "ok") {
								  t += "OK"
							  } else {
								  t += qsTr("Hiba: ")+u.error
							  }
						  }

						  _labelResult.text = t

					  })
				.fail(control, (msg) => {
						  _loading = false
						  _labelError.text = qsTr("Az importálás sikertelen!\n")+msg
					  })

			}
		}
	}


	Component {
		id: _cmpFileOpen

		QFileDialog {
			title: qsTr("Adatok feltöltése")
			filters: [ "*.xlsx" ]
			onFileSelected: file => {
				_importer.upload(file)
				Client.Utils.settingsSet("folder/excelImport", modelFolder.toString())
			}

			folder: Client.Utils.settingsGet("folder/excelImport", "")
		}

	}



	Component {
		id: _cmpFileSave

		QFileDialog {
			title: qsTr("Sablon letöltése")
			filters: [ "*.xlsx" ]

			folder: Client.Utils.settingsGet("folder/excelImport", "")

			isSave: true
			suffix: ".xlsx"
			onFileSelected: file => {
				if (Client.Utils.fileExists(file))
					overrideQuestion(file)
				else
					_importer.downloadTemplate(file)

				Client.Utils.settingsSet("folder/excelImport", modelFolder.toString())
			}

		}
	}

	function overrideQuestion(file) {
		JS.questionDialog({
							  onAccepted: function()
							  {
								  _importer.downloadTemplate(file)
							  },
							  text: qsTr("A fájl létezik. Felülírjuk?\n%1").arg(file),
							  title: qsTr("Sablon letöltése"),
							  iconSource: Qaterial.Icons.fileAlert
						  })
	}

	ListModel {
		id: _preparedClassList

		signal reloaded()

		function reload() {
			clear()

			append({classid: -1, name: qsTr("-- Osztály nélkül --")})

			for (var i=0; i<_classList.length; i++) {
				var o = _classList.get(i)
				append({classid: o.classid, name: o.name})
			}

			reloaded()
		}
	}

	SortFilterProxyModel {
		id: sortedClassList
		sourceModel: _preparedClassList

		filters: [
			RangeFilter {
				roleName: "classid"
				minimumValue: -1
			}
		]

		sorters: [
			FilterSorter {
				ValueFilter {
					roleName: "classid"
					value: -1
				}
				priority: 1
			},
			StringSorter {
				roleName: "name"
				sortOrder: Qt.AscendingOrder
			}
		]
	}


	StackView.onActivated: {
		Client.reloadCache("classList", control, _preparedClassList.reload)
	}

	Component.onCompleted: {
		_components.push(_cmpStart)
		_components.push(_cmpSelect)
		_stack.replace(_cmpStart)
	}

}
