import QtQuick 2.0

Item {

    property int range:230;
    property int center_x;
    property int center_y;

    signal joystickPositionUpdated(var joy_x,var joy_y);
    Image{
        id: joystick_frame
        source: "images/joystick_frame.png"
        z:2;
    }

    Image {
        id: joystick
        source: "images/joystick.png"
        z:5;
    }
    Image{
        id: joystick_range
        source: "images/joystick_range.png"
        z:1;
        opacity: 0.1;
    }

    MouseArea{
        id: mouse
        anchors.fill: joystick_frame
        width:joystick_frame.width
        height:joystick_frame.height
        onPressed: {
            joystick_range.visible=true;
        }

        onPositionChanged: {
            joystick.x = mouse.x-joystick.width/2;
            joystick.y = mouse.y-joystick.height/2;
            var x = joystick.x-center_x;
            var y = joystick.y-center_y;
            var r = Math.sqrt(Math.pow(x,2)+Math.pow(y,2));
            var alpha=0;
            joystick_range.opacity = (r/range);
            alpha = calc_alpha(x,y,r);
            if(r>range){
                joystick.x= center_x+range*Math.cos(alpha);
                joystick.y = center_y+range*Math.sin(alpha);
            }
            joystickPositionUpdated(Math.cos(alpha),-Math.sin(alpha));
        }
        onReleased: {
            reset();
        }
    }
    function reset(){
        joystick.x = center_x;
        joystick.y = center_y;

        joystick_range.visible=false;
        joystickPositionUpdated(0,0);
    }
    function calc_alpha(x,y,r){
        if(r==0)return 0;
        var alpha=Math.asin(y/r);
        if(x<0){
            alpha = Math.PI-alpha;
        }

        return alpha;
    }
    function width(){
        return joystick_frame.width;
    }

    function height(){
        return joystick_frame.height;
    }

    Component.onCompleted: {
        center_x = joystick_frame.x+(joystick_frame.width-joystick.width)/2;
        center_y = joystick_frame.y+(joystick_frame.height-joystick.height)/2;
        joystick_range.x =  joystick_frame.x+(joystick_frame.width-joystick_range.width)/2;
        joystick_range.y =  joystick_frame.x+(joystick_frame.width-joystick_range.width)/2;
        reset();
    }

}
