import 'dart:io';

import 'package:flutter/material.dart';



abstract class ModuleView extends StatefulWidget
{
    SelectableDeviceViewWidget? parent;

    MappedModule _module;

    DeviceView({Key? key, this.parent}) : super(key: key)
    {
    print("initstate ModuleView constr");
    }

    Widget getSubCaptionWidget();
    Widget getImage(BuildContext context);
}