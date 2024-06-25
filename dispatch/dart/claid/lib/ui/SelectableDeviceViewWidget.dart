import 'dart:io';

import 'package:flutter/material.dart';

import 'CLAIDModuleView.dart';

class RectanglePainter extends CustomPainter
{
  String firstLine;
  String secondLine;

  RectanglePainter(this.firstLine, this.secondLine)
  {

  }

  @override
  void paint(Canvas canvas, Size size) {
    final paint = Paint()
      ..color = Color.fromARGB(175, 255, 255, 255)
      ..style = PaintingStyle.fill;

    final rrect = RRect.fromRectAndRadius(
      Rect.fromLTWH(10, 0, size.width, size.height),
      Radius.circular(5),
    );
    canvas.drawRRect(rrect, paint);

    final textSpan = TextSpan(
      children: [
        TextSpan(
          text: firstLine + "\n",
          style: TextStyle(
            color: Color.fromARGB(255, 47, 110, 187),
            fontSize: 16,
            fontWeight: FontWeight.bold,
            fontFamily: 'Sans Serif', // Change font family to Arial
          ),
        ),
        TextSpan(
          text: secondLine,
          style: TextStyle(
            color: Color.fromARGB(255, 47, 110, 187),
            fontSize: 10,
            fontWeight: FontWeight.bold,
            fontFamily: 'Sans Serif', // Change font family to Arial
          ),
        ),
      ],
    );

    final textPainter = TextPainter(
      text: textSpan,
      textAlign: TextAlign.center,
      textDirection: TextDirection.ltr,
    );

    textPainter.layout(
      minWidth: 0,
      maxWidth: size.width,
    );

    final offset = Offset(
      (size.width - textPainter.width) / 2  + 10,
      (size.height - textPainter.height) / 2,
    );

    textPainter.paint(canvas, offset);
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) {
    return false;
  }
}
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

    final screenSize = MediaQuery.of(context).size;
    final buttonHeight = screenSize.height * 0.09;
    final typeRectHeight = buttonHeight * 0.8;
    final typeRectOffsetTop = (buttonHeight - buttonHeight * 0.8) / 2;

    return SizedBox(
      height: buttonHeight,
      child: ElevatedButton(
          onPressed: () {
            this.onPressed();
          },
        style: buttonStyle,
        child: Stack(
          children: [
            Align(
              alignment: Alignment.centerLeft,
              child: Row(
                mainAxisAlignment: MainAxisAlignment.start,
                mainAxisSize: MainAxisSize.min,
                children: [
                  deviceView.getImage(context),
                  SizedBox(width: 20),
                  Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: <Widget>[
                      Padding(padding: EdgeInsets.only(top: 9)),
                      Text(
                        title,
                        style: const TextStyle(color: Colors.white),
                      ),
                      deviceView.getSubCaptionWidget(),
                    ],
                  ),
                ],
              ),
            ),
            Positioned(
              right: 0,
              top: typeRectOffsetTop, // Center the rectangle vertically within the button
              child: CustomPaint(
                size: Size(60, typeRectHeight),
                painter: RectanglePainter(
                    deviceView.getModuleType(),
                    deviceView.getModuleSubType()),
              ),
            ),
          ],
        ),
      ),
    );

  }
}