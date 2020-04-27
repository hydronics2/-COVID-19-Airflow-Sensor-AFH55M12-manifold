#include <stdint.h>
#include <Arduino.h>
#include "Display.h"
#include "SensorData.h"
#include "Types.h"

SensorData::SensorData(String _title, Axis _yAxis) {
	title = _title;
	yAxis = _yAxis;
}

String SensorData::getTitle()
{
	return title;
}

Axis SensorData::getYAxis()
{
	return yAxis;
}

void SensorData::deleteDatapointsBefore(uint32_t millis)
{
	// First the first Datapoint _after_ millis:
	int32_t i;
	for (i = 0; i < dataLen; i++) {
		if (data[i].millis >= millis) break;
	}

	// Anything to shift off?
	if (i) {
		pop_front(i);
	}
}

void SensorData::add(float value, uint32_t millis)
{
	// data array is full?
	if (dataLen == MAX_DATAPOINTS) {
		pop_front(1);	// Remove earliest datapoint
	}

	data[dataLen++] = {value: value, millis: millis};
}

float SensorData::getLastValue()
{
	if (dataLen > 0) {
		return data[dataLen - 1].value;
	}
	return 0;
}

const Datapoint * SensorData::at(int32_t index)
{
	if ((index < 0) || (dataLen <= index)) {
		static const Datapoint dummyData = {value: 0.0f, millis: 0};
		return &dummyData;
	}

	return &data[index];
}

int32_t SensorData::length()
{
	return dataLen;
}

// -- PROTECTED --

void SensorData::pop_front(int32_t popCount)
{
	// pop more values than dataLen? Just set length to 0.
	if (popCount >= dataLen) {
		dataLen = 0;
		return;
	}

	// Bucket brigade: move higher datapoints to lower positions.
	dataLen -= popCount;
	for (int32_t i = 0; i < dataLen; i++) {
		data[i] = data[i + popCount];
	}
}
