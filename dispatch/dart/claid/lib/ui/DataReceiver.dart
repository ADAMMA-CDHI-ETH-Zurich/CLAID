import 'package:claid/module/type_mapping.dart';
import 'package:claid/module/module_manager.dart';
import 'package:claid/generated/claidservice.pb.dart';

typedef DataReceiverCallback<T> = void Function(T);

class DataReceiver<T>
{
    Mutator<T> _mutator;
    DataReceiverCallback<T> _callback;

    DataReceiver(this._mutator, this._callback)
    {

    }

    void onData(DataPackage package)
    {
        T data = _mutator.getter(package);
        _callback(data);
    }
}