import Adafruit_BBIO.ADC as ADC
import time

sensor_pin = 'P9_40'

ADC.setup()

while True:
	reading = ADC.read(sensor_pin)
	millivolts = reading * 1800
	temp_c = (millivolts - 500) / 10
	print('(Sampled millivolts: %d), Temperature: %d' % (millivolts, temp_c))
	time.sleep(1)

