import QtQuick 2.0

Item {
    id:scanner_panel
    property real new_scale:parent.width/1080;

    Image { //bar scanner_title image
        id: scanner_title
        source: "images/bar_landscape.png"
        width:parent.width
        height:120*new_scale;
        z:5;

        property bool scan_state: false;
        Text {
            id:scanner_btn;
            text: qsTr("Press to SCAN")
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 22
            color: "#FFFFFF"
            //font.bold: true
            font.family: "Helvetica"
            font.pointSize: 15
        }

        MouseArea {
            id:mouse
            anchors.fill: parent;
            onClicked: {
                if(scanner_title.scan_state){
                    //means it is running, stop it.
                    cstController.stop();
                }else{
                    cstController.start();
                }
            }

        }

        function setScanningState( b)
        {
            scanner_title.scan_state = b;
            if(scanner_title.scan_state){
                scanner_btn.text="Press to STOP SCANNING";
            }else{
                scanner_btn.text="Press to SCAN";
            }
        }
    }
    Image{
        id:scanner_progress_icon
        source: "images/scanning.png"
        scale: 0.8
        x:parent.width/2-width/2
        y:0
        z: 2;
        visible:false;

        Timer {

           id:scanning_timer;
           running:false;
           interval:20;
           repeat:true;

           onTriggered: {
               scanner_progress_icon.rotation = scanner_progress_icon.rotation + 5;
           }
        }

        function setScanningState(b){
            scanning_timer.running = b;
            scanner_progress_icon.y = parent.height/2;
            scanner_progress_icon.visible=b;
        }
    }

    Item {
        id:scanner_device_view
        x:0;
        y:scanner_title.height
        visible: true;
        z:-1;
        width:parent.width
        height:parent.height-scanner_title.height

        ListView {
            id: scanner_device_listview;
            anchors.fill:parent;
            clip: true
            highlightFollowsCurrentItem: true
            model:cstController.deviceList

            delegate: Component {
                    Item{
                        id: box
                       // height:80
                        width: parent.width
                        height:device_img.height
                        property real new_scale:parent.width/1080;
                        Image{
                            id:device_img
                            source:"images/scan_device_landscape.png"
                            width:parent.width
                            height:130*new_scale;
                        }
                        Text{
                            id:txt_device_name
                            text:modelData.deviceName
                            //anchors.verticalCenter: parent.verticalCenter
                            x:parent.width*0.1;
                            y:parent.height*0.2;
                            font.bold: true
                            font.pixelSize: parent.height*0.2
                            color:"#FFFFFF"
                        }

                        Text{
                            text:modelData.deviceAddress
                            //anchors.verticalCenter: parent.verticalCenter
                            x:txt_device_name.x;
                            y:parent.height*0.5;
                            font.bold: false
                            font.pixelSize: parent.height*0.18
                            color:"#808080"
                        }

                        Text{
                            text:"RSSI:"+modelData.deviceRSSI
                            //anchors.verticalCenter: parent.verticalCenter
                            x:parent.width*0.8;
                            y:parent.height*0.5;
                            font.bold: false
                            font.pixelSize: parent.height*0.18
                            color:"#909090"
                        }

                        function fuck(){
                            console.log("fuck!");
                            console.log(index);
                        }

                        MouseArea{
                            anchors.fill:parent;
                            onClicked: {
                                  cstController.connectToDevice(modelData.deviceAddress);
                                  main_window.showMessage("connecting to "+modelData.devcieName);
                            }
                        }

                    }
            }

        }
    }

    Connections{
        target:cstController
        onStateDeviceDiscoverStarted:{
            scanner_title.setScanningState(true);
            scanner_progress_icon.setScanningState(true);
            main_window.showMessage("Scanning new devices.");
        }
        onStateDeviceDiscoverDone:{
            scanner_title.setScanningState(false);
            scanner_progress_icon.setScanningState(false);
            main_window.showMessage("Scan done.");
        }

        /** controller unused signals
         *void stateDeviceDiscoverStarted
         *void stateDeviceDiscoverDone
         *void stateDeviceConnected(const QString & address);
         *void stateDeviceDisconnected(const QString & address);
         *void stateServiceDiscoverStarted
         *void stateServiceDiscoverDone
         */
    }

    Component.onCompleted: {
        cstController.start();
    }
}
