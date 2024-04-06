# Codec options
As the walkie talkie will use digital voice transmission, we need a way to digitize speech.  Several open source speech codecs are available.  We will focus on low-bitrate codecs because we want long range.  Opus and Speex won't do.  There's one codec that excels : **codec2**.

## Codec2

* open source, royalty free replacement for AMBE.  At 2400bps, [AMBE+](https://www.rowetel.com/?p=5520) still performs better than Codec2.
* bitrates as low as 700bps possible (but not usable, see [Codec2 evaluation](https://hackaday.io/project/174231-digital-walkie-talkie/log/182246-codec2-installation-test).
* open source
* already implemented on embedded platforms : STM32, nRF52
* used in [FreeDV](https://freedv.org/)</a>, [QRadioLink](http://qradiolink.org/)

### Codec2 Configuration
Audio input format : 16bit signed integer, 8kHz sample rate, mono
Codec2 packet details (Referencen = [codec2 source code](https://github.com/blanu/codec2-arduino/blob/master/src/codec2/codec2.c)).

| Encoded Data rate [bps] | Bits/packet | Bytes/packet | Time interval [ms] | Packets/s |
|------------------------|-------------|--------------|--------------------|-----------|
| 3200                   | 64          | 8            | 20                 | 50        |
| 2400                   | 48          | 6            | 20                 | 50        |
| 1600                   | 64          | 8            | 40                 | 25        |
| 1400                   | 56          | 7            | 40                 | 25        |
| 1200                   | 48          | 6            | 40                 | 25        |

When using one of the lowest three data rates, there's a drawback that loosing a single packet will cost you 40ms of audio.

### Codec2 Evaluation
Evaluation of Codec2 will be done using [command-line](#command-line-tools) tools and python tools.

#### Command line tools
```bash	
sudo apt install codec2
```
##### Some experiments
After installation of codec2, the raw audio file test samples are available in **/usr/share/codec2/raw<br>**.

I tried playing with the 700bps bitrate, but that never yielded results that were **easily** understandable.  If you had a conversation with someone using this bitrate, you would frequently have to ask to repeat sentences.

1200bps seems to me the minimum practically achievable bitrate.

```bash
$ c2enc 1200 ve9qrp.raw ~/ve9qrp.bit --natural && c2dec 1200 ~/ve9qrp.bit ~/ve9qrp_codec2_1200.raw --natural && aplay -f S16_LE ~/ve9qrp_codec2_1200.raw
max_amp: 80 m_pitch: 320
p_min: 20 p_max: 160
Wo_min: 0.039270 Wo_max: 0.314159
nw: 279 tw: 40
max_amp: 80 m_pitch: 320
p_min: 20 p_max: 160
Wo_min: 0.039270 Wo_max: 0.314159
nw: 279 tw: 40
Playing raw data '/home/christoph/vk5qi_codec2_1200.raw' : Signed 16 bit Little Endian, Rate 8000 Hz, Mono
```
For your audio playback convenience, these raw files have been converted to WAV-file format using:
```bash
sox -e signed-integer -b 16 -r 8000 -c 1 ve9qrp.raw ~/ve9qrp.wav
```
File sizes:
* ve9qrp.raw (original file) : 16bit samples, 8kHz sampling = 128ksps : 1799168 bytes ([WAV-file version](https://cdn.hackaday.io/files/1742317454299104/ve9qrp.wav))
* ve9qrp.bit (codec2 1200 encoded) : 1.2ksps : 16866 bytes
* ve9qrp_codec2_1200.raw (decoded) : 1799040 bytes ([WAV-file version](https://cdn.hackaday.io/files/1742317454299104/ve9qrp_codec2_1200.wav))

So we see that codec2 achieves a 128k/1.2k = 106.7/1 compression ratio.  That's truly impressive.

Of course, this compression ratio comes at a price : computational complexity.  There's no way you could pull this off in real time with an AVR-microcontroller.  

###### TEST0: offline encoding & decoding using cli-tools
The codec2 examples you can find on the internet are presumably specially chosen to go well with the algorithm.  Let's grab [a video from youtube](https://www.youtube.com/watch?v=-PBf58Molvc) using a [Youtube Video Downloader](https://www.freemake.com/nl/free_video_downloader/).  This will give you an mp4-file.  Strip the audio from that video and convert the audio to 8kHz mono and strip it down to the first two minutes using a only a single command:
```bash
ffmpeg -i youtube.mp4 -acodec pcm_s16le -ac 1 -ar 8000 -t 00:02:00 out.wav
```
Then using codec2 on that file is as simple as:
```bash
c2enc 1200 ve9qrp.wav ve9qrp.bit
c2dec 1200 ve9qrp.bit ve9qrp_decoded.raw
ffmpeg -f s16le -ar 8k -ac 1 -i ve9qrp_decoded.raw ve9qrp_decoded.wav
```
The sample from the Youtube video, after running it through codec2 sounds like [this](https://cdn.hackaday.io/files/1742317454299104/youtube_codec2_1200.wav).  No, it doesn't sound great, but keep in mind that the original video has a 44.1kHz stereo signal.  Converting that to 8kHz mono already has an audible impact.  Passing it through a 1200bps codec2 tunnel is responsible for the other artifacts.

##### TEST1: Sine waves
It's easy to generate [sine waves online](https://onlinetonegenerator.com/432Hz.html) and then downsampling them to 8kHz (sox 440.wav -r 8000 440_8kHz.wav).  Unfortunately, pure sine waves are filtered completely out by codec2.  The codec2 algorithm is designed to work with human speech, not with pure sine waves. 

##### TEST4: encoding and decoding of streaming audio using FIFO
```bash
sox ../m17-tools/apollo11_1.wav -t raw - | c2enc 2400 - - | c2dec 2400 - - | play -q -b 16 -r 8000 -c1 -t s16 -
```

#### Python
When you're using Ubuntu, version 19 is needed.  The codec2 library is not available in the standard repositories.  You can install it using:
```bash
sudo apt install python3-pip libcodec2-dev
```
It might be better not to use "sudo" to avoid messing up the libraries that come with your Linux distribution.  Alternatively, you can use PyCharm and use a virtual environment where all of these libraries get installed.
```bash
sudo pip3 install Cython numpy pycodec2
```
##### TEST1: offline encoding & decoding using python
Download [example.py](https://raw.githubusercontent.com/gregorias/pycodec2/master/example.py):
```bash
wget https://raw.githubusercontent.com/gregorias/pycodec2/master/example.py
```
Then run it using: 
```bash
python3 example.py ve9qrp.wav
```
To simultaneously encode/decode this 1'52s audio fragment, the Wandboard (iMX6Q) needs 8.21s.  So real-time implementation should be possible.  [Example.py](https://raw.githubusercontent.com/gregorias/pycodec2/master/example.py) will create output.raw which will contain the wav file encoded and then decoded by codec2.  You can listen to it with:
```bash
aplay -D hw:CARD=imx6wandboardsg,DEV=0 -f S16_LE output.raw
```
The -D option is to send the audio to line-out on a Wandboard.  You could remove it when you're using other hardware.

You can encode and decode a file by using python or using cli-tools and compare the results.  They will be identical.  I simplified the example.py by using np.frombuffer() and np.asarray(), so the "import struct" is no longer necessary.

##### TEST2: reading wav-file from disk, encode, decode, send result to line-out
[play_sound_samplerate_codec2](./software/play_sound_samplerate_codec2) : works fine on a core i7,8thGen and on the Wandboard.

##### TEST3: encoding and decoding of streaming audio
[record_play_codec2](./software/record_play_codec2) : get input from line-in, encode it to Codec2, decode it and output it to line-out.  Works both on the Wandboard and on the laptop.  On the Wandboard, the samplerate filter had to be reduced from "sinc_best" to "sinc_medium", otherwise there was no sound.

## Opus
* Open source, royalty free
* replacement for Speex
* down to 6kbps
* used in VoIP-applications (e.g. WhatsApp)

## MELPe
* NATO standard
* licensed & copyrighted

## Speech-to-text-to-speech
Using a speech codec, data transmission can be brought down to about 1200bps.  But how can we reduce data even further?  Let's take a 1min52s mono 8kHz speech sample as an example.

* [ve9qrp.wav](https://cdn.hackaday.io/files/1742317454299104/ve9qrp.wav) : 1.799.212 bytes : **128000bps**
* ve9qrp.bin : codec2 1200bps encoded : 16.866 bytes : **1200bps**
* [ve9qrp.txt](https://cdn.hackaday.io/files/1742317454299104/ve9qrp.txt) : audio transcription of ve9qpr.wav : 1.178 bytes : **85bps**

Using codec2, we get a 106/1 compression ratio, using text 1527/1 compression ratio.   That's almost fifteen times better!

If we use speech recognition on the transmit side, then send the transcription over and use speech synthesis on the receiving side, this might work.


### Speech recognition
[https://pypi.org/project/SpeechRecognition/](https://pypi.org/project/SpeechRecognition/)

Even though speech recognition engines are very good these days, they're still not as good as human beings.  The transcript text could be shown to the speaker during transmission.  In case of errors, the speaker could repeat the incorrect word or spell out its characters.

A speech engine also requires a language preset.  That shouldn't be too much of a hurdle because most of us only commonly use a single language.

1. Is there good speech recognition software that runs offline?
2. Is speech recognition software not too power hungry?

### Speech synthesis
[Linux command line tools](https://hackaday.io/project/28445-usb-cdc-robosapien-v1/log/181633-text-to-speech)
