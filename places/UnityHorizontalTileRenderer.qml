/*
 * This file is part of unity-2d
 *
 * Copyright 2010-2011 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import Qt 4.7
import Unity2d 1.0 /* required for drag’n’drop handling */

RendererGrid {
    cellWidth: 250
    cellHeight: 70
    horizontalSpacing: 10
    verticalSpacing: 10

    cellRenderer: Component {
        Button {
            property string uri
            property string iconHint
            property string mimetype
            property string displayName
            property string comment

            onClicked: {
                dashView.active = false
                placeEntryModel.place.activate(decodeURIComponent(uri))
            }

            DragItemWithUrl {
                anchors.fill: parent
                url: decodeURIComponent(uri)
                defaultAction: {
                    if (!url.indexOf("application://")) return Qt.CopyAction
                    else if (!url.indexOf("unity-install://")) return Qt.IgnoreAction
                    else return Qt.LinkAction
                }
                supportedActions: defaultAction
                delegate: Component {
                    Image {
                        source: icon.source
                        width: icon.width
                        height: icon.height
                        fillMode: icon.fillMode
                        sourceSize.width: width
                        sourceSize.height: height
                        asynchronous: true
                    }
                }
                onPressed: parent.pressed = true
                onReleased: {
                    parent.pressed = false
                    parent.clicked()
                }
                onDrop: parent.pressed = false
            }

            Image {
                id: icon

                source: iconHint != "" ? "image://icons/"+iconHint : ""
                width: 48
                height: 48
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.topMargin: 10
                anchors.leftMargin: 10
                fillMode: Image.PreserveAspectFit
                sourceSize.width: width
                sourceSize.height: height

                asynchronous: true
                opacity: status == Image.Ready ? 1 : 0
                Behavior on opacity {NumberAnimation {duration: 200; easing.type: Easing.InOutQuad}}
            }

            TextMultiLine {
                id: label

                text: displayName
                color: parent.state == "pressed" ? "#5e5e5e" : "#ffffff"
                state: parent.state == "selected" ? "expanded" : ""
                horizontalAlignment: Text.AlignLeft
                anchors.left: icon.right
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.top: parent.top
                anchors.topMargin: 10
                anchors.bottomMargin: 5
                anchors.rightMargin: 3
                anchors.leftMargin: 3
                font.underline: parent.activeFocus
            }
        }
    }
}
