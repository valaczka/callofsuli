import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

Qaterial.FullLoaderItemDelegate {
	id: control

	width: ListView.view.width

	leftPadding: Math.max(!mirrored ? Qaterial.Style.delegate.leftPadding(control.type, control.lines) : Qaterial.Style.delegate
	  .rightPadding(control.type, control.lines), Client.safeMarginLeft)
	rightPadding: Math.max(mirrored ? Qaterial.Style.delegate.leftPadding(control.type, control.lines) : Qaterial.Style.delegate
	  .rightPadding(control.type, control.lines), Client.safeMarginRight)
}

