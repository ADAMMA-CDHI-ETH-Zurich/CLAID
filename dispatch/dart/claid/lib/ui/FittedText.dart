import 'package:flutter/material.dart';

class FittedText extends StatelessWidget {
  final String text;
  final double maxWidth;
  final double maxHeight;
  final TextStyle? style;

  FittedText({
    required this.text,
    required this.maxWidth,
    required this.maxHeight,
    this.style,
  });

  double calculateFontSize(String text, double maxWidth, double maxHeight, TextStyle? textStyle, {double minFontSize = 8, double maxFontSize = 100}) {
    double fontSize = maxFontSize;
    final baseStyle = textStyle ?? TextStyle();
    final textPainter = TextPainter(
      text: TextSpan(text: text, style: baseStyle.copyWith(fontSize: fontSize)),
      maxLines: 1,
      textDirection: TextDirection.ltr,
    );

    while (fontSize > minFontSize) {
      textPainter.text = TextSpan(text: text, style: baseStyle.copyWith(fontSize: fontSize));
      textPainter.layout();

      if (textPainter.size.width <= maxWidth && textPainter.size.height <= maxHeight) {
        break;
      }
      fontSize -= 1;
    }

    return fontSize;
  }

  @override
  Widget build(BuildContext context) {
    final baseStyle = style ?? TextStyle();
    final fontSize = calculateFontSize(text, maxWidth, maxHeight, style);
    return Text(
      text,
      style: baseStyle.copyWith(fontSize: fontSize),
    );
  }
}
