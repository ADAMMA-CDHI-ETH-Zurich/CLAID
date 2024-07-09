

import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';

import 'CLAIDModuleView.dart';
import 'SelectableDeviceViewWidget.dart';


class CLAIDModuleList extends StatefulWidget
{

  State<CLAIDModuleList>? myState;

  @override
  State<CLAIDModuleList> createState()
  {
    myState = CLAIDModuleListState();
    return myState!;
  }
  final Function onPressed;
  List<CLAIDModuleView> modules;

  CLAIDModuleList({Key? key, required this.onPressed, required this.modules});

  void update()
  {
     if(myState != null)
     {

       myState!.setState(() {

      });
     }
  }

  static CLAIDModuleListState? of(BuildContext context) => context.findAncestorStateOfType<CLAIDModuleListState>();
}

class CLAIDModuleListState extends State<CLAIDModuleList> with AutomaticKeepAliveClientMixin
{
  @override
  // TODO: implement wantKeepAlive
  bool get wantKeepAlive => true;


  Future<void> onRefresh() async
  {
    setState(() {

    });
  }
  @override
  Widget build(BuildContext context) {
    print("CLAIDModuleList build " + this.widget.modules.toString());

    int index = -1;
    return
      RefreshIndicator(onRefresh: onRefresh,

        child: ListView(
          physics: BouncingScrollPhysics(parent: AlwaysScrollableScrollPhysics()),
          children: this.widget.modules.map((widget) {
            index++;
            int currentIndex = index;

            return Padding(padding: const EdgeInsets.only(bottom: 7),
              child:  SelectableDeviceViewWidget(
                title: widget.getName(),
                buttonId: index,
                numButtons: this.widget.modules.length,
                deviceView: widget,
                onPressed: () {
                  this.widget.onPressed(currentIndex);
                },
            ));
          }).toList()
        ));

  }
}