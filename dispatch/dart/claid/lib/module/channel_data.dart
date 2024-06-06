import 'dart:async';

class ChannelData<T> {
  final T value;
  final DateTime timestamp;
  final String userId;
  const ChannelData(this.value, this.timestamp, this.userId);
}
