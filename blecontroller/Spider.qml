import QtQuick 2.0

Item {
    id:spider
    property int step_distance:32;
    property int autopilot_threshold: 35

    property bool is_autopilot_running: false;
    property int  autopilot_x;
    property int  autopilot_y;
    property int  autopilot_radius;
    property int  autopilot_degree;
    Image{
        anchors.centerIn: parent
        id:image_spider
        source:"images/heading.png"
    }
    Image{
        anchors.centerIn: parent
        id:image_spider_to
        source:"images/heading_arrow.png"
        visible: is_autopilot_running
    }

    NumberAnimation{
        id:animate_heading
        target:image_spider
        property:"rotation"
        duration: 100 //ms

    }
    function setSpiderRotation(degree){
        animate_heading.from = image_spider.rotation;
        animate_heading.to = degree;
        animate_heading.running=true;
    }
    function getSpiderRotation(){
        return image_spider.rotation;
    }



    function setPosition(x,y){
        //set position
        spider.x = x;
        spider.y = y;

        if(is_autopilot_running)
        {//update the UI
            updateAutopilotHeading();
            if(autopilot_radius<autopilot_threshold)
            {//check if the target is reached.
                is_autopilot_running=false;
            }
        }
    }

    NumberAnimation{
        id:animate_heading_to
        target:image_spider_to
        property: "rotation"
        duration:50
    }
    function setAutopilotDestination(x,y){
        is_autopilot_running = true;
        autopilot_x = x;
        autopilot_y = y;
        updateAutopilotHeading();
        //console.log("degree="+autopilot_degree);
    }
    function updateAutopilotHeading()
    {
        var dx = autopilot_x-spider.x;
        var dy = autopilot_y-spider.y;
        var r = Math.sqrt((dx*dx+dy*dy));
        var alpha = calc_alpha(dx,dy,r);

        autopilot_radius = r;
        autopilot_degree = alpha*180/Math.PI;
        //set animation
        animate_heading_to.from = image_spider_to.rotation;
        animate_heading_to.to = autopilot_degree;
        animate_heading_to.running=true;
    }

    function calc_alpha(x,y,r){
        if(r==0)return 0;
        var alpha=Math.asin(x/r);
        if(y>0){
            alpha = Math.PI-alpha;
        }

        return alpha;
    }
}
