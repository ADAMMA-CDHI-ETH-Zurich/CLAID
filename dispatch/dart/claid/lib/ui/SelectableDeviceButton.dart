import 'dart:io';

import 'package:flutter/material.dart';


class SelectableDeviceButton extends StatelessWidget
{
  final String title;
  final String imageAssetPath;
  final Function onPressed;

  final ButtonStyle style =
  ElevatedButton.styleFrom(textStyle: const TextStyle(fontSize: 20));

  Widget _subCaptionWidget = Column(
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

  SelectableDeviceWidget({Key? key, required this.title, required this.imageAssetPath, required this.onPressed, Widget? captionWidget}) : super(key: key)
  {
    if(captionWidget != null)
    {
      _subCaptionWidget = captionWidget;
    }
    else
    {
    }
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
              Image.asset(this.imageAssetPath,
                height: MediaQuery.of(context).size.height * 0.075,
              ),
              SizedBox(width: 20),

              Column(
                  crossAxisAlignment: CrossAxisAlignment.start, // for left side
                  // crossAxisAlignment: CrossAxisAlignment.end, // for right side
                  children: <Widget>[
                    Padding(padding: EdgeInsets.only(top: 9)),
                    Text(this.title),
                    _subCaptionWidget,

                  ])



            ],
          ),
        ),
      ),

    );

  }
}