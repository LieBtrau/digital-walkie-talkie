Original source comes from https://github.com/UBNanosatLab/lib446x.
That's a C-library, while Arduino is C++.  Instead of using "extern C", I changed the source to C++.  Just for a PoC.  Not a very nice job.
## Disadvantages
The library doesn't really use interrupts.  It's actually polling the IRQ-line.
This library can send packets of 160 bytes, but it has trouble doing that reliably.  Sending the first packet normally works, but following packets contain about 1/4 of zeroes and junk data at the end.
Resetting the transmitter brings it back to "sending the first packet", which "solves" the issue.
## Advantages
 - Contains pre-configured setups, so that configuring for other baud rates shouldn't be much of a problem.
