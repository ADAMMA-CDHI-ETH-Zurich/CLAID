import 'dart:async';

import 'channel_access_rights.dart';
import 'channel_data.dart';
import 'type_mapping.dart';
import 'module_manager.dart';
import 'package:claid/generated/claidservice.pb.dart';
import 'package:protobuf/protobuf.dart';
import 'package:fixnum/fixnum.dart';
typedef ChannelCallback<T> = FutureOr<void> Function(ChannelData<T> payload);

class Channel<T> 
{    
    final ChannelCallback<T>? _callback;
    final ChannelAccessRights _accessRights;
    
    final String _moduleId;
    final String _channelId;

    final Mutator<T> _mutator;
    final ModuleManager _manager;

    Channel(this._moduleId, this._channelId, this._mutator, this._manager, this._accessRights, this._callback)
    {

    }

    bool _canRead()
    {
        return this._accessRights == ChannelAccessRights.READ || this._accessRights == ChannelAccessRights.READ_WRITE;
    }

    bool _canWrite()
    {
        return this._accessRights == ChannelAccessRights.WRITE || this._accessRights == ChannelAccessRights.READ_WRITE;
    }

    Future<void> post(T payload) async
    {   
        if(!_canWrite())
        {
            throw AssertionError('Cannot post to channel ${_channelId}, Channel was not published.');
        }
        final pkt =
            DataPackage(sourceModule: _moduleId, channel: _channelId, unixTimestampMs: Int64(DateTime.now().millisecondsSinceEpoch));

        _mutator.setter(pkt, payload);
        await _manager.publish(pkt);
    }

    void onMessageReceived(DataPackage pkt) async
    {
        final cData = ChannelData<T>(
            _mutator.getter(pkt),
            DateTime.fromMillisecondsSinceEpoch(pkt.unixTimestampMs.toInt()),
            pkt.sourceUserToken,
        );
        print("on message received");

        if (_callback != null) 
        {
            await _callback!(cData);
        }

    }
}
