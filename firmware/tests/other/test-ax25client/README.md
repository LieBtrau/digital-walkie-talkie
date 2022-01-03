
# Description
Connect to Aprsdroid application using Bluetooth SPP.

# APRS-messages
Send to and receive messages from the Aprsdroid application.
When receiving messages, acknowledgements will be sent in case requested.
When sending messages, an acknowledgement can be requested.  In that case, the message will be resent a couple of times until the acknowledgement arrives.

# APRS-location
## Sending location
* Set callsign
* Set location (decimal latitude, decimal longitude)
* Set symbol
* Set comment
## Receiving location
I couldn't find any Arduino/embedded application that was capable of decoding location frames.
Location, symbol and source address are decoded.
