import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Column {
	id: control

	required property var grades
	required property var grading
	required property string name

	property bool separator: false

	width: parent.width

	Item {
		width: parent.width
		height: 70

		visible: control.separator

		Rectangle {
			width: parent.width
			height: 2
			color: "black"
			anchors.verticalCenter: parent.verticalCenter
			opacity: 0.5
		}
	}

	Row {
		QLabel {
			anchors.verticalCenter: parent.verticalCenter
			text: control.name
			wrapMode: Text.Wrap
			width: Math.max(control.width-rowGrades.width, control.width*0.25)
			color: "saddlebrown"
			font.weight: Font.Medium
			font.capitalization: Font.AllUppercase
		}

		Row {
			id: rowGrades

			anchors.bottom: parent.bottom
			spacing: 20

			Repeater {
				model: control.grades
				Column {
					id: col
					anchors.verticalCenter: parent.verticalCenter

					required property int gradeid
					required property int xp
					required property bool forecast

					readonly property var gradeData: studentMaps.grade(gradeid)
					readonly property color textColor: forecast ? "darkred" : "black"

					QLabel {
						anchors.horizontalCenter: parent.horizontalCenter
						text: col.gradeid == -1 ? col.xp : col.gradeData.shortname
						color: col.textColor
						font.pixelSize: CosStyle.pixelSize*2
						font.weight: Font.Bold
					}

					QLabel {
						anchors.horizontalCenter: parent.horizontalCenter
						text: col.gradeid == -1 ? qsTr("XP") : col.gradeData.longname
						color: col.textColor
						font.pixelSize: CosStyle.pixelSize*0.8
						font.weight: Font.Medium
					}
				}
			}
		}
	}

	ListModel {
		id: gradeSourceModel
	}



	Repeater {
		model: SortFilterProxyModel {
			sourceModel: gradeSourceModel

			sorters: [
				StringSorter { roleName: "gradingType"; sortOrder: Qt.AscendingOrder; priority: 1 },
				RoleSorter { roleName: "value"; sortOrder: Qt.AscendingOrder; priority: 0 }
			]
		}

		Column {
			id: colGrade

			required property int index

			required property var criteria
			required property int ref
			required property int value
			required property string gradingType

			readonly property var gradeData: studentMaps.grade(ref)

			width: parent.width-10
			anchors.left: parent.left
			anchors.leftMargin: 10

			QLabel {
				anchors.left: parent.left
				text: colGrade.gradingType == "grade" ? qsTr("(%1) %2").arg(colGrade.gradeData.shortname).arg(colGrade.gradeData.longname) :
														qsTr("%1 XP").arg(colGrade.value)
				wrapMode: Text.Wrap
				color: "saddlebrown"
				font.weight: Font.Bold
				font.pixelSize: CosStyle.pixelSize*0.8
				font.capitalization: Font.AllUppercase
				topPadding: colGrade.index > 0 ? 10 : 0
				bottomPadding: 5
			}

			Repeater {
				model: colGrade.criteria

				Row {
					id: rowCriteria
					required property var criterion
					required property bool success

					readonly property string module: criterion.module
					readonly property int value: criterion.value
					readonly property bool required: criterion.mode === "required"

					width: parent.width-10
					anchors.left: parent.left
					anchors.leftMargin: 10

					spacing: 10

					Image {
						id: imgSuccess
						anchors.top: parent.top
						width: CosStyle.pixelSize*1.6
						height: CosStyle.pixelSize*1.6
						fillMode: Image.PreserveAspectFit
						source: rowCriteria.success ? "qrc:/internal/img/checkmark_red.png" : ""
					}

					QLabel {
						width: parent.width-imgSuccess.width-rowCriteria.spacing
						wrapMode: Text.Wrap
						anchors.verticalCenter: rowCriteria.success && imgSuccess.height > height ? parent.verticalCenter : undefined
						anchors.top: rowCriteria.success && imgSuccess.height > height ? undefined : parent.top
						color: rowCriteria.required ? "darkred" : "black"
						font.strikeout: rowCriteria.success
						opacity: rowCriteria.success ? 0.4 : 1.0

						text: if (rowCriteria.module == "xp")
								  qsTr("Gyűjts össze legalább %1 XP-t").arg(rowCriteria.value)
							  else if (rowCriteria.module == "trophy")
								  qsTr("Gyűjts össze legalább %1 trófeát").arg(rowCriteria.value)
							  else
								  "%1: %2".arg(rowCriteria.module).arg(rowCriteria.value)
						font.family: "Special Elite"
					}
				}
			}
		}
	}




	Component.onCompleted: {
		for (var i=0; i<grading.grade.length; i++) {
			var o = grading.grade[i]

			o.gradingType = "grade"

			var onlyDefault = true

			for (var j=0; j<o.criteria.length; j++) {
				var cc = o.criteria[j].criterion
				if (!cc.mode || cc.mode !== "default")
					onlyDefault = false
			}

			if (!onlyDefault)
				gradeSourceModel.append(o)
		}

		for (i=0; i<grading.xp.length; i++) {
			o = grading.xp[i]

			o.gradingType = "xp"
			o.ref = -1				// A required property miatt kell

			onlyDefault = true

			for (j=0; j<o.criteria.length; j++) {
				cc = o.criteria[j].criterion
				if (!cc.mode || cc.mode !== "default")
					onlyDefault = false
			}

			if (!onlyDefault)
				gradeSourceModel.append(o)
		}
	}

}
