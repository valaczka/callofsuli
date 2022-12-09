import QtQuick 2.15
import QtQuick.Controls 2.15
import "Style"
import "JScript.js" as JS

QTabButton {
	id: control

	property QSwipeContainer swipeContainer: null

	text: swipeContainer ? swipeContainer.title : ""
	icon.source: swipeContainer ? swipeContainer.icon : ""
}
