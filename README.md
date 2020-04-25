#### This project is similar to the project using the standalone sensor described here: [AFH55M12](https://github.com/hydronics2/COVID-19-Airflow-Sensor-AFH55M12)

# Automotive Airflow Sensor (AFH55M12) in a manifold configuration

![foo](https://github.com/hydronics2/COVID-19-Airflow-Sensor-AFH55M12-manifold/blob/master/pics/manifold_design.PNG)
Picture. Manifold by Patrick Campbell with Tee Fittings modified to accept airflow sensors.

## Project Description ([from Helpful Engineering](https://www.helpfulengineering.org/))
The intent here is to create a monitoring device, based on a mass airflow meter, that can be used when splitting a ventilator into two or more patients. This will allow staff to monitor individual patients while being controlled by one device in extreme situations where the number of ventilators are not enough to handle the number of patients. The readout should be visible locally on the device and there may need to be parameters input by staff to create a safe operating range and to possibly create alarms when the system is measuring an out of range parameter.


### [Project Requirements](https://docs.google.com/document/d/17Ps910A2vRwnM4EM6F-71GNG1XNa0PaeImd53F7428c/edit?usp=sharing)

This is a quick study of using an inexpensive off-the-shelf automotive airflow type sensor.

Reading from an automotive mass airflow sensor using a microController 12bit ADC, 20ms interval


![foo](https://github.com/hydronics2/COVID-19-Airflow-Sensor-AFH55M12-manifold/blob/master/pics/step_file.PNG)

[![Foo](https://github.com/hydronics2/COVID-19-Airflow-Sensor-AFH55M12-manifold/blob/master/pics/video.PNG)](https://youtu.be/N4MVttKF_WM)

![foo](https://github.com/hydronics2/COVID-19-Airflow-Sensor-AFH55M12-manifold/blob/master/pics/display_image.PNG)

![foo](https://github.com/hydronics2/COVID-19-Airflow-Sensor-AFH55M12-manifold/blob/master/pics/pressure_graph.PNG)

### --- Updates April 24th -- Assembled PCB, airflow sensor

### Teensy pins
- pin 0 - user led
- pin 1 - error led
- pin 23 - buzzer
- pin A0 - flow sensor pin
- pin 20 - encoder A
- pin 21 - encoder B
- pin 22 - encoder button
- pin 5 - button





### Sensor Availability
The sensor shows up as multiple part values. It fits a 1998 Nissan/Datsun Truck Frontier 2WD 2.4L FI 4cyl among other years and other vehicles.
- [China](https://www.aliexpress.com/item/32989444018.html?spm=a2g0o.cart.0.0.74dd3c00g1XL5u&mp=1)
- [US Supplier](https://www.amazon.com/gp/product/B074FZMGWG/ref=ppx_yo_dt_b_asin_title_o00_s00?ie=UTF8&psc=1)
- [local supplier oreilly](https://www.oreillyauto.com/search/1998/nissan/frontier?q=MF21219)
- [local supplier auto-zone](https://www.autozone.com/engine-management/mass-air-flow-sensor/duralast-new-mass-air-flow-sensor-dl-3025/337334_0_4995?&searchText=DL3025)

#### Pinout
- Pin A - Empty
- Pin B - 7-12V DC, 90-115mA
- Pin C - Ground
- Pin D Signal

#### Parts List
- microController - [Teensy 4.0](https://www.pjrc.com/store/teensy40.html)
- display - [2.8" TFT 320x240 display](https://www.pjrc.com/store/display_ili9341_touch.html)
- BME280 pressure sensor [828-1063-1-ND](https://www.digikey.com/product-detail/en/bosch-sensortec/BME280/828-1063-1-ND/6136314)
