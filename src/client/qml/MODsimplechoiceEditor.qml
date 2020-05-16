import QtQuick 2.12
import QtQuick.Controls 2.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


MODabstractEditor {
	id: control

	property string storageModule: ""

	QCollapsible {
		title: qsTr("Kérdések készítése kérdés-válasz alapján")

		visible: storageModule == "questionpair"

		ButtonGroup { id: grp }
		ButtonGroup { id: grp2 }

		QGridLayout {
			width: parent.width

			QGridLabel {
				field: textPrefix
			}

			QGridTextField {
				id: textPrefix
				fieldName: qsTr("Kérdés szövegének kezdete:")
				placeholderText: qsTr("pl. Melyik a, Ki volt,...stb.")
				sqlField: "prefix"
				onTextModified: saveJson()
			}

			QGridLabel {
				field: textSuffix
			}

			QGridTextField {
				id: textSuffix
				fieldName: qsTr("Kérdés szövegének vége:")
				placeholderText: qsTr("pl: ?,...stb.")
				sqlField: "suffix"
				onTextModified: saveJson()
			}

			QGridCheckBox {
				id: checkAll
				text: qsTr("Mindet megkérdezi")
				ButtonGroup.group: grp
				onToggled: saveJson()
			}

			QGridCheckBox {
				id: checkFavorites
				text: qsTr("Csak a kiemelteket kérdezi meg")
				ButtonGroup.group: grp
				onToggled: saveJson()
			}

			QGridCheckBox {
				id: checkNoFavorites
				text: qsTr("Csak a nem kiemelteket kérdezi meg")
				ButtonGroup.group: grp
				onToggled: saveJson()
			}

			QGridCheckBox {
				id: checkRest
				text: qsTr("A még fel nem használtakat kérdezi meg")
				ButtonGroup.group: grp
				onToggled: saveJson()
			}

			QGridCheckBox {
				id: checkNumber
				text: qsTr("Pontos darabszámot kérdez meg")
				ButtonGroup.group: grp
				onToggled: saveJson()
			}

			QGridText {
				visible: checkNumber.checked
				text: qsTr("Kérdések száma:")
				field: spinNumber
			}

			QGridSpinBox {
				id: spinNumber
				visible: checkNumber.checked
				ToolTip.text: qsTr("Ennyi kérdést készít")
				from: 1
				to: 999

				sqlField: "number"

				onValueModified: saveJson()
			}

			QGridCheckBox {
				id: checkFavoritesFirst
				visible: checkNumber.checked
				sqlField: "favoritesFirst"
				ButtonGroup.group: grp2
				text: qsTr("Először a kiemelteket használja fel")
				onToggled: saveJson()
			}

			QGridCheckBox {
				id: checkNoFavoritesFirst
				visible: checkNumber.checked
				sqlField: "noFavoritesFirst"
				ButtonGroup.group: grp2
				text: qsTr("Először a nem kiemelteket használja fel")
				onToggled: saveJson()
			}
		}
	}


	QCollapsible {
		title: qsTr("Kérdések készítése sorrend alapján")

		visible: storageModule == "order"

		QGridLayout {
			width: parent.width

			QGridText {
				text: qsTr("Kérdések száma:")
				field: spinNumber2
			}

			QGridSpinBox {
				id: spinNumber2
				ToolTip.text: qsTr("Ennyi kérdést készít")
				from: 1
				to: 999

				sqlField: "number"

				onValueModified: saveJson()
			}

			QGridCheckBox {
				id: checkFavoritesFirst2
				sqlField: "favoritesFirst"
				text: qsTr("Először a kiemelteket használja fel")
				onToggled: saveJson()
			}

			QGridLabel {
				field: textMinus
			}

			QGridTextField {
				id: textMinus
				fieldName: qsTr("Kérdés a kisebbre:")
				placeholderText: qsTr("pl. Melyik a kisebb?, Melyik történt korábban?,...")
				sqlField: "minus"
				onTextModified: saveJson()
			}

			QGridLabel {
				field: textPlus
			}

			QGridTextField {
				id: textPlus
				fieldName: qsTr("Kérdés a nagyobbra:")
				placeholderText: qsTr("pl. Melyik a nagyobb?, Melyik történt később?,...")
				sqlField: "plus"
				onTextModified: saveJson()
			}

			QGridLabel {
				field: textMinimum
			}

			QGridTextField {
				id: textMinimum
				fieldName: qsTr("Kérdés a legkisebbre:")
				placeholderText: qsTr("pl. Melyik a legkisebb?, Melyik történt legkorábban?,...")
				sqlField: "minimum"
				onTextModified: saveJson()
			}

			QGridLabel {
				field: textMaximum
			}

			QGridTextField {
				id: textMaximum
				fieldName: qsTr("Kérdés a legnagyobbra:")
				placeholderText: qsTr("pl. Melyik a legnagyobb?, Melyik történt legkésőbb?,...")
				sqlField: "maximum"
				onTextModified: saveJson()
			}
		}
	}



	onEditorDataChanged: {
		storageModule = editorData.storageModule
		if (storageModule == "questionpair") {
			var mode = editorData.data.mode

			if (mode === "favorites")
				checkFavorites.checked = true
			else if (mode === "nofavorites")
				checkNoFavorites.checked = true
			else if (mode === "rest")
				checkRest.checked = true
			else if (mode === "number") {
				checkNumber.checked = true
			} else
				checkAll.checked = true

			JS.setSqlFields([textPrefix, textSuffix, spinNumber, checkFavoritesFirst, checkNoFavoritesFirst], editorData.data)
		} else if (storageModule == "order") {
			JS.setSqlFields([spinNumber2, checkFavoritesFirst2, textMinus, textPlus, textMinimum, textMaximum], editorData.data)
		}


	}


	function saveJsonData() {
		if (storageModule == "questionpair") {
			var d = JS.getSqlFields([textPrefix, textSuffix, spinNumber, checkFavoritesFirst, checkNoFavoritesFirst])

			if (checkNumber.checked) {
				d.mode = "number"
			} else if (checkFavorites.checked) {
				d.mode = "favorites"
			} else if (checkNoFavorites.checked) {
				d.mode = "nofavorites"
			} else if (checkRest.checked) {
				d.mode = "rest"
			} else {
				d.mode = "all"
			}

			return d
		} else if (storageModule == "order") {
			return JS.getSqlFields([spinNumber2, checkFavoritesFirst2, textMinus, textPlus, textMinimum, textMaximum])
		}

		return {}
	}
}

