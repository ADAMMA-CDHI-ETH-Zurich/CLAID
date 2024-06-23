import 'dart:io';

import 'package:flutter/material.dart';

import 'CLAIDModuleView.dart';




class EmptyDefaultDeviceView extends CLAIDModuleView  {

  String imagePath = "assets/images/questionmark.png";


  EmptyDefaultDeviceView(
      super.entityName,
      super.mappedModuleId, super.moduleClass,
      super.moduleManager, {super.key});

  @override
  State<EmptyDefaultDeviceView> createState() => _EmptyDefaultDeviceViewState();

  @override
  Widget getSubCaptionWidget()
  {
    return Column(
        crossAxisAlignment: CrossAxisAlignment.start, // for left side
        children: [
          Row(
            children: [
              Text("Online", style:TextStyle(fontSize: 15)),
              SizedBox(width:5),
              Icon(Icons.circle,
                size: 15,
                color: Colors.green,
              ),
            ],
          ),
          Text("Last value received: 10 min ago", style:TextStyle(fontSize: 15))
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
    return super.mappedModuleId + " (" + super.moduleClass + ")";
  }

  String getModuleType()
  {
    return "Example";
  }

  String getModuleSubType()
  {
    return "(internal)";
  }

}


class _EmptyDefaultDeviceViewState extends State<EmptyDefaultDeviceView> with AutomaticKeepAliveClientMixin
{

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
            Text("Nothing to see here! This is just the default view.")
          ],
        )
    );
  }


  bool get wantKeepAlive => true;


}