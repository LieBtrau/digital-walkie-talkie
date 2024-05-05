The use of a digital module has been taken into consideration, but it's not the best choice for this project due to the limited flexibility.

# LoRa versus FSK
Two parameters are important here : sensitivity and adjacent channel blocking or selectivity.  Sensitivity is the minimum signal strength that can be received.  Blocking is the ability to receive a signal in the presence of a strong signal.  Sensitivity is important for long range communication, while blocking is important for multiple access communication.  

## FSK Theory

### 2FSK
* Frequency deviation (Δf) = the difference between the minimum and maximum extent of a frequency modulated signal, and the nominal center or carrier frequency.  
$$f_{carrier} - f_{min} = f_{max} - f_{carrier} = \Delta f$$
* The relation between bandwidth (BW) and bitrate (BR) is defined by [Carson's Rule](https://en.wikipedia.org/wiki/Carson_bandwidth_rule) : $$BW = 2*(\Delta f+\frac{BR}{2}) = BR+2*\Delta f$$
* Modulation index : $$m = \frac{2*\Delta f}{BR}$$
  * MSK : Minimum Shift Keying : m = 0.5 -> Δf = BR/4
  * Narrowband FM when m < 1

### 4FSK
* modulation index m (innerdeviation is frequency difference of the two used modulating frequencies): 
$$m = \frac{2*innerdeviation}{symbolrate}$$
  * 2x more spectrally efficient modulation than 2GFSK.
* For the same data rate and modulation index, 4GFSK has about 2dB loss in sensitivity with respect to 2GFSK:
  * inner deviation = outer deviation / 3.  For the same data rate the bandwidth is halved, so the inner deviation for 4GFSK is only 1/6 of the frequency deviation of the 2GFSK.  This corresponds to a 5dB loss (depends on modulation index).
  * Halving the RX bandwidth for 4GFSK reduces noise power in the modem by 3dB.

### References
* [4gfsk_vs_2gfsk](https://www.silabs.com/community/wireless/proprietary/knowledge-base.entry.html/2015/06/05/4gfsk_vs_2gfsk_sens-u6p6)
* [Calculation of the modulation index for digital frequency modulation](https://www.silabs.com/community/wireless/proprietary/knowledge-base.entry.html/2015/02/04/calculation_of_them-Vx5f)

## LoRa
For LoRa SF7, CR=2 (4/6 = 2 error correction bits per 4 data bits), **BW=250kHz**, bit rate is 9.1kbps.  It's 10.1kbps when CR=1.  For these settings, the SX1278 has a receiver sensitivity of -120 to -122dBm.

# Choice of chipset
* Si4463 : (PER 1%), (9.6 kbps, 4GFSK, BT = 0.5, df = +/-2.4kHz) : -110dBm
* ADF7021 : NRND
* CC1200 : 38.4kbps 2-GFSK, DEV=20 kHz CHF=104 kHz : -111dBm
* CC1125 : 50-kbps 2-GFSK, DEV=25 kHz, CHF=100 kHz : -109dBm
* AX5243 : no longer advertised on the OnSemi website

Sensitivity is hard to compare between chipsets because there's no data available of the mode we're interested in.  

# Firmware support
* Si4463 : Currently (2024) the WDS application for configuring this radio is no longer available on the SiLabs website.  Older versions can be installed, but automatic firmware upgrade doesn't work.  A username and password is required for that.  The Silabs website isn't working properly.  I receive a lot of "Access Denied" messages.  The "sub GHz protocol" section doesn't seem publicly available. 
* CC1200 : All data is publicly available from the TI website.

# Choice of high power module
## Si4463
| Module | Output Power | Price |
| --- | --- | --- |
| [HopeRF RFM26W](https://www.hoperf.com/modules/rf_transceiver/RFM26W.html) | 10dBm | (?) |
| [NiceRF RF4463F30](https://www.nicerf.com/fsk-front-end-module/1w-rf-module-rf4463f30.html) | 30dBm | [€23.17/2pcs](https://www.aliexpress.com/item/32393788759.html) |
| [Ebyte E30-400M30S(4463)](https://www.cdebyte.com/products/E30-400M30S(4463)) | 30dBm | [€6.65/pce](https://www.aliexpress.com/item/1005003695107704.html) |

The RF4463F30 is better documented than the E30-400M30S(4463) but it's also twice as expensive.  Firmware development will continue on the now obsolete E10-433MD-SMA.

## CC1200
There's a module with a CC1190 front-end.  The CC1190 is a 27dBm amplifier. It's only suited for the 868MHz band.