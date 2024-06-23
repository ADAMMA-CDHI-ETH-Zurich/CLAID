import 'dart:io';

import 'package:claid/ui/CLAIDModuleView.dart';
import 'package:flutter/material.dart';





class TestStreamView extends CLAIDModuleView {

  String imagePath = "assets/images/img.png";


  TestStreamView(
      super.entityName,
      super.mappedModuleId, super.moduleClass,
      super.moduleManager, {super.key});

  @override
  State<TestStreamView> createState() => _TestStreamViewState();

  @override
  Widget getSubCaptionWidget()
  {
    return Column(
        crossAxisAlignment: CrossAxisAlignment.start, // for left side
        children: [
          Row(
            children: [
              Text("Online", style:TextStyle(fontSize: 15, color: Colors.white)),
              SizedBox(width:5),
              Icon(Icons.circle,
                size: 15,
                color: Colors.green,
              ),
            ],
          ),
          Text("Last value received: 10 min ago",
              style:TextStyle(fontSize: 15, color: Colors.white),)
        ]
    );
  }



  Widget getImage(BuildContext context)
  {
    return Image.asset(this.imagePath,
      height: MediaQuery.of(context).size.height * 0.075,
      width: MediaQuery.of(context).size.width * 0.125,
    );
  }

  String getName()
  {
    return super.mappedModuleId;
  }

  String getModuleType()
  {
    return "Sensor";
  }
  String getModuleSubType()
  {
    return "(BLE)";
  }

}


class _TestStreamViewState extends State<TestStreamView> with AutomaticKeepAliveClientMixin
{

  String currentText = "";
  @override
  void initState() {
    super.initState();
    print("TestStreamView init state");

    this.widget.subscribeModuleChannel("MyData", "", (data) => onStreamData(data));
  }

  void onStreamData(String data)
  {
    setState(() {
      currentText = data;
    });
  }

  bool loaded = false;

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {


    return Scaffold(
        appBar: AppBar(
          title: const Text('Galaxy Watch 5'),
        ),
        body:
        Column(
          children: [
            SizedBox(
              height: 50,
            ),
            currentText == "" ? CircularProgressIndicator() : Text(currentText)
          ],
        )
    );
  }
  bool get wantKeepAlive => true;




}