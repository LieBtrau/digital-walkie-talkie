#include <Arduino.h>

#include "esp_adc_cal.h"

// calibration values for the adc
const uint32_t DEFAULT_VREF=1100;
const adc_unit_t ADC_UNIT = ADC_UNIT_1;
esp_adc_cal_characteristics_t *adc_chars;

void setup()
{
  Serial.begin(115200);
  Serial.println("Started up");

  //Range 0-4096
  adc1_config_width(ADC_WIDTH_BIT_12);
  // full voltage range
  adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);

    //Characterize ADC
    adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Serial.printf("eFuse Vref\r\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        Serial.printf("Two Point\r\n");
    } else {
        Serial.printf("Default\r\n");
    }
}

void loop()
{
  // for a more accurate reading you could read multiple samples here

  // read a sample from the adc using GPIO35
  int sample = adc1_get_raw(ADC1_CHANNEL_7);
  // get the calibrated value
  int milliVolts = esp_adc_cal_raw_to_voltage(sample, adc_chars);

  Serial.printf("Sample=%d, mV=%d\n", sample, milliVolts);
  //Serial.printf("%04X\r\n", sample);
  delay(500);
}