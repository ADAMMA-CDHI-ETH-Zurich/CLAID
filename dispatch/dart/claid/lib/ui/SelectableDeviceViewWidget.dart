import 'dart:io';

import 'package:flutter/material.dart';

import 'CLAIDModuleView.dart';


class SelectableDeviceViewWidget extends StatelessWidget
{
  final String title;
  final Function onPressed;
  final int buttonId;
  final numButtons;
  Color interpolateColor(int index, int maxElements) {
    Color startColor = Color.fromARGB(255, 107, 180, 236);
    Color endColor = Color.fromARGB(255,  85, 148, 232);
    double fraction = index.toDouble() / (maxElements.toDouble() - 1);
    print("Fraction $fraction $index $maxElements");
    int red = startColor.red + ((endColor.red - startColor.red) * fraction).round();
    int green = startColor.green + ((endColor.green - startColor.green) * fraction).round();
    int blue = startColor.blue + ((endColor.blue - startColor.blue) * fraction).round();

    return Color.fromARGB(255, red, green, blue);
  }

  late ButtonStyle buttonStyle;
  CLAIDModuleView deviceView;

  SelectableDeviceViewWidget({Key? key,
    required this.title,
    required this.buttonId,
    required this.numButtons,
    required this.deviceView, required this.onPressed}) : super(key: key)
  {
    print("initstate SelectableDeviceViewWidget constructor ${deviceView} ${identityHashCode(deviceView)}");
    buttonStyle =
      ElevatedButton.styleFrom(
        textStyle: const TextStyle(fontSize: 20),
        backgroundColor: interpolateColor(this.buttonId, this.numButtons),
        shadowColor: Colors.black, // Color of the shadow
        elevation: 10, // Elevation to create the shadow effect
        shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(9),
      ));
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
        style: buttonStyle,
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
                    Text(this.title, style: const TextStyle(color: Colors.white),),
                    deviceView.getSubCaptionWidget(),

                  ])



            ],
          ),
        ),
      ),

    );

  }
}