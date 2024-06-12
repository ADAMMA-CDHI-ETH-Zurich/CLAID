import 'dart:io';

import 'package:claid/ui/CLAIDModuleView.dart';
import 'package:flutter/material.dart';





class TestStreamView extends CLAIDModuleView {

  String imagePath = "assets/images/img.png";


  TestStreamView(
      super.mappedModuleId, super.moduleClass,
      super.remoteFunctionHandler, {super.key});

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

}


class _TestStreamViewState extends State<TestStreamView> with AutomaticKeepAliveClientMixin
{

  @override
  void initState() {
    super.initState();
    print("TestStreamView init state");
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
            !loaded ? CircularProgressIndicator() : CircularProgressIndicator() ,
            !loaded ? CircularProgressIndicator() : CircularProgressIndicator() ,
            !loaded ? CircularProgressIndicator() : CircularProgressIndicator() ,
          ],
        )
    );
  }
  bool get wantKeepAlive => true;




}