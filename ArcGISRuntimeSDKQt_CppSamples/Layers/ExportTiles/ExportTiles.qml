// [WriteFile Name=ExportTiles, Category=Layers]
// [Legal]
// Copyright 2016 Esri.

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// [Legal]

import QtQuick 2.3
import QtQuick.Controls 1.2
import QtGraphicalEffects 1.0
import Esri.Samples 1.0
import Esri.ArcGISExtras 1.1

ExportTilesSample {
    id: exportTilesSample
    width: 800
    height: 600

    property double scaleFactor: System.displayScaleFactor
    property string dataPath: System.userHomePath + "/ArcGIS/Runtime/Data/"
    property string statusText: ""

    // add a mapView component
    MapView {
        id: mapView
        anchors.fill: parent
        objectName: "mapView"
    }

    onHideWindow: {
        exportWindow.hideWindow(time);

        if (success) {
            extentRectangle.visible = false;
            downloadButton.visible = false;
        }
    }

    onUpdateStatus: statusText = status;

    Rectangle {
        id: extentRectangle
        anchors.centerIn: parent
        width: parent.width - (50 * scaleFactor)
        height: parent.height - (125 * scaleFactor)
        color: "transparent"
        border {
            color: "red"
            width: 3 * scaleFactor
        }
    }

    // Create the download button to export tile cache
    Rectangle {
        id: downloadButton
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
            bottomMargin: 10 * scaleFactor
        }

        width: 130 * scaleFactor
        height: 35 * scaleFactor
        color: "#DADADA"
        radius: 8
        border {
            color: "#585858"
            width: 1 * scaleFactor
        }

        Row {
            anchors.fill: parent
            spacing: 5
            Image {
                width: 38 * scaleFactor
                height: width
                source: "qrc:/Samples/Layers/ExportTiles/download.png"
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: "Export tiles"
                font.pixelSize: 14 * scaleFactor
                color: "#474747"
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                // call the C++ invokable function to export tile cache from the input screen coordinates
                exportTilesSample.exportTileCacheFromCorners(extentRectangle.x, extentRectangle.y, (extentRectangle.x + extentRectangle.width), (extentRectangle.y + extentRectangle.height), dataPath);
                exportWindow.visible = true;
            }
        }
    }

    // Create a window to display the export window
    Rectangle {
        id: exportWindow
        anchors.fill: parent
        color: "transparent"
        visible: false
        clip: true

        GaussianBlur {
            anchors.fill: exportWindow
            source: mapView
            radius: 40
            samples: 20
            rotation: 180
        }

        MouseArea {
            anchors.fill: parent
            onClicked: mouse.accepted = true
            onWheel: wheel.accepted = true
        }

        Rectangle {
            anchors.centerIn: parent
            width: 125 * scaleFactor
            height: 100 * scaleFactor
            color: "lightgrey"
            opacity: 0.8
            radius: 5
            border {
                color: "#4D4D4D"
                width: 1 * scaleFactor
            }

            Column {
                anchors {
                    fill: parent
                    margins: 10 * scaleFactor
                }
                spacing: 10

                BusyIndicator {
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: statusText
                    font.pixelSize: 16 * scaleFactor
                }
            }
        }

        Timer {
            id: hideWindowTimer

            onTriggered: exportWindow.visible = false;
        }

        function hideWindow(time) {
            hideWindowTimer.interval = time;
            hideWindowTimer.restart();
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border {
            width: 0.5 * scaleFactor
            color: "black"
        }
    }

    FileFolder {
        path: dataPath

        // create the data path if it does not yet exist
        Component.onCompleted: {
            if (!exists) {
                makePath(dataPath);
            }
        }
    }
}
