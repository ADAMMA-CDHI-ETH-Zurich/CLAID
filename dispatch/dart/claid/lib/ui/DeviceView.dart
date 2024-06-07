import 'dart:io';

import 'package:flutter/material.dart';

import 'SelectableDeviceViewWidget.dart';


abstract class DeviceView extends StatefulWidget
{
  SelectableDeviceViewWidget? parent;

  DeviceView({Key? key, this.parent}) : super(key: key)
  {
    print("initstate DeviceView constr");
  }

  Widget getSubCaptionWidget();
  Widget getImage(BuildContext context);
}