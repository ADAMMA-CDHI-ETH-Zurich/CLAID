import 'dart:io';

import 'package:flutter/material.dart';

import 'DeviceView.dart';


class SelectableDeviceViewWidget extends StatelessWidget
{
  final String title;
  final Function onPressed;

  final ButtonStyle style =
  ElevatedButton.styleFrom(textStyle: const TextStyle(fontSize: 20));

  DeviceView deviceView;

  SelectableDeviceViewWidget({Key? key,
    required this.title,
    required this.deviceView, required this.onPressed}) : super(key: key)
  {
    print("initstate SelectableDeviceViewWidget constructor ${deviceView} ${identityHashCode(deviceView)}");
  }


  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return SizedBox(

      height: 75,
      child:   ElevatedButton(
        onPressed: () {
          this.onPressed();
        },
        style: style,
        child: Align(
          alignment: Alignment.centerLeft,
          child: Row(
            mainAxisAlignment: MainAxisAlignment.start,
            mainAxisSize: MainAxisSize.min,
            children: [
              deviceView.getImage(context),
              SizedBox(width: 20),

              Column(
                  crossAxisAlignment: CrossAxisAlignment.start, // for left side
                  // crossAxisAlignment: CrossAxisAlignment.end, // for right side
                  children: <Widget>[
                    Padding(padding: EdgeInsets.only(top: 9)),
                    Text(this.title),
                    deviceView.getSubCaptionWidget(),

                  ])



            ],
          ),
        ),
      ),

    );

  }
}