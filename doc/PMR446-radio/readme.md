
# Radio comms
DCS : Digital Code Squelch (DCS) is encoded below 300Hz.  Also receivers should have a 300Hz high pass audio filter to reduce the CDCSS signal at the speaker.

## Yeasu FT-65E to Midland G9 Pro
* PMR446 (DCS enabled): max. output at 670Hz.  Bandwidth = 460Hz to 1.3kHz
* PMR446 (DCS disabled, FT65-E narrow TX, G9Pro traditional 8 channels): Bandwidth = 432Hz to 1.2kHz
* LPD433 : 
 * (DCS enabled) FT65-E set to TX:Wide : max output at 714Hz.  Bandwidth = 458Hz to 1.46kHz
 * (DCS enabled) FT65-E set to TX:Narrow : max output 612Hz at Bandwidth = 432Hz to 1.20kHz
 * (DCS disabled) FT65-E set to TX:Narrow : max output 612Hz at Bandwidth = 432Hz to 1.20kHz
 
Maybe audio is being corrected because human ear is more sensitive to frequencies in 1kHz to 3kHz range?
Sine wave audio is hearable from 300Hz to 3.3kHz
Is some kind of pre-emphasis into play here?
Do we see the same in SdrPlay (with AGC disabled)?
 
# Radio communication modes
There are few options available for radios having only 1kHz bandwidth.  The problem is that there's no access to the modulator.  There's no way to skip the 460Hz to 1.2kHz filter.
I see no way to make digital voice work.  Digital keyboard-to-keyboard communication could work, but it would be very spectrum inefficient.

## RTTY
* Several frequency shifts possible : https://www.sigidwiki.com/wiki/Radio_Teletype_(RTTY)

## QPSK31
32baud

## MT63-2000L
2kHz BW = 20baud
* Fldigi (also on linux) , with GUI
* https://github.com/DavidGriffith/hf -> CLI
* Flmsg (on linux)
* DM780, MultiPSK

