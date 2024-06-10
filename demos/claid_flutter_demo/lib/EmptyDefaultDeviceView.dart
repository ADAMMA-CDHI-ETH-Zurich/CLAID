import 'dart:io';

import 'package:flutter/material.dart';

import 'CLAIDModuleView.dart';




class EmptyDefaultDeviceView extends CLAIDModuleView {

  String imagePath = "";


  EmptyDefaultDeviceView({Key? key, required this.imagePath,
      required super.mappedModuleId, required super.moduleClass, required super.remoteFunctionHandler}) : super(key: key);

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
    return "DefaultView";
  }

}


class _EmptyDefaultDeviceViewState extends State<EmptyDefaultDeviceView>
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
            !loaded ? CircularProgressIndicator() : CircularProgressIndicator() ,
            !loaded ? CircularProgressIndicator() : CircularProgressIndicator() ,
            !loaded ? CircularProgressIndicator() : CircularProgressIndicator() ,
          ],
        )
    );
  }




}