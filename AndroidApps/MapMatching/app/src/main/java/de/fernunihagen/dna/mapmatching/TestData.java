package de.fernunihagen.dna.mapmatching;

import de.fernunihagen.dna.mapmatchingcore.Point;

public class TestData {

    // To run testdata
    // comment in:
    // MapMatchingActivity: TestData.test(); in public void mapMatchingStart() {}
    // comment out:
    // MapMatchingCoreInterface: mapMatchingActivity.showLineInfoMessage("Location Point is being processed...");
    // MapMatchingGps: content of public void onLocationChanged(Location location){..}
    //-----------------------------------------------------------------------------------
    // TestData: public static void test(){..} -> Thread.sleep(6000); Time has to be adjusted or app may crash

    //Test Route
    static Point point1 = new Point( 51.50799055752574,7.466119229793549 ,285 , 11, 6, System.currentTimeMillis());
    static Point point2 = new Point(51.50807770121974, 7.465852349996567, 285, 25, 6, System.currentTimeMillis()+20000);
    static Point point3 = new Point(51.50811075572293, 7.465586364378396, 285, 35, 6, System.currentTimeMillis()+40000);
    static Point point4 = new Point(51.508190887678225, 7.46524304162449,285 , 35, 6, System.currentTimeMillis()+60000);
    static Point point5 = new Point(51.508257664199995, 7.464931905378762, 285, 45, 6, System.currentTimeMillis()+80000);
    static Point point6 = new Point(51.50846133198691, 7.464835345854226, 355, 80, 6, System.currentTimeMillis()+100000);

    static Point point7 = new Point(51.50871841953274, 7.464776337255898, 355, 0, 6, System.currentTimeMillis()+120000);
    static Point point8 = new Point(51.50902892593318, 7.464765608419839, 355, -20, 6, System.currentTimeMillis()+140000);
    static Point point9 = new Point(51.50924260652791, 7.46473878632969, 355, 15, 6, System.currentTimeMillis()+160000);
    static Point point10 = new Point(51.509472979796435, 7.46474415074772, 355, 25, 6, System.currentTimeMillis()+180000);
    static Point point11 = new Point(51.50964993233461, 7.464717328657571, 355, -68, 6, System.currentTimeMillis()+200000);

    static Point point12 = new Point(51.5097233841509, 7.46498554955906, 90, 10, 6, System.currentTimeMillis()+220000);
    static Point point13 = new Point(51.50974341644387, 7.4652484060425195, 90, 10, 6, System.currentTimeMillis()+240000);
    static Point point14 = new Point(51.50975677130096, 7.465511262525979, 90, 10, 6, System.currentTimeMillis()+260000);
    static Point point15 = new Point(51.50972004543454, 7.4658975006241235, 90, 10, 6, System.currentTimeMillis()+280000);
    static Point point16 = new Point(51.5097233841509, 7.466160357107583, 90, 10, 6, System.currentTimeMillis()+300000);
    static Point point17 = new Point(51.50972004543454, 7.46649295102543, 90, 10, 6, System.currentTimeMillis()+320000);
    static Point point18 = new Point(51.5097233841509, 7.466916740049783, 90, 10, 6, System.currentTimeMillis()+340000);

    static Point point19 = new Point(51.509689996976356, 7.467361986746255, 135, 10, 6, System.currentTimeMillis()+360000);
    static Point point20 = new Point(51.50944293117531, 7.467640936483804, 135, 10, 6, System.currentTimeMillis()+380000);

    static Point point21 = new Point(51.50908167833148, 7.467940449714661, 180, 10, 6, System.currentTimeMillis()+400000);
    static Point point22 = new Point(51.50825365762166, 7.46795117855072, 180, 10, 6, System.currentTimeMillis()+420000);

    static Point point23 = new Point(51.50744565506478, 7.4675434827804565, 270, 10, 6, System.currentTimeMillis()+440000);
    static Point point24 = new Point(51.507372310845476, 7.466336935649451, 270, 10, 6, System.currentTimeMillis()+460000);

    static Point point25 = new Point(51.50659100423277, 7.466138452182349, 180, 10,6, System.currentTimeMillis()+480000);


    //No One Way Roads Test
//    static Point point1 = new Point( 51.507025621618695,7.451372444629669 ,100 , 11, 6, System.currentTimeMillis());
//    static Point point2 = new Point(51.507025621618695, 7.451769411563873, 100, 25, 6, System.currentTimeMillis()+20000);
//    static Point point3 = new Point(51.50699223246719, 7.4521127343177795, 100, 35, 6, System.currentTimeMillis()+40000);
//    static Point point4 = new Point(51.506982215716974, 7.452568709850311,100 , 35, 6, System.currentTimeMillis()+60000);
//    static Point point5 = new Point(51.50696552112837, 7.452954947948456, 100, 45, 6, System.currentTimeMillis()+80000);
//    static Point point6 = new Point(51.50693213193282, 7.453646957874298, 100, 80, 6, System.currentTimeMillis()+100000);
//
//    static Point point7 = new Point(51.50687537024426, 7.454558908939362, 100, 0, 6, System.currentTimeMillis()+120000);
//    static Point point8 = new Point(51.50679857490598, 7.455615699291229, 100, -20, 6, System.currentTimeMillis()+140000);
//    static Point point9 = new Point(51.50669840687895, 7.456500828266144, 100, 15, 6, System.currentTimeMillis()+160000);
//    static Point point10 = new Point(51.50665833960647, 7.457428872585297, 100, 25, 6, System.currentTimeMillis()+180000);



    //bbox = 750 meter -> skippedStreet for checking trajectories
    static Point skippedStreet1 = new Point(51.511713994174684, 7.485198855138151, 270, 25, 5, System.currentTimeMillis());
    static Point skippedStreet2 = new Point(51.509490452059126,7.48472678635153 ,110 , 11, 5, System.currentTimeMillis()+30000);
    static Point skippedStreet3 = new Point(51.51174660126075,7.485656440258026 ,270 , 11, 5, System.currentTimeMillis()+60000);

    //motorway junction
    static Point motorwayJunction1 = new Point(51.443426398601474, 7.48494029045105, 200, 25, 5, System.currentTimeMillis());
    static Point motorwayJunction2 = new Point(51.44257043603694,7.48418927192688 ,200 , 11, 5, System.currentTimeMillis()+30000);
    static Point motorwayJunction3 = new Point(51.44231631906215,7.481743097305298 ,270 , 11, 5, System.currentTimeMillis()+60000);
    static Point motorwayJunction4 = new Point(51.4431321632795, 7.48167872428894, 90, 25, 5, System.currentTimeMillis()+90000);
    static Point motorwayJunction5 = new Point(51.44310541370358, 7.482858896255493, 90, 25, 5, System.currentTimeMillis()+120000);
    static Point motorwayJunction6 = new Point(51.44321240864811, 7.485283613204956, 90, 25, 5, System.currentTimeMillis()+150000);

    //fernuni hagen
    static Point fernuni  = new Point(51.37759, 7.494619999999941, 200, 25, 5, System.currentTimeMillis());



    static Point[] test = {point1,point2,point3,point4,point5,point6,point7,point8,point9,point10,point11,point12,point13,point14,point15,point16,point17,point18,point19,point20,point21,point22,point23,point24,point25};
//    static Point[] test = {point1,point2,point3,point4,point5,point6,point7,point8,point9,point10};
//    static Point[] test = {point1,point2,point3,point4,point5,point6,point5,point4,point3,point2,point1};
//    static Point[] test = {point1,point3,point5,point7,point9,point11,point13,point15,point17,point19,point21,point23,point25};
//    static Point[] test = {point1,point2,point3,point4,point5};
//    static Point[] test2 = {point9,point10,point11,point12};
//    static Point[] test3 = {point16,point17,point18,point19,point20,point21,point22,point23,point24,point25};
//    static Point[] test = {point1,point2,point3,point4,point5,point6,point7};
//    static Point[] test = {point7,point8,point9,point10,point11,point12,point11,point10,point9,point8,point7}
//    static Point[] test = {point23,point24,point25};
//    static Point[] test = {point1};

//    static Point[] test = {point1,point2,point3,point4,point5,point6,point7,point8,point9,point10,point11,point12,point13,point14,point15,point16,point17,point18,point19,point20};
//    static Point[] test = {point18,point19,point20,point21,point22,point23,point24,point25};
//    static Point[] test = {point1,point4,point5,point7,point9};

//    static Point[] test = {point7,point8,point9,point10,point11,point12,point11,point10,point9,point8,point7};

//    static Point[] test = {motorwayJunction1, motorwayJunction2, motorwayJunction3, motorwayJunction4, motorwayJunction5, motorwayJunction6};
    //static Point[] test = {fernuni};
//    static Point[] test = {skippedStreet1, skippedStreet2};
//    static Point[] test = {skippedStreet1, skippedStreet2, skippedStreet3};


    static int testCounter = 0;

    public static void test(){
        new Thread(new Runnable() {
            public void run() {
                while(test.length > testCounter){
                    try {
                        if(testCounter == 0){
                            Thread.sleep(1);
                        }
                        Thread.sleep(6000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    MapMatchingCoreInterface.processLocationPoint(test[testCounter]);
                    testCounter = testCounter +1;
                    System.out.println(testCounter);
                }
                testCounter = 0;
            }
        }).start();
    }
}
