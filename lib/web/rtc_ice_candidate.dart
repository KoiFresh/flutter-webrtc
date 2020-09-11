import 'dart:html' as html;

class RTCIceCandidate {
  final String candidate;
  final String sdpMid;
  final int sdpMlineIndex;

  RTCIceCandidate(this.candidate, this.sdpMid, this.sdpMlineIndex);
  RTCIceCandidate.fromJs(html.RtcIceCandidate jsIceCandidate)
      : this(jsIceCandidate.candidate, jsIceCandidate.sdpMid,
            jsIceCandidate.sdpMLineIndex);

  dynamic toMap() {
    return {
      'candidate': candidate,
      'sdpMid': sdpMid,
      'sdpMLineIndex': sdpMlineIndex
    };
  }

  html.RtcIceCandidate toJs() => html.RtcIceCandidate(toMap());
}
