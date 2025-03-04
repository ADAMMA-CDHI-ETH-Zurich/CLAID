//
//  Duration+TimeInterval.swift
//  CLAID
//
//  Created by Patrick Langer on 22.02.2025.
//
import Foundation
extension Duration {
  /// Possibly lossy conversion to TimeInterval
  var timeInterval: TimeInterval {
    TimeInterval(components.seconds) + Double(components.attoseconds)/1e18
  }
}
