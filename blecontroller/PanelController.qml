import QtQuick 2.0
import "grid_generator.js" as GridGen
import "queue.js" as Queue
Rectangle {
    anchors.fill:parent

    Joystick{
        id:control_joystick
        scale:0.8
        x:(parent.width-width()*scale)/2;
        y:parent.height*2/3;
        z:5
    }

    Image{
        id:image_step
        scale:0.8
        x:10
        y:parent.height/2
        source:"images/step_counter.png"
    }

    Image{
        id:image_autopilot_target
        x:100;
        y:100;
        source:"images/flag.png"
        visible:spider.is_autopilot_running;
    }

    Text{
        id:autopilot_countback_txt
        visible:spider.is_autopilot_running;
        anchors.left: image_step.left;
        y:image_step.height+image_step.y+4;
        font.pixelSize: control_txt.font.pixelSize
    }

    Spider{
        id:spider
    }


    Text{
        id:control_txt;
        anchors.verticalCenter: image_step.verticalCenter
        text:"step counter";
        x:image_step.x+image_step.width*image_step.scale+40;
        font.pixelSize:image_step.height*0.6
    }

    Item{
        id:control_background
        opacity: 0.3
        Component.onCompleted: {
            GridGen.generateGridStack(control_background);
            var w_num = 12;
            var h_num = 25;
            for(var i=0;i<h_num;++i){
                for(var j=0;j<w_num;++j){
                    //console.log("i="+i+",j="+j);
                    var pos = i*w_num+j;
                    var g = GridGen.grid_stack[pos];
                    g.x = -50+j*100;
                    g.y = i*100;
                    g.visible=true;
                    g.setMarked(false);
                }
            }

            GridGen.generateFootprintStack(control_background);
        }
    }

    Connections{
        target:control_joystick
        onJoystickPositionUpdated:{
            cstController.setJoystickPosition(joy_x,joy_y);
        }
    }
    Connections{
        target:cstController
        onStepCounterUpdated:{
            control_txt.text = ""+step+",delta:"+delta;
            var foot = GridGen.get_footprint();
            foot.visible=true;
            foot.rotation = spider.getSpiderRotation();
            foot.x = spider.x;
            foot.y = spider.y;
            var arc = spider.getSpiderRotation()*Math.PI/180.0;
            var dst_x = spider.x+Math.sin(arc)*spider.step_distance*delta;
            var dst_y = spider.y-Math.cos(arc)*spider.step_distance*delta;
            spider.setPosition(dst_x,dst_y);
        }
    }
    Connections{
        target:cstController
        onHeadingUpdated:{
            //console.log(heading);
            spider.setSpiderRotation(heading);
        }
    }
    function reset(){
        spider.x = (parent.width)/2;
        spider.y = parent.height*3/7;

        //reset autopilot;
        spider.is_autopilot_running=false;
        timer_autopilot.stop();
        cstController.setAutopilotTarget(0,0);
    }
    ParallelAnimation{
        id:animate_autopilot
        NumberAnimation{
            id:animate_autopilot_x
            target:image_autopilot_target
            property: "x"
            duration: 80
        }
        NumberAnimation{
            id:animate_autopilot_y
            target:image_autopilot_target
            property: "y"
            duration: 80
        }
        SpringAnimation{
            id:animate_autopilot_scale
            target:image_autopilot_target
            property: "scale"
            spring: 3.0
            damping: 0.2
            epsilon: 0.05
            from:0.2
            to:1.0
            duration:60
        }
    }

    Timer{
        //for auto pilot control
        id:timer_autopilot
        interval:100
        repeat:true
        property int autopilot_counter: 0;//TODO: I have to do it for a quick restult. Eliminate it later, please!
        onTriggered: {
            //check is autopilot stop
            //TODO:this 200 & 500 are 2 const values.
            if(spider.autopilot_radius<cstController.getAutopilotMinRadius() || autopilot_counter>600){
                spider.is_autopilot_running=false;
                cstController.setJoystickPosition(0,0);
            }

            //update the radius
            cstController.setAutopilotTarget(spider.autopilot_degree,spider.autopilot_radius);
            //upload autopilot message to controller
            if(spider.is_autopilot_running){
                autopilot_countback_txt.text = "Time left:"+(600-autopilot_counter)/10+"s, distance:"+spider.autopilot_radius;
            }else{
                timer_autopilot.stop();
            }
            autopilot_counter+=1;
        }
    }

    MouseArea{
        anchors.fill: parent
        z:1
        onClicked: {
            //set the autopilot target image position
            //x
            animate_autopilot_x.from = image_autopilot_target.x;
            animate_autopilot_x.to   = mouse.x;
            //y
            animate_autopilot_y.from = image_autopilot_target.y;
            animate_autopilot_y.to   = mouse.y;
            //start animation
            animate_autopilot.running = true;
            //update the
            spider.is_autopilot_running=true;
            spider.setAutopilotDestination(mouse.x,mouse.y);
            timer_autopilot.start();
            timer_autopilot.autopilot_counter = 0;
        }
    }
    Image{
        id:image_reset
        source:"images/reset_btn.png"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom:parent.bottom;
        z:2

        MouseArea{
            anchors.fill: image_reset
            onClicked: {
                console.log("hello?")
                reset();
            }
        }
    }
}
