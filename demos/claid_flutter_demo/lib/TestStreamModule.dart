
import 'package:claid/module/module.dart';
import 'package:claid/module/channel.dart';

import 'package:claid/module/properties.dart';

class TestStreamModule extends Module
{
  late Channel<String> dataChannel;
  int ctr = 0;

  @override
  void initialize(Properties properties) {
    moduleInfo("Init!");
    dataChannel = publish<String>("MyData", "");
    registerPeriodicFunction("DataStreamer", Duration(milliseconds: 200), () => _streamData());
  }

  void _streamData()
  {
    print("Posting data");
    dataChannel.post("This is test $ctr");
    ctr++;
  }

}