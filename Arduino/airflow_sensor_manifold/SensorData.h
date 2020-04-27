#pragma once

#include <stdint.h>
#include <Arduino.h>
#include "Display.h"
#include "Types.h"

#define MAX_DATAPOINTS    (300)

typedef struct {
	float value;
	uint32_t millis;
} Datapoint;

class SensorData {

public:
	SensorData(String _title, Axis _yAxis);
	String getTitle();
	Axis getYAxis();
	void deleteDatapointsBefore(uint32_t millis);
	void add(float value, uint32_t millis);
	float getLastValue();

	// Returns dummy data (value==0, millis==0) if out of range/invalid
	const Datapoint * at(int32_t index);
	int32_t length();

protected:
	String title;
	Axis yAxis;
	int32_t dataLen = 0;
	Datapoint data[MAX_DATAPOINTS];	// FIFO

	void pop_front(int32_t popCount);	// Remove data from front
};
