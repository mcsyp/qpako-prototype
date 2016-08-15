import QtQuick 2.6
import QtQuick.Window 2.2


Window {
    id:main_window

    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")

    PanelScan{
        id:panel_scan
        width:parent.width  //set as full screen size
        height: parent.height
        z:1;
    }
    Text {
        id:main_bottom_txt;
        text: qsTr("hello message?")
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        z:5;
        visible: panel_scan.visible
    }
    PanelController{
        id:panel_controller
        z:2;
        visible: false
    }
    Connections{
        target:cstController
        onStateDeviceConnected:{
            main_window.showMessage(address+" connceted");
        }

        onStateDeviceDisconnected:{
            //address
            main_window.showMessage(address+" disconnceted");

            //show panel scan
            panel_controller.visible=false;
            panel_scan.visible=true;
        }
        onStateServiceDiscoverStarted:{
            main_window.showMessage("Searching required serivce.");
        }

        onStateServiceDiscoverDone:{
            //show control panel
            if(cstController.isServiceDiscovered()){
                //show controller
                panel_controller.visible = true;
                panel_controller.reset();
                panel_scan.visible = false;
            }else{
                main_window.showMessage("Required service not found on target.");
            }
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
    function showMessage(message){
        main_bottom_txt.text = message;
    }


}
