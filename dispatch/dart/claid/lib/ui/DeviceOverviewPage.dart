
import 'package:alex_data_collection/CLAID/EmptyDefaultDeviceView.dart';
import 'package:alex_data_collection/GalaxyWatchView.dart';
import 'package:alex_data_collection/SmartInhaler/SmartInhalerDeviceView.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';

import 'CLAID/SelectableDeviceViewWidget.dart';

class DeviceOverviewPage extends StatefulWidget
{


  State<DeviceOverviewPage>? myState;

  @override
  State<DeviceOverviewPage> createState()
  {
    myState = DeviceOverviewPageState();
    return myState!;

  }
  final Function onPressed;

  DeviceOverviewPage({Key? key, required this.onPressed});

  void update()
  {
     if(myState != null)
     {

       myState!.setState(() {

      });
     }
  }

  static DeviceOverviewPageState? of(BuildContext context) => context.findAncestorStateOfType<DeviceOverviewPageState>();
}

class DeviceOverviewPageState extends State<DeviceOverviewPage> with AutomaticKeepAliveClientMixin
{
  @override
  // TODO: implement wantKeepAlive
  bool get wantKeepAlive => true;


  Future<void> onRefresh() async
  {
    setState(() {

    });
  }
  ScrollController _controller = new ScrollController();
  @override
  Widget build(BuildContext context) {
    print("DeviceOverviewPage build");
    return
      RefreshIndicator(onRefresh: onRefresh,

        child: ListView(
          physics: BouncingScrollPhysics(parent: AlwaysScrollableScrollPhysics()),
          children: [
            SizedBox(height: 5,),
            SelectableDeviceViewWidget(title: "Samsung GalaxyWatch 5",deviceView: GalaxyWatchView(),
              onPressed: () {
                widget.onPressed(1);
              },),
            SizedBox(height: 5,),
            SelectableDeviceViewWidget(title: "Smart Inhaler",
                deviceView: SmartInhalerDeviceView(),
                onPressed: () {
                  widget.onPressed(2);

                }
            ),
            SizedBox(height: 5,),
            SelectableDeviceViewWidget(title: "MIR Spirometer", deviceView: EmptyDefaultDeviceView(imagePath: "assets/images/MIRSpirometer.png"),
                onPressed: () {
                  /*Navigator.push(
                    context,
                    MaterialPageRoute(builder: (context) => COREView()));*/
                }),
            SizedBox(height: 5,),
            SelectableDeviceViewWidget(title: "Microphone", deviceView: EmptyDefaultDeviceView(imagePath: "assets/images/microphone.png"),
                onPressed: () {
                  /*Navigator.push(
                    context,
                    MaterialPageRoute(builder: (context) => COREView()));*/
                }),
            SizedBox(height: 5,),
            SelectableDeviceViewWidget(title: "Cough detector", deviceView: EmptyDefaultDeviceView(imagePath: "assets/images/cough.png"),
                onPressed: () {
                  /*Navigator.push(
                    context,
                    MaterialPageRoute(builder: (context) => COREView()));*/
                }),
            SizedBox(height: 5,),
            SelectableDeviceViewWidget(title: "Location", deviceView: EmptyDefaultDeviceView(imagePath: "assets/images/location.png"),
                onPressed: () {
                  /*Navigator.push(
                    context,
                    MaterialPageRoute(builder: (context) => COREView()));*/
                }),
            SizedBox(height: 5,),
            SelectableDeviceViewWidget(title: "Pollen counts", deviceView: EmptyDefaultDeviceView(imagePath: "assets/images/ambee.png"),
                onPressed: () {
                  /*Navigator.push(
                    context,
                    MaterialPageRoute(builder: (context) => COREView()));*/
                }),
            SizedBox(height: 5,),
            SelectableDeviceViewWidget(title: "VivatmoMe", deviceView: EmptyDefaultDeviceView(imagePath: "assets/images/vivatmo.png"),
                onPressed: () {
                  /*Navigator.push(
                    context,
                    MaterialPageRoute(builder: (context) => COREView()));*/
                }),
            ElevatedButton(child: Text("Refresh"),
                onPressed: (){
                  setState(() {

                  });
                })

          ],
        ));

  }
}